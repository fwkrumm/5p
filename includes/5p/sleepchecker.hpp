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
     * sleep for given amount of seconds
     * source
     * https://blat-blatnik.github.io/computerBear/making-accurate-sleep-function/
     * more precise sleep function without any additional dependencies
     * @param seconds - time to sleep in seconds
     */
    void PreciseSleep(double seconds);

    /*
     * check if sleep should be applied and set
     * new data packet timestamp
     */
    void CheckSleep(const uint64_t timestamp, const int64_t runtime);

   private:
    // store last data packet timestamp
    uint64_t lastDataPacketTimestamp_;
    // total sleep time in milliseconds from config i.e. parameters
    int32_t sleepTime_;
};

}    // namespace sleepchecker