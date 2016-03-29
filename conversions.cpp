#include "outstream.h"
#include "instream.h"
using namespace std;

void test(const char *str) {
   Reader::Error err;
   int res = 0;
   double x = 0;
   string s;
   short b = 0;

   outs("input")(str)();
   //~ Writer("/tmp/tmp.tmp")(str);
   //~ Reader s1("/tmp/tmp.tmp");
   StrReader s1(str);
   s1 (res)(x)(s)(b)(err);
   outs("output")(res)(x)(s)(b)();
   outs("error")(err.msg)();

}

int main()
{
   outs.sep(' '); // DEFAULT

   test("10 4.2 boo 34");
   test("10 4.2 boo 344455555");
   test("10 x4.2 boo 23");
   test("x10 4.2 boo 52");
   test("10 4.2");

   return 0;
}