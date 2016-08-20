// A simple print function for C++11
#include "print.h"
using namespace std;
using namespace stream;

int main()
{
   int answer = 42;
   string who = "world";
   double x = 23;

   print("hello",who,"answer",answer,'\n');

   print("answer in hex",fmt(answer,hex_u),fmt(x,"%5.2f"))('\n');

   return 0;
}
