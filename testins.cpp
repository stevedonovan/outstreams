#include "instream.h"
#include "outstream.h"
#include <vector>
using namespace std;
using namespace stream;

int main(int argc, char **argv)
{
    int i;
    double x = 1.2;
    int res = 0, N = 0;
    string s1,s2,s3;
    vector<string> lines;
    Reader::Error err;
    char ch = 'X';

    outs.sep(' ');

    outs("+++all lines from file matching some condition")();
    Reader inf("instream.cpp");
    while (inf.getline(s3)) {
        if (s3.find('#')==0)
            outs(s3)(eol);
    }
    // this will normally happen when inf goes out of scope
    inf.close();

    // alternatively, getlines will append lines using push_back
    // and an optional number of lines to grab can be set
    inf.open("instream.cpp");
    inf.getlines(lines,4);
    inf.close();
    outs(range(lines),quote_s)(eol);

    outs("+++read variables from file")();
    Reader rdr("input-test.txt");

    rdr(i)(x)(s1);
    outs(i)(x)(s1,"q")(eol);

    // this line fails since there's no good match for res
    // () means grab rest of line and continue...
    // (err) captures error state of instream
    rdr(i)(s1)(s2)()(res)(ch)(err);
    if (err) {
        outs("failed")(err.errcode)(err.msg)(eol);
    }
    outs(i)(s1)(s2)(res)(ch)(eol);


    outs("+++read input-test.txt contents")();
    string contents;
    Reader("input-test.txt").readall(contents);
    outs(contents)(eol);

    outs("+++read from string with errors")();
    StrReader sc(contents);
    sc()(i)(s1)(s2)()(res)(ch)(err);
    if (err) {
        outs("failed")(err.errcode)(err.msg)(eol);
    }
    outs(i)(s1)(s2)(res)(ch)(eol);

    outs("+++all header files in this directory")();
    lines.clear();
    CmdReader("ls *.h").getlines(lines);
    for (string L : lines) outs(L,quote_s)(eol);

    outs("+++file doesn't exist")();
    Reader b("bonzo.txt");
    b(res);
    if (! b) {
       outs("bonzo.txt doesn't exist")(b.error())(eol);
    }

    outs("+++CmdReader result available as string conversion")();
    string s = CmdReader("uname");
    outs("uname")(s)(eol);

    outs("+++More reliable way to execute commands silently and test errors")();
    if (CmdReader("true",CMD_OK) == "OK") {
        outs("true is OK")(eol);
    }
    if (CmdReader("false",CMD_OK) != "OK") {
        outs("false is not OK")(eol);
    }

    /*

   s = "one two   30";
   StrReader ss(s);
   ss(s1)(s2)(res);

   printf("'%s' '%s' '%s' %d\n",C(s1),C(s2),C(s3),res);
*/
   return 0;
}
