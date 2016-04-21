// outstreams works fine with older standard
// (without intializer lists)
#include "outstream.h"
#include <vector>
using namespace std;
using namespace stream;

int main()
{

   string who = "world!";
   outs("hello")(who)("answer is")(42)();
   
   vector<int> vi;
   vi.push_back(10);
   vi.push_back(20);
   vi.push_back(30);
   outs(range(vi),0,',')();
   
   
   float floats[] = {1.2,4.2,5};
   StrWriter sw;
   sw(range(floats,floats+3),0,' ');
   outs(sw.str())();
   
   char buff[1024];
   BufWriter bw(buff,sizeof(buff));
   bw(range(floats,floats+3),0,' ')(" and that's it")('\0');
   outs(buff)();
   
   return 0;
}
