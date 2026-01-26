#ifndef COMMANDLINEPARSER_HPP
#define COMMANDLINEPARSER_HPP

#include <string>
#include "SSNP.hpp"

struct EnvServerConfig {
    int port;
    int numThreads;
    int sensorInterval;
    std::string deviceName;

    bool showHelp;
    bool showVersion;
};

struct EnvClientConfig {
    std::string hostname;
    int port;
    ssnp::SsnpRequestType request;

    bool showHelp;
    bool showVersion;
};

class ServerCommandLineParser
{
public:
    ServerCommandLineParser();
    int parse(int argc, char* argv[], EnvServerConfig& config);
    void printUsage(const char* progName);
    void printVersion();

private:
    void printError(const std::string& error);
};

class ClientCommandLineParser
{
public:
    ClientCommandLineParser();
    int parse(int argc, char* argv[], EnvClientConfig& config);
    void printUsage(const char* progName);
    void printVersion();

private:
    void printError(const std::string& error);
};

#endif // COMMANDLINEPARSER_HPP