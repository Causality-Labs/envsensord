#include "CommandLineParser.hpp"
#include <iostream>
#include <getopt.h>

CommandLineParser::CommandLineParser() {}

int CommandLineParser::parse(int argc, char* argv[], EnvServerConfig& config)
{
    // Set default values
    config.port = 3500;
    config.numThreads = 4;
    config.sensorInterval = 1000;
    config.deviceName = "";
    config.showHelp = false;
    config.showVersion = false;

    // Define long options
    static struct option long_options[] = {
        {"port",     required_argument, 0, 'p'},
        {"threads",  required_argument, 0, 't'},
        {"interval", required_argument, 0, 'i'},
        {"device",   required_argument, 0, 'd'},
        {"help",     no_argument,       0, 'h'},
        {"version",  no_argument,       0, 'v'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "p:t:i:d:hv", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'p':
                config.port = std::stoi(optarg);
                if (config.port <= 0 || config.port > 65535) {
                    printError("Invalid port number. Must be between 1 and 65535.");
                    return -1;
                }
                break;

            case 't':
                config.numThreads = std::stoi(optarg);
                if (config.numThreads <= 0) {
                    printError("Number of threads must be positive.");
                    return -1;
                }
                break;

            case 'i':
                config.sensorInterval = std::stoi(optarg);
                if (config.sensorInterval <= 0) {
                    printError("Sensor interval must be positive.");
                    return -1;
                }
                break;

            case 'd':
                config.deviceName = optarg;
                break;

            case 'h':
                config.showHelp = true;
                return 0;

            case 'v':
                config.showVersion = true;
                return 0;

            case '?':
                // getopt_long already printed an error message
                return -1;

            default:
                return -1;
        }
    }

    return 0;
}

void CommandLineParser::printUsage(const char* progName)
{
    std::cout << "Usage: " << progName << " [OPTIONS]\n\n";
    std::cout << "Environmental Sensor Server - Exposes BME280 sensor data via TCP/IP\n\n";
    std::cout << "Options:\n";
    std::cout << "  -p, --port PORT          Server port (default: 3500)\n";
    std::cout << "  -t, --threads NUM        Number of worker threads (default: 4)\n";
    std::cout << "  -i, --interval MS        Sensor update interval in milliseconds (default: 1000)\n";
    std::cout << "  -d, --device NAME        Device name for logging (optional)\n";
    std::cout << "  -h, --help               Show this help message\n";
    std::cout << "  -v, --version            Show version information\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << progName << " --port 8080 --threads 8\n";
    std::cout << "  " << progName << " -p 3500 -i 500\n";
}

void CommandLineParser::printVersion()
{
    std::cout << "Environmental Sensor Server v1.0.0\n";
    std::cout << "Built for BeagleBone Black with BME280 sensor\n";
}

void CommandLineParser::printError(const std::string& error)
{
    std::cerr << "Error: " << error << "\n";
    std::cerr << "Use --help for usage information.\n";
}
