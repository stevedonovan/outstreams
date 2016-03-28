#include "instream.h"
#include <errno.h>
#include <string.h>

Reader::Reader(FILE *in) 
  : in(in), owner(false),fpos(0),pos(0),bad(0)
{
}

Reader::Reader(const char *file, const char *how)
  : in((FILE*)nullptr), owner(true),fpos(0),pos(0) 
{
    open(file,how);
}

Reader::Reader(const std::string& file, const char *how)
  : in((FILE*)nullptr), owner(true),fpos(0),pos(0) 
{
    open(file,how);
}

Reader::~Reader() {
  close();   
}

void Reader::close() {
  if (owner && in != nullptr) {
     close_handle();
     in = nullptr;
  }
}

bool Reader::fail() {
    return bad != 0;
}

Reader::operator bool () {
  return ! fail();
}   

std::string Reader::error() {
  return err_msg;
}

void Reader::set(FILE *new_in, bool own) {
  close();
  in = new_in;
  owner = own;
  pos = 0;
  bad = in == nullptr;
}

bool Reader::open(const std::string& file, const char *how) {
    in = fopen(file.c_str(),how);
    bad = errno;
    if (bad) {
        err_msg = strerror(errno);
    }
    return ! bad;
}

 void Reader::close_handle() {
    fclose(in);
}

 int Reader::read_fmt(const char *fmt, va_list ap) {
    return vfscanf(in,fmt,ap);
}

 char *Reader::read_raw_line(char *buff, int buffsize) {
    return fgets(buff,buffsize,in);
}

 size_t Reader::read(void *buff, int buffsize) {
  if (fail()) return 0;
  size_t sz = fread(buff,1,buffsize,in);
  ((char*)buff)[sz] = 0;
  return sz;
}

Reader& Reader::formatted_read(const char *ctype, const char *def, const char *fmt, ...) {
  if (fail()) return *this;
  va_list ap;
  va_start(ap,fmt);
  if (fmt == nullptr) {
     fmt = def;
  }
  int res = read_fmt(fmt,ap);
  if (res == EOF) {
     err_msg = strerror(errno);
     bad = errno;
  } else
  if (res != 1 && *ctype != 'S') {
     std::string chars;
     (*this)(chars,"%5s");
     err_msg = "error reading " + std::string(ctype) + " '" + std::string(fmt) + "' at  '" + chars + "'";
     bad = 1;
     //~ fprintf(stderr,"error for format '%s' at fpos %d %ld\n",fmt,pos,streampos());
     
  }
  pos += fpos;
  va_end(ap);
  return *this;
}

 long Reader::getpos() {
  return ftell(in);
}

 void Reader::setpos(long p, char end) {
  int whence = SEEK_SET;
  if (end == '$') {
     whence = SEEK_END;
  } else
  if (end == '.') {
     whence = SEEK_CUR;
  }
  fseek(in,p,whence);      
}

int Reader::read_line(char *buff, int buffsize) {
  char *res = read_raw_line(buff,buffsize);
  if (res == nullptr) {
     bad = 1;
     err_msg = "EOF";
     return 0;
  }
  int sz = res != nullptr ? strlen(res) : 0;
  pos += sz;
  return sz;      
}

bool Reader::readall (std::string& s) {
  char buff[512];
  size_t sz;
  s.clear();
  do {
     sz = read(buff,sizeof(buff));
     s.append(buff,sz);
  } while (sz == sizeof(buff));
  return errno != 0;
}

Reader& Reader::getfpos(int& p) {
  p = pos;
  return *this;
}

Reader& Reader::operator() (Reader::Error& err) {
  err.errcode = bad;
  err.msg = err_msg;
  err.pos = pos;
  return *this;
}

Reader& Reader::operator() (int &i,const char *fmt) {
  return formatted_read("int","%d%n",fmt,&i,&fpos);
}

Reader& Reader::operator() (double &i,const char *fmt) {
  return formatted_read("double","%lf%n",fmt,&i,&fpos);
}

Reader& Reader::operator() (char &i,const char *fmt) {
  return formatted_read("char","%c%n",fmt,&i,&fpos);
}

Reader& Reader::operator() (std::string &s,const char *fmt) {
  char buff[256];
  formatted_read("string","%s%n",fmt,buff,&fpos);
  s = buff;
  return *this;
}

Reader& Reader::operator() (const char *extra) {
  char buff[256];
  strcpy(buff,extra);
  strcat(buff,"%n");
  return formatted_read("S",extra,nullptr,&fpos);
}

Reader& Reader::operator() () {
  return skip();
}

Reader& Reader::getline(std::string& s) {
  if (fail()) return *this;
  char buff[256];
  int n = read_line(buff,sizeof(buff));
  if (n > 0) {
     buff[n-1] = 0; // trim \n
     s = buff;
  }
  return *this;
}

Reader& Reader::skip(int lines) {
  if (fail()) return *this;
  int i = 0;
  char buff[256];
  while (i < lines && read_line(buff,sizeof(buff)) > 0) {      
     ++i;
  }
  return *this;   
}

Reader ins(stdin);

CmdReader::CmdReader(std::string cmd, std::string extra) 
: Reader((FILE*)nullptr) {
  std::string cmdline = cmd + " 2>&1 " + extra;
  set(popen(cmdline.c_str(),"r"),true);
}

bool CmdReader::operator == (std::string s) {
    return ((std::string)*this) == s;
}

bool CmdReader::operator != (std::string s) {
    return ((std::string)*this) != s;
}

 void CmdReader::close_handle() {
    pclose(in);
}

CmdReader::operator std::string () {
    std::string tmp;
    getline(tmp);
    return tmp;
}

StrReader::StrReader(const std::string& s) : Reader((FILE*)nullptr), pc(s.c_str()) {
    size = s.size();
}

StrReader::StrReader(const char* pc) : Reader((FILE*)nullptr), pc(pc) {
    size = strlen(pc);
}
  
int StrReader::read_fmt(const char *fmt, va_list ap) {
    return vsscanf(pc+pos,fmt,ap);
}

 char *StrReader::read_raw_line(char *buff, int buffsize) {
    char *P = buff;
    const char *Q = pc+pos;
    int i = 1;
    while (*Q != '\n' && *Q != 0) {
        *P++ = *Q++;
        ++i;
    }
    *P = 0;
    return buff;
}

long StrReader::getpos() {
    return pos;
}

void StrReader::setpos(long p, char end) {
    if (end == '^') {
        pos = p;
    } else
    if (end == '.') {
        pos += p;
    } else 
    if (end == '$') {
        pos = size + p;
    }
}



