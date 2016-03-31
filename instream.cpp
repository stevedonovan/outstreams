#include "instream.h"
#include <errno.h>
#include <string.h>

const int line_size = 256;

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

int Reader::error_code() {
    return bad;
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
        if (errno != 0) { // we remain in hope
            err_msg = strerror(errno);
            bad = errno;
        } else { // but invariably it just means EOF
            err_msg = "EOF reading " + std::string(ctype);
            bad = EOF;
        }            
    } else
    if (res != 1 && *ctype != 'S') {
         std::string chars;
         (*this)(chars,"%5s");
         err_msg = "error reading " + std::string(ctype) + " '" + std::string(fmt) + "' at  '" + chars + "'";
         bad = 1;
    }
    pos += fpos;
    va_end(ap);
    return *this;
}

Reader& Reader::conversion_error(const char *kind, uint64_t val, bool was_unsigned) {
   bad = ERANGE;
   err_msg = "error converting " + std::string(kind) + " out of range ";
   if (was_unsigned) {
      err_msg += std::to_string(val);
   } else {
      err_msg += std::to_string((int64_t)val);
   }
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

Reader& Reader::operator() (double &i,const char *fmt) {
  return formatted_read("double","%lf%n",fmt,&i,&fpos);
}

Reader& Reader::operator() (float &i,const char *fmt) {
  return formatted_read("double","%f%n",fmt,&i,&fpos);
}

Reader& Reader::operator() (int64_t &i,const char *fmt) {
  return formatted_read("int64","%" SCNd64 "%n",fmt,&i,&fpos);
}

Reader& Reader::operator() (uint64_t &i,const char *fmt) {
  return formatted_read("uint64","%" SCNu64 "%n",fmt,&i,&fpos);
}

Reader& Reader::operator() (int32_t &i,const char *fmt) {
   int64_t val;
   if (! (*this)(val)) return *this;
   if (val < INT32_MIN || val > INT32_MAX) return conversion_error("int32",val,false);
   i = (int16_t)val;
   return *this;
  //~ return formatted_read("int32","%" SCNd32 "%n",fmt,&i,&fpos);
}

Reader& Reader::operator() (uint32_t &i,const char *fmt) {
   uint64_t val;
   if (! (*this)(val)) return *this;
   if (val > UINT32_MAX) return conversion_error("uint32",val,true);
   i = (uint32_t)val;
   return *this;
  //~ return formatted_read("uint32","%" SCNu32 "%n",fmt,&i,&fpos);
}

Reader& Reader::operator() (int16_t &i,const char *fmt) {
   int64_t val;
   if (! (*this)(val)) return *this;
   if (val < INT16_MIN || val > INT16_MAX) return conversion_error("int16",val,false);
   i = (int16_t)val;
   return *this;
  //~ return formatted_read("int16","%" SCNd16 "%n",fmt,&i,&fpos);
}

Reader& Reader::operator() (uint16_t &i,const char *fmt) {
   uint64_t val;
   if (! (*this)(val)) return *this;
   if (val > UINT16_MAX) return conversion_error("uint16",val,true);
   i = (uint16_t)val;
   return *this;
  //~ return formatted_read("uint16","%" SCNu16 "%n",fmt,&i,&fpos);
}

Reader& Reader::operator() (char &i,const char *fmt) {
  return formatted_read("char","%c%n",fmt,&i,&fpos);
}

Reader& Reader::operator() (uint8_t &i,const char *fmt) {
   uint64_t val;
   if (! (*this)(val)) return *this;
   if (val > UINT8_MAX) return conversion_error("uchar",val,true);
   i = (uint8_t)val;
   return *this;
   //return formatted_read("uchar","%" SCNd8 "%n",fmt,&i,&fpos);
}

Reader& Reader::operator() (std::string &s,const char *fmt) {
  if (fail()) return *this;
  char buff[line_size];
  if (! formatted_read("string","%s%n",fmt,buff,&fpos)) return *this;
  s = buff;
  return *this;
}

Reader& Reader::operator() (const char *extra) {
  char buff[line_size];
  strcpy(buff,extra);
  strcat(buff,"%n");
  return formatted_read("S",extra,nullptr,&fpos);
}

Reader& Reader::operator() () {
  return skip();
}

Reader& Reader::getline(std::string& s) {
  if (fail()) return *this;
  char buff[line_size];
  int n = read_line(buff,line_size);
  if (n > 0) {
     buff[n-1] = 0; // trim \n
     s = buff;
  }
  return *this;
}

Reader& Reader::skip(int lines) {
  if (fail()) return *this;
  int i = 0;
  char buff[line_size];
  while (i < lines && read_line(buff,line_size) > 0) {
     ++i;
  }
  return *this;
}

Reader::LineInfo Reader::getlineinfo (long p) {
    if (p == -1)
        p = pos;
    setpos(0,'^');
    pos = 0;
    char buff[line_size];
    int last_pos, lineno = 1;
    read_line(buff,line_size);
    while (pos < p) {
        lineno++;
        last_pos = pos;
        read_line(buff,line_size);
    }
    int col = p - last_pos;
    setpos(p,'^');
    return {lineno, col};
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
    if (pos >= size) {
        bad = 1; return 0;
    }
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



