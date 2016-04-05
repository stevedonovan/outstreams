#include "logger.h"
#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/Priority.hh>
using namespace std;

static log4cpp::Category* plogger;

class LogWriter: public StrWriter {
    log4cpp::Priority::PriorityLevel level;
public:
    LogWriter(log4cpp::Priority::PriorityLevel level) : StrWriter(' '),level(level) {}
        
    virtual void put_eoln() {
        plogger->log(level,"%s",str().c_str());
        clear();
    }
};

namespace logging {
    bool initialize_logging(string log_properties) {
        try {    
           log4cpp::PropertyConfigurator::configure(log_properties);    
           plogger = &(log4cpp::Category::getInstance("testlog"));    
           atexit (log4cpp::Category::shutdown); 
           return true;
        } catch(log4cpp::ConfigureFailure& err)  {
           errs("log4cpp")(err.what())();
           return false;       
        }
        
        
    }
    Writer& error = *new LogWriter(log4cpp::Priority::ERROR);
    Writer& warn = *new LogWriter(log4cpp::Priority::WARN);
    Writer& info = *new LogWriter(log4cpp::Priority::INFO);
    Writer& debug = *new LogWriter(log4cpp::Priority::DEBUG);
    Writer& notice = *new LogWriter(log4cpp::Priority::NOTICE);
        
}