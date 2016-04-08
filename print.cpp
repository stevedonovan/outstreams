// A simple print function for C++11
#include "outstream.h"
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

int main()
{
   int answer = 42;
   string who = "world";
     
   print("hello",who,"answer",answer)();
   
   return 0;
}
