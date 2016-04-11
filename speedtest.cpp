#include "outstream.h"
#include <fstream>
#include <time.h>
using namespace std;
using namespace stream;

static uint64_t millisecs() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME,&ts);
    return 1000L*ts.tv_sec + ts.tv_nsec/1000000L;
}

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
        out << x1 << ' ' << x2 << ' ' << x3
             << ' ' << x4 << ' ' << x5 << '\n';
    }

}

typedef uint64_t U64;

int exec(const char *cmd) { return system(cmd); }

int main(int argc, char **argv)
{
    U64 start = millisecs();
    testwrite_f("f.dat");
    U64 diff = millisecs() - start;
    U64 stdio_ms = diff;
    outs("stdio ")(diff)("ms")();
    start = millisecs();
    testwrite_s("s.dat");
    diff = millisecs() - start;
    outs("outstreams ")(diff)("ms")();
    start = millisecs();
    testwrite_i("io.dat");
    diff = millisecs() - start;
    outs("iostreams ")(diff)("ms")();
    exec("rm f.dat s.dat io.dat");
    return 0;
}
