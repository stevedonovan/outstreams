#ifndef _LOGGER_H
#define _LOGGER_H
#include "outstream.h"
namespace logging {
    bool initialize_logging(std::string log_properties);
    
    extern Writer& error;
    extern Writer& warn;
    extern Writer& notice;
    extern Writer& info;
    extern Writer& debug;
}
#endif