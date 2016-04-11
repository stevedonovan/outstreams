#include "outstream.h"
#include <vector>
using namespace std;
using namespace stream;

class Point: public Writeable {
    int X;
    int Y;
public:
    Point(int X, int Y) : X(X),Y(Y) { }

    virtual void write_to(Writer& out, const char *) const {
        out.fmt("(%d,%d)",X,Y);
    }
};

 void custom_type_point() {
    outs("*custom Point output")();
    Point P(10,100);
    outs(P)('!')('\n');
}

void writing_iterator_range() {
    outs("*iterator range")();
     // the convenience of initializer lists
    /// empty () is equivalent to ('\n')
    outs({10,20,30})();

    float arr[] {1.2,1.5,2.1};
    outs(arr,arr+3)();

    // can specify the FORMAT and the SEPARATOR for a range
    string s = "\xFE\xEE\xAA";
    outs(s.begin(),s.end(),"X",0)("and that's all")();

    vector<int> vi {10,2,5,11,4};
    outs("bork")(vi.begin(),vi.end(),"%#X",',')("heh")();

    outs(vi.begin(),vi.end(),',')();

    // write out a quick little burst of json
    // The hex_u format (double-quoted) only applies to text fields
    // so can be safely passed for all times. q means single-quoted
    outs('{')({
        make_pair("hello",42),
        make_pair("dolly",99),
        make_pair("frodo",111)
    },hex_u,',')('}')();

}

int writing_to_file() {
    outs("*writing to file")();
    double x = 1.1, y = 2.0, z = 2.8;
    Writer("test.txt").sep(',')(x)(y)(z)();
    return system("cat test.txt");
}

void building_strings() {
    outs("*building strings")();
    StrWriter sw(' ');
    sw(42)("and")(3.4);

    outs(sw.str())();
}

void macro_magic() {
    outs("*macro magic")();
    #define VA(var) (#var)(var,"Q")
    #define VX64(var) (#var)(var,"%#016" PRIX64)
    string full_name = "bonzo the dog";
    uint64_t  id_number = 666;

    outs VA(full_name) VA(id_number)   ();

    outs VX64(id_number) ();

    #undef VA
    #undef VX64
}

void outstream_tests() {
    outs("*basic tests")();
    // not initially true
    outs.sep(' ');

    // outstreams are good for quick dumps of variable values
    int i = 10;
    string s = "hello you";
    double x = 3.1412;

    outs(s)(i)(x)("finis")('\n');

    writing_iterator_range();

    writing_to_file();

    building_strings();

    custom_type_point();

    macro_magic();

 }


int main(int argc, char **argv)
{
    outstream_tests();
    return 0;
}
