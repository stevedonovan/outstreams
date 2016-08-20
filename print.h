// Lightweight operator() overloading stdio wrapper
// Steve Donovan, (c) 2016
// MIT license
#ifndef __OUTSTREAM_PRINT_H
#define __OUTSTREAM_PRINT_H
#include "outstream.h"
namespace stream {
   template <typename T>
    struct Fmt_ {
        const T& v;
        const char *fmt;
        Fmt_(const T& v, const char *fmt) : v(v),fmt(fmt) {}
   };

    template <typename T>
    Fmt_<T> fmt(const T& v, const char *f) {
        return Fmt_<T>(v,f);
    }

    template <typename T>
    Writer& print(T v) {
        return outs(v);
    }

    template <typename T>
    Writer& print(const Fmt_<T>& vf) {
        return outs(vf.v,vf.fmt);
    }

    template <typename T, typename... Args>
    Writer& print(T first, Args... args) {
        print(first);
        return print(args...);
    }

}
#endif