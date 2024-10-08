#pragma once

#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING

#include <CLI/CLI.hpp>
#include <string>

#include "5p/logging.hpp"
#include "5p/pcapreader.hpp"

namespace cli {

// gathers all important data for the program from parameters
struct config {
    // path to pcap(ng) file
    std::string path;

    // ip address and port to send data to
    std::string ip;
    uint16_t port;
    // protocol to use for data sending/forwarding
    ppppp::ProtocolType protocol;

    // BPF filter for pcap replay
    std::string filter;

    // log level
    logging_5p::LogLevel level;

    // specify default values here
    config()
        : level(logging_5p::LogLevel::INFO_LEVEL),
          ip("127.0.0.1"),
          port(49999),
          protocol(ppppp::ProtocolType::UDP) {}
};

/*
 * Get parameters from command line; this function must be used by main.cpp
 * @param argc: number of arguments
 * @param argv: arguments
 * @param cfg: config object to store parameters in
 * @return 0 if successful, otherwise return value is determined
 *   by CLI11
 */
int GetParameters(const int argc, char** const argv, config& cfg);

/*
 * This function is a wrapper around CLI11_PARSE to catch if help was printed to
 * prevent program execution after help print. only for usage within GetParameters function
 * @param app: CLI11 app object
 * @param argc: number of arguments
 * @param argv: arguments
 * @return 0 if successful, otherwise return value is determined
 *  by CLI11
 */
static int WrapReturnValue(CLI::App& app, const int argc, char** const argv);

}    // namespace cli