#pragma once

#include <thread>

#include "5p/common.hpp"
#include "5p/logging.hpp"


namespace sleepchecker {


/*
 * wrapper class to unify tcp and udp socket logic 
 */
class SleepChecker {
    public:
    
    SleepChecker(const common::config& cfg);
    ~SleepChecker();

    /*
     * check if sleep should be applied and set
     * new data packet timestamp
     */
    void CheckSleep(const uint16_t timestamp);

    private:
    uint64_t lastDataPacketTimestamp_;
    int32_t sleepTime_;

};

} // namespace sleepchecker