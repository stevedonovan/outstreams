## An alternative to either stdio or iostreams

The standard C++ iostreams library is generally easier to use
than stdio, if you don't particularly care about the shape of the
output and have no resource constraints.  No ugly `.c_str()` and
the nasty `"%" PRIu64`-style formats needed to consistently output integers
across different platforms.

Embedded platforms can benefit from the better program structuring and abstraction
capabilities of C++ over C, although as always you have to know what
you're doing.  Even so, embedded developers still prefer stdio. iostreams is
a large and sprawling library, and it represents a measurable cost in
systems with limited program space. Even `std::string` is now entangled
with `iostreams`. I've written about this unfortunate [state of affairs] before,
but the standard is the standard.

Apart from the bloat,
getting precise output with iostreams is harder than with stdio - once you've
marshalled all the necessary manipulators to force the correct format, the
result is more verbose.  Then there are some misfeatures; a lot of people
don't know that `endl` is more than just a line-end; it _flushes the stream_.
They wonder why their C++ programs have such bad I/O performance
and end up on Stackoverflow.

## Basic outstreams: Writer and StrWriter

Here are the basic capabilties of outstreams. The `Writer` class can be
initialized with a standard stream (like `stdout` and `stderr`, or a file name.
The object maintans ownership of the `FILE*` in this case, so destruction
closes the file.  `Writer` is overridable so constructing strings is
straightforward, avoiding the ugliness of naked `sprintf`. Everything is inside
the `stream` namespace, although I'm careful to avoid collisions with `std`
 - [this table](http://www.bobarcher.org/software/C++11%20Symbols.html) is
 an excellent resource.

```cpp
using namespace stream;
double x = 1.2;
const char *msg = "hello";
int i = 42;

outs.fmt("%g %s %d",x,msg,i);
// --> 1.2 hello 42

// dtor closes file
Writer("/tmp/myfile").fmt("answer = %d\n",i);

// StrWriter converts to std::string
string s = StrWriter().fmt("greeting is '%s'",msg);
// s = "greeting is 'hello'"
```
## Using the Call Operator for Fun and Profit

These are modest advantages, but outstreams goes further.
A char separator can be defined between fields, and `operator()`
is overriden to mean 'write out a field"

```cpp
// assuming above declarations
outs.sep(' ');
// separator is 'sticky' - remains set until reset.

outs(x)(msg)(i)('\n\);
// --> 1.2 hello 42

outs(msg,"'%s'")('\n');
// --> 'hello'
```

The advantage of the call operator is that it may have extra optional arguments,
in this case an explicit format.  "'%s'" is a bit clumsy, so `quote_s` can be used here
(`quote_d` for double quotes). The real power of these constants is when applied to
multiple fields, since only the string fields will obey it.

The standard algorithm `for_each` can be used to dump out a container, since
it expects a _callable.

```cpp
vector<int> vi {10,20,30};
for_each(vi.begin(),vi.end(),outs);
outs();
// ---> 10 20 30
```

## Support for Containers

The above idiom is common enough that there is a  template version of
`operator()` for displaying the elements of a container. Note that the
sticky separator is not used - since we cannot guarantee it will be suitable; you provide
a separator in addition to the usual format.  The `trace()` adapter takes two forms;
the first wraps a _container_ and the second an explicit iterator range.

```cpp
// default format, list separator
outs(range(vi),0,',')('\n');
// --> 10,20,30

outs(range(vi),"'%d'",' ')('\n');
// --> '10' '20' '30'

int arr[] {10,20,30};
outs(range(arr,arr+3),0,' ')('\n');
// --> 10 20 30

string s = "\xFE\xEE\xAA";
outs(range(s),hex_u)("and that's all")('\n');
// --> FEEEAA and that's all
```
`hex_u` and `hex_l` are constants for hex format; if you try the obvious '%X' you
will notice the problem; without the length modifier the value is printed out as an `int`
and sign-extended.

In the C++11 standard there is a marvelous class called `std::intializer_list`
which is implicitly used in bracket initialization of containers. We overload it
directly to support brace lists of objects of the same type; there is also an
overload for `std::pair`

```cpp
outs({10,20,30})();
// --> 10 20 30

// little burst of JSON - note "Q" format!
outs('{')({
    make_pair("hello",42),
    make_pair("dolly",99),
    make_pair("frodo",111)
},quote_d,',')('}')();
// --> { "hello":42,"dolly":99,"frodo":111 }

// which will also work when iterating over `std::map`
```

## Overriding Writer

Here is `StrWriter` presented in inline form for convenience.
`write_char` is used to output any separator and '\n' at the end
of lines; `write_out` takes a format and a `va_list`.

```cpp
static char buf[128];

class StrWriter: public Writer {
    std::string s;
public:
    StrWriter(char sepr=0, size_t capacity = 0)
    : Writer(stderr)
     {
        if (capacity != 0) {
            s.reserve(capacity);
        }
        sep(sepr);
    }

    std::string str() { return s; }
    operator std::string () { return s; }
    void clear() { s.clear(); }

    virtual void write_char(char ch) {
        s += ch;
    }

    virtual void write_out(const char *fmt, va_list ap) {
        int nch = vsnprintf(buf,sizeof(buf),fmt,ap);
        s.append(buf,nch);
    }
    
    virtual Writer& flush() { return *this; }
};
```
No doubt this can be improved (keep a resizable line buffer) but this is
intended as a humble 'serving suggestion'.

The more common form of extension in iostreams is to teach it to output
your own types, simply by adding yet another overload for `operator<<`.
Alas, `operator()` may only be defined as a method of a type. In the first
version of this library I had an ugly and inconsistent scheme, and finally
settled on an old-fashioned solution; your types must implement a `Writeable` interface:

```cpp
struct Point: public Writeable {
    int X;
    int Y;

    Point(int X, int Y) : X(X),Y(Y) {}

    void write_to(Writer& w, const char *format) {
        w.fmt("(%d,%d)",p.X,p.Y);
    }

};

 ...

Point P(10,100);
outs(P)('\n');
// --> (10,20)

```

## Output in a Hurry

There are some kinds of debugging which can only be done by selectively
dumping out values, for instance real-time or heavily-threaded code.
outstreams provides a useful low-cost alternative for `printf` here where
the exact formatting isn't so important.

```cpp
#define VA(var) (#var "=")(var,quote_s)
...
string full_name;
uint64_t  id_number;
...
outs VA(full_name) VA(id_number)();
// --> full_name "bonzo the dog" id_number 666
...
#define VX64(var) (#var)(var,"%#016" PRIX64)
...
outs VX64(id_number) ();
// --> id_number 0X0000000000029A

// if the FILE* is NULL, then a Writer instance converts to false
// can say logs.set(nullptr) to switch off tracing
#define TRACE if (logs) logs ("TRACE")
TRACE VA(full_name) ();

```
Speaking of speed, how much speed are we losing relative to stdio?

`speedtest.cpp` writes a million lines to a file; each line consists
of five double values separated by spaces. Obviously we are
calling the underlying formatted write function a lot more, but
in the end it makes little difference, except that using iostreams seems
rather slower:

```
stdio 2720ms
outstreams 2937ms
iostreams 4541ms

```

## 'scanf' Considered Harmful

Many still like using the `printf` family of functions, but the reputation of
the `scanf` family is less certain, even among C programmers. The problem
is that handling errors with `scanf` is a mission.  Jonathan Leffler on Stackoverflow
expresses the conventional wisdom: "I don't use scanf() in production code; it is just
too damn hard to control properly". And (for that matter) neither have I.

However, there are some immediate advantages to doing a wrapper around
`fgets`. Here is the very useful 'all lines in a file' pattern.

```cpp
#include "instream.h"
...
string line;
Reader inf("instream.cpp");
while (inf.getline(line)) {
    ...
}
```
This is very much like the standard iostreams way of doing things; `Reader` objects
convert to a status `bool`, so this loop will end when we hit an error, which is _usually_
EOF.  The file handle will be closed when `inf` goes out of scope.

Another way to do this is use the template method `getlines`, which will work with
any container that understands `push_back`. This has a convenient optional argument
that is the maximum number of lines you wish to collect.

```cpp
vector<string> lines;
Reader inf("instream.cpp");
inf.getlines(lines);
```

There is a `read(void *,int)` for reading binary objects, and `readall(string&)` which
slurps in the whole file without needing to know its size. (`std::string` is a very
flexible data structure, if considered as a bunch of bytes, since its size does not
depend on any silly NUL ending in the data.)

## A Symmetrical approach to Wrapping stdio Input

One way to explore an idea is to see how far you can push it. `Reader` overloads
`operator()` just like `Writer`:

```cpp
int i;
double x;
string s;

Reader rdr("input-test.txt");
// first line is '1 3.14 lines'
rdr (i) (x) (s);
```
Which is certainly compact!  But in an imperfect world, there are errors.

The approach I take is for `Reader` to note errors when they first occur and
thereafter perform _no_ input operations. So even if this file doesn't exist
no terrible things will happen. But you must check the error state afterwards!

```cpp
if (! rdr) {
    outs(rdr.error())();
}
```

This just gives you a descriptive error string. More details are available using
`Reader::Error` which is read as a pseudo-input-field.

```cpp
Reader::Error err;
if (! rdr (i) (x) (s) (err)) {
    outs(err.errcode)(err.msg)(err.pos)();
}
```
If the input line had a non-convertable field - say it is "1 xx3 lines" - then
the error message will look like "error reading double '%lf%n' 'xx'" and
`err.pos` will tell you where in the file this happened.  Although not strictly
necessary, it is useful to check the error as soon as possible, close to the
context where it happened.

## Reading Strings and the Output of Commands

`Reader` is overrideable, like `Writer`.  In particular, can use `StrReader` to parse
strings - this overrides `raw_read_line` and `read_fmt`.  I went to some trouble
to track position in the stream manually (using '%n') and not depend on the
underlying `FILE*`, precisely to make this specialization possible.

This is useful because C++ string handling is not very complete and instream
operations complement them well, just as with `std::istrstream`.  Strings come to
us from many sources that are not files.

`CmdReader` wraps `popen` and overrides `close_handle` so that `pclose`
is called on the handle after destruction. `stderr` is redirected to `stdout` so
that the stream captures all of the output, good or bad.

```cpp
vector<string> header_files;
CmdReader("ls *.h").getlines(header_files);
```
A common pattern is to invoke a simple command and capture the first line
of output. Can use `getline` but a convenient `line` method
is provided:

```cpp
string platform = CmdReader("uname").line();
```
There is no reliable way of getting the _actual error_ when using `popen`, so
workarounds are useful.  If it is a command where there is no output, or the output
is unneeded, then a shell trick is to silence all output and conditionally execute
a following command.

```cpp
if (CmdReader("true",cmd_ok).line() == "OK") {
    outs("true is OK");
}
if (CmdReader("false",cmd_ok).line() != "OK") {
    outs("false is not OK");
}

if (CmdReader("g++",cmd_retcode) == "4") {
    outs("no files provided")();
}

```

