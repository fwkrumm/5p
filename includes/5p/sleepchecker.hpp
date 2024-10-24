#pragma once

#include <thread>

#include "5p/common.hpp"
#include "5p/logging.hpp"


namespace sleepchecker {


/*
 * SleepChecker class to check if sleep should be applied
 * and if so, does it
 */
class SleepChecker {
    public:

    explicit SleepChecker(const common::config& cfg);
    ~SleepChecker();

    /*
     * check if sleep should be applied and set
     * new data packet timestamp
     */
    void CheckSleep(const uint64_t timestamp);

    private:
    uint64_t lastDataPacketTimestamp_;
    int32_t sleepTime_;

};

} // namespace sleepchecker