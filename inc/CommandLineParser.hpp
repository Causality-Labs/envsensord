#ifndef COMMANDLINEPARSER_HPP
#define COMMANDLINEPARSER_HPP

#include <string>

struct EnvServerConfig {
    int port;
    int numThreads;
    int sensorInterval;
    std::string deviceName;

    bool showHelp;
    bool showVersion;
};

class CommandLineParser
{
public:
    CommandLineParser();
    int parse(int argc, char* argv[], EnvServerConfig& config);
    void printUsage(const char* progName);
    void printVersion();

    private:
    void printError(const std::string& error);
};

#endif