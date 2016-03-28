// Lightweight operator() overloading stdio wrapper
// Steve Donovan, (c) 2016
// MIT license
#include "outstream.h"
using namespace std;

void Writer::write_char(char ch) {
    fputc(ch,out);
}

void Writer::write_out(const char *fmt, va_list ap) {
    vfprintf(out,fmt,ap);
}

Writer& Writer::fmt(const char *fmtstr,...) {
    va_list ap;
    va_start(ap,fmtstr);
    write_out(fmtstr,ap);
    va_end(ap);
    return *this;
}

void Writer::sep_out() {
    if (eoln) {
        eoln = false;
    } else {
        if (sepc != next_sepc) {
            next_sepc = sepc;
        }
        if (next_sepc) {
            write_char(next_sepc);
        }
        next_sepc = sepc;
    }
}

Writer& Writer::formatted_write(const char *def, const char *fmt,...) {
    if (fmt!=nullptr && fmt[1]==0) {
        if (def[1] == 's') {
            if (fmt[0] == 'q') fmt = "'%s'"; else
            if (fmt[0] == 'Q') fmt = "\"%s\"";
        } else {
            fmt = nullptr;
        }
    }
    sep_out();
    va_list ap;
    va_start(ap,fmt);
    write_out(fmt ? fmt : def,ap);
    va_end(ap);
    return *this;
}

Writer::Writer(FILE *out)
    : out(out),sepc(0),eoln(true),owner(false),old_sepc(0),next_sepc(0)
{
}

Writer::Writer(const char *file)
    : out(fopen(file,"w")), sepc(0), eoln(true), owner(true),old_sepc(0),next_sepc(0)
{
}

Writer::Writer(const string& file)
    : out(fopen(file.c_str(),"w")), sepc(0), eoln(true), owner(true),old_sepc(0),next_sepc(0)
{
}

Writer::Writer(const Writer& w, char sepc)
    : out(w.out),sepc(sepc),eoln(w.eoln),owner(false),old_sepc(0),next_sepc(0)
{
}

Writer::~Writer() {
    close();
}

void Writer::close() {
    if (owner && out != nullptr) {
        fclose(out);
    }
}

Writer& Writer::set(FILE *nout) {
    close();
    out = nout;
    return *this;
}

Writer& Writer::sep(int ch) {
    if (ch == -1) {
        sepc = old_sepc;
    } else {
        old_sepc = sepc;
        sepc = ch;
    }
    return *this;
}

char Writer::reset_sep(char sep) {
    char res = sepc;
    sepc = sep;
    eoln=true;
    
    return res;
}

Writer& Writer::restore_sep(char sepr) {
    sep(sepr);
//        sep_out();
    return *this;
}

Writer& Writer::operator() () {
    eoln = true;
    write_char('\n');
    next_sepc = 0;
    return *this;
}

Writer& Writer::flush() {
    fflush(out);
    return *this;
}

Writer outs(stdout);
Writer errs(stderr);

static char buf[128];

StrWriter::StrWriter(char sepr, size_t capacity) : Writer((FILE *)nullptr) {
    if (capacity != 0) {
        s.reserve(capacity);
    }
    sep(sepr);
}

void StrWriter::write_char(char ch) {
    s += ch;
}

void StrWriter::write_out(const char *fmt, va_list ap) {
    int nch = vsnprintf(buf,sizeof(buf),fmt,ap);
    s.append(buf,nch);
}

//// FINIS

