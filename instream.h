#ifndef INSTREAMS_H
#define INSTREAMS_H
#include <stdio.h>
#include <stdarg.h>
#include <string>

class Reader {
protected:
   FILE *in;
   bool owner;
   int fpos;
   int pos;
   int bad;
   std::string err_msg;

public:
   struct Error {
      int errcode;
      std::string msg;
      long pos;
      
      operator bool () { return errcode != 0; }
   };

   Reader(FILE *in);
   Reader(const char *file, const char *how="r");
   Reader(const std::string& file, const char *how="r");
   ~Reader();
   void close();
   
   bool fail();   
   operator bool ();
   std::string error();
   
   void set(FILE *new_in, bool own);
   bool open(const std::string& file, const char *how="r");
   
   virtual void close_handle();
   virtual int read_fmt(const char *fmt, va_list ap);
   virtual char *read_raw_line(char *buff, int buffsize);
   virtual size_t read(void *buff, int buffsize);   
   
   Reader& formatted_read(const char *ctype, const char *def, const char *fmt, ...);

   virtual long getpos();
   virtual void setpos(long p, char end='^');
   
   int read_line(char *buff, int buffsize);
   bool readall (std::string& s);
   Reader& getfpos(int& p);
   
   Reader& operator() (Error& err);
   Reader& operator() (int &i,const char *fmt = nullptr);
   Reader& operator() (double &i,const char *fmt = nullptr);
   Reader& operator() (char &i,const char *fmt = nullptr);
   Reader& operator() (std::string &s,const char *fmt = nullptr);
   Reader& operator() (const char *extra);
   Reader& operator() ();
   
   Reader& getline(std::string& s);

   Reader& skip(int lines=1);
   
   template <class C>
   Reader& getlines(C& c, unsigned int lines=-1) {
      if (fail()) return *this;
      std::string tmp;
      unsigned int i = 0; 
      while (i < lines && getline(tmp)) {
         c.push_back(tmp);
         ++i;
      }
      return *this;
   }

};

extern Reader ins;

class CmdReader: public Reader {
public:
   CmdReader(std::string cmd, std::string extra="");
   
   bool operator == (std::string s);
   bool operator != (std::string s);

   virtual void close_handle();

   operator std::string ();   
};

const std::string CMD_OK = "> /dev/null && echo OK";
const std::string CMD_RETCODE = "> /dev/null || echo $?";

class StrReader: public Reader {
protected:
   const char * pc;
   size_t size;
public:
   StrReader(const std::string& s);
   StrReader(const char *pc);

   virtual int read_fmt(const char *fmt, va_list ap);
   virtual char *read_raw_line(char *buff, int buffsize);
   virtual long getpos();
   virtual void setpos(long p, char end='^');
};
#endif


