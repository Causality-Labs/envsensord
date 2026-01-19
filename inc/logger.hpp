#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <syslog.h>
#include <string>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <mutex>


// Thread safe.
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

class StdLogger {
    private:
        std::string logger_name;
        std::mutex logMutex;

    public:
        StdLogger(const std::string& name) : logger_name(name) {
        }
        
        ~StdLogger() {
        }
        
        void info(const std::string& message) {
            std::lock_guard<std::mutex> lock(logMutex); 
            std::cout << "[" << logger_name << "] [INFO] " << message << std::endl;
        }
        
        void error(const std::string& message) {
            std::lock_guard<std::mutex> lock(logMutex); 
            std::cerr << "[" << logger_name << "] [ERROR] " << message << std::endl;
        }
};

#endif // LOGGER_HPP