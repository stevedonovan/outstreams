// outstreams works fine with older standard
// (without intializer lists)
#include "outstream.h"
#include <vector>
using namespace std;

int main()
{

   string who = "world!";
   outs("hello")(who)("answer is")(42)();
   
   vector<int> vi;
   vi.push_back(10);
   vi.push_back(20);
   vi.push_back(30);
   outs(vi.begin(),vi.end(),',')();
   
   float floats[] = {1.2,4.2,5};
   StrWriter sw;
   sw(floats,floats+3,' ');
   outs(sw.str())();
   return 0;
}