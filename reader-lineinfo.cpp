#include "instream.h"
#include "outstream.h"
using namespace std;

int main()
{
    outs.sep(' ');
    Reader rdr("instream.cpp");
    string tok;
    while (rdr(tok)) {
        if (tok == "Reader::getlineinfo") {
            // Reader::LineInfo Reader::getlineinfo (long p) {
            string type, mname;
            rdr (type)(mname); 
            outs("type")(type,"q")(",")("name")(mname,"q")();
            break;
        }
    }
    auto pos = rdr.getlineinfo();
    outs("line")(pos.line)("column")(pos.column)();
  
    return 0;
}