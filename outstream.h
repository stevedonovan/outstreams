// Lightweight operator() overloading stdio wrapper
// Steve Donovan, (c) 2016
// MIT license

#ifndef __OUTSTREAM_H
#define __OUTSTREAM_H
#include <string>
#ifndef OLD_STD_CPP
#include <initializer_list>
#else
#define nullptr NULL
#include <errno.h>
#endif
#include <stdio.h>
#include <stdarg.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

namespace stream {

class Writer;

class Writeable {
public:
   virtual void write_to(Writer&,const char*) const = 0;
   std::string to_string(const char* fmt = nullptr);
};

class Writer {
protected:
    FILE *out;
    char sepc;
    bool eoln;
    bool owner;
    char old_sepc;
    char next_sepc;

    virtual void write_char(char ch);
    virtual void write_out(const char *fmt, va_list ap);
    virtual void put_eoln();

    void sep_out();
    Writer& formatted_write(const char *def, const char *fmt,...);

public:
    Writer(FILE *out, char sep=0);
    Writer(const char *file);
    Writer(const std::string& file);
    Writer(const Writer& w, char sepc);

    ~Writer();

    FILE *stream() { return out; }

    operator bool () { return out != nullptr; }
    std::string error();

    void close();
    Writer& set(FILE *nout);

    Writer& fmt(const char *fmtstr,...);

    Writer& sep(int ch = 0);
    char reset_sep(char sep=0);
    Writer& restore_sep(char sepr);

    Writer& operator() (const char *s, const char *fmt=nullptr) {
        return formatted_write("%s",fmt,s);
    }

    Writer& operator() (const std::string& s, const char *fmt=nullptr) {
        return (*this)(s.c_str(),fmt);
    }

    Writer& operator() (int i, const char *fmt=nullptr) {
        return formatted_write("%d",fmt,i);
    }

    Writer& operator() (char ch, const char *fmt=nullptr) {
        if (ch == '\n') return (*this)();
        return formatted_write("%c",fmt,ch);
    }

    Writer& operator() (uint64_t i, const char *fmt=nullptr) {
        return formatted_write("%" PRIu64,fmt,i);
    }

    Writer& operator() (int64_t i, const char *fmt=nullptr) {
        return formatted_write("%" PRIi64,fmt,i);
    }

    Writer& operator() (double x, const char *fmt=nullptr) {
        return formatted_write("%g",fmt,x);
    }

    Writer& operator() (float x, const char *fmt=nullptr) {
        return (*this)((double)x,fmt);
    }

    Writer& operator() (void *p, const char *fmt=nullptr) {
        return formatted_write("%p",fmt,p);
    }

    Writer& operator() (const Writeable& w, const char *fmt=nullptr) {
       w.write_to(*this,fmt);
       return *this;
    }

    Writer& operator() (const Writeable* w, const char *fmt=nullptr) {
       w->write_to(*this,fmt);
       return *this;
    }

    Writer& operator() ();

    virtual Writer& flush();

    template <class T, class S>
    Writer& operator() (std::pair<T,S> pp, const char *fmt=nullptr) {
        sep_out();
        char osep = reset_sep();
        (*this)(pp.first,fmt)(':')(pp.second,fmt);
        return restore_sep(osep);
    }

    template <class It>
    Writer& operator() (It start, It finis, const char *fmt=nullptr, char sepr=' ') {
        sep_out();
        char osep = reset_sep(sepr);
        for(; start != finis; ++start) {
            (*this)(*start,fmt);
        }
        return restore_sep(osep);
    }

    template <class It>
    Writer& operator() (It start, It finis, char sepr) {
        return (*this)(start,finis,nullptr,sepr);
    }

#ifndef OLD_STD_CPP
    template <class T>
    Writer& operator() (const std::initializer_list<T>& arr, const char *fmt=nullptr, char sepr=' ') {
        return (*this)(arr.begin(),arr.end(),fmt,sepr);
    }
#endif

};

extern Writer outs;
extern Writer errs;

class StrWriter: public Writer {
    std::string s;
public:
    StrWriter(char sepr=0, size_t capacity = 0);

    std::string str() { return s; }
    operator std::string () { return s; }
    void clear() { s.clear(); }

    virtual void write_char(char ch);
    virtual void write_out(const char *fmt, va_list ap);
    virtual Writer& flush() { return *this; }
};

class BufWriter: public Writer {
    char *P;
    char *P_end;
public:
    BufWriter(char *buff, int size, char sep=0);

    virtual void write_char(char ch);
    virtual void write_out(const char *fmt, va_list ap);
    virtual Writer& flush() { *P++ = '\0'; return *this; }
};

typedef const char *str_t_;
const str_t_ hex_u="X";
const str_t_ hex_l="x";
const str_t_ quote_d="S";
const str_t_ quote_l="s";

} // namespace stream


#endif
