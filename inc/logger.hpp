#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <syslog.h>
#include <string>

class SysLogger {
    private:
        std::string logger_name;

    public:
        SysLogger(const std::string& name)
        : logger_name(name) {
            openlog(logger_name.c_str(), LOG_PID | LOG_CONS, LOG_USER);
        }
        
        ~SysLogger() {
            closelog();
        }
        
        void info(const std::string& message) {
            syslog(LOG_INFO, "%s", message.c_str());
        }
        
        void error(const std::string& message) {
            syslog(LOG_ERR, "%s", message.c_str());
        }
};

#endif // LOGGER_HPP