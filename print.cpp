// A simple print function for C++11
#include "outstream.h"
#include <vector>
using namespace std;

template <typename T>
Writer& print(T v) {
   return outs(v);
}

template <typename T, typename... Args>
Writer& print(T first, Args... args) {
   print(first);
   return print(args...);
}

// pretty easy to override print for other types
template <typename T>
Writer& print(const vector<T>& v) {
   return outs('{')(v.begin(),v.end(),"Q",',')('}');
}

template <typename T>
struct Fmt_ {
   const T& v;
   const char *fmt;
   Fmt_(const T& v, const char *fmt) : v(v),fmt(fmt) {}
};

template <typename T>
Writer& print(const Fmt_<T>& vf) {
   return outs(vf.v,vf.fmt);
}

template <typename T>
Fmt_<T> fmt(const T& v, const char *f) {
   return Fmt_<T>(v,f);
}

int main()
{
   int answer = 42;
   string who = "world";
     
   print("hello",who,"answer",answer)();
   
   vector<string> ss {"hello","dolly"};
   print(ss)();
   
   print("answer as hex",fmt(answer,"X"))();
 
   return 0;
}
