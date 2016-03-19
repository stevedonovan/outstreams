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
straightforward, avoiding the ugliness of naked `sprintf`.

```cpp
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

outs(x)(msg)(i)();
// --> 1.2 hello 42

outs(msg,"'%s'")();
// --> 'hello'
```
The last statement has _exactly_ the same formatting as the first statement; the
empty call outputs '\n' and sets the eoln state so that we will not get
a separator on the start of the next line.

The advantage of the call operator is that it may have extra optional arguments,
in this case an explicit format.  "'%s'" is a bit clumsy, so "q" can be used here 
("Q" for double quotes). The real power of 'q/Q' comes when it is applied to
multiple fields, since only the string fields will obey it.

The standard algorithm `for_each` can be used to dump out a container, since
it expects a callable.

```cpp
vector<int> vi {10,20,30};
for_each(vi.begin(),vi.end(),outs);
outs();
// ---> 10 20 30
```

## Support for Containers

The above idiom is common enough that there is a  template version of 
`operator()` for displaying the elements of a container. Note that the 
sticky separator is not used - since we cannot guarantee it will be suitable.

```cpp
// default format, list separator
outs(vi.begin(),vi.end(),',')();
// --> 10,20,30

outs(vi.begin(),vi.end(),"'%d'",' ')();
// --> '10' '20' '30'

string s = "\xFE\xEE\xAA";
outs(s.begin(),s.end(),O_HEX_BYTE)("and that's all")();
// --> FEEEAA and that's all
```
`O_HEX_BYTE` is "%hhX" - you need an explicit length to treat the
fields as signed char and not their promoted value as signed int. This is
another strength of individually specifying the format for an item; it's
easy to give the format a symbolic name which is easier to remember.

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
},"Q",',')('}')();
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
    StrWriter(char sepr=0, size_t capacity = 0) {
        if (capacity != 0) {
            s.reserve(capacity);
        }
        sep(sepr);
    }

    std::string str() { return s; }
    operator std::string () { return s; }

    virtual void write_char(char ch) {
        s += ch;
    }
    
    virtual void write_out(const char *fmt, va_list ap) {
        int nch = vsnprintf(buf,sizeof(buf),fmt,ap);
        s.append(buf,nch);    
    }
};
```
No doubt this can be improved (keep a resizable line buffer) but this is 
intended as a humble 'serving suggestion'.

The more common form of extension in iostreams is to teach it to output
your own types, simply by adding yet another overload for `operator<<`.
`operator()` may only be defined as a method of a type; a 
overloadable plain function called `Writer_streamer` is defined for this 
purpose.

```cpp
struct Point {
    int X;
    int Y;
};

 void Writer_streamer(Writer& w, const Point& p) {
     w.fmt("(%d,%d)",p.X,p.Y);
 }
 ...

Point P{10,100};
outs("point ",P)();
// --> point (10,20)

...

template <class T>
void Writer_streamer(Writer& w, const vector<T>& v) {
     w(v.begin(),v.end());
}

vector<int> vi {10,2,5,11,4};
vector<double> vd {10.1,20.5,30.05};
outs("ints ",vi)("doubles ",vd)();
// --> ints 10 2 5 11 4 doubles 10.1 20.5 30.05

```
Alas, but my attempts to completely define `operator()` as a template
were frustrated by the specificity of template type deduction. So the custom
template operator has the signature `(const char *,const T& v)`.
It would be extremely cool if someone could show the way
forward here.

## Output in a Hurry

There are some kinds of debugging which can only be done by selectively
dumping out values, for instance real-time or heavily-threaded code.
outstreams provides a useful low-cost alternative for `printf` here where
the exact formatting isn't so important.

```cpp
#define VA(var) (#var "=")(var,"Q")
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
in the end it makes little difference:

```

```
