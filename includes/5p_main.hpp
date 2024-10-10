#pragma once

#include <thread>

#include "5p/cli.hpp"
#include "5p/logging.hpp"
#include "5p/pcapreader.hpp"
#include "5p/sender.hpp"

namespace returns {

// Return codes for the program
enum class ReturnCodes {
    HELP_PRINTED = -1,
    SUCCESS = 0,
    MISSING_PARAMETERS = 1,
    PCAP_FILE_NOT_FOUND = 2,
    INVALID_FILTER = 3,
    SOCKET_FAILURE = 4
};

}    // namespace returns