#include "outstream.h"
#include <fstream>
using namespace std;

double x1 = 1000,x2 = 1001,x3 = 1003,x4 = 1004,x5 = 1005;
const auto N = 1000000;

void testwrite_s(string file) {
    Writer w(file.c_str());
    w.sep(' ');

    for (int i = 0; i < N; i++) {
        w(x1)(x2)(x3)(x4)(x5)();
    }
}

void testwrite_f(string file) {
    FILE *out = fopen(file.c_str(),"w");

    for (int i = 0; i < N; i++) {
        fprintf(out,"%g %g %g %g %g\n",x1,x2,x3,x4,x5);
    }

    fclose(out);
}

void testwrite_i(string file) {
    ofstream out(file);
    
    for (int i = 0; i < N; i++) {
        out << x2 << ',' << x2 << ',' << x3
             << ',' << x4 << ',' << x5 << '\n';
    }    
    
}

int main(int argc, char **argv)
{
    testwrite_s("s.dat");
    testwrite_f("f.dat");
    testwrite_i("io.dat");
    return 1;
}
