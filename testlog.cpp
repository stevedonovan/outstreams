#include "logger.h"
using namespace std;
using namespace logging;

int main(int argc, char **argv)
{
    string log_config = "log4cpp.properties";
    initialize_logging(log_config);
    
    error("hello")(42)();
    warn("this is a warning")();
    debug("won't appear in default setting")();
    
    return 0;
}