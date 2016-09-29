/***
* An example of a Reader which operates on input consisting
* of 'parts' returned from a iterator. You may then split the input
* string how you wish, and still use the reader for conversion.
*/

#include "instream.h"
#include "outstream.h"
#include <string.h>
using namespace std;
using namespace stream;

template <class It>
class PartsReader: public Reader {
    It start;
    It finish;
public:
    // note Reader(stderr) - so that the reader's FILE* isn't NULL
    PartsReader(It start, It finish)
     : Reader(stderr), start(start),finish(finish) {
     }
     
    // only method that needs to be overriden
    virtual int read_fmt(const char *fmt, va_list ap) {
        if (start == finish) {
            return 0;
        }
        const char *s = *start++;
        if (fmt[0] == '%' and fmt[1] == 's') {
            strncpy(va_arg(ap, char *),s,256);
            return 1;
        } else {
            return vsscanf(s,fmt,ap);
        }
    }

};

// a helper function, until C++17 class template type deduction
// arrives
template <class It>
PartsReader<It> make_parts_reader(It start, It finish) {
     return PartsReader<It>(start,finish);
}

int main()
{
    int n;
    double x;
    string s;   
    
    auto parts = {"42","5.2","hello"};
    auto rdr = make_parts_reader(parts.begin(),parts.end());
    
    rdr (n) (x) (s);
    
    outs(n)(x)(s)(eol);
    return 0;
}
