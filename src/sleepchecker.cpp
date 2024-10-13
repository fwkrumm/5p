#include <5p/sleepchecker.hpp>

using namespace sleepchecker;

SleepChecker::SleepChecker(const common::config& cfg)
    : lastDataPacketTimestamp_(0U), sleepTime_(cfg.sleep) {
}

SleepChecker::~SleepChecker() { 
}

void SleepChecker::CheckSleep(const uint16_t timestamp) {

    // prevent initial sleep
    if (lastDataPacketTimestamp_ == 0U) {
        lastDataPacketTimestamp_ = timestamp;
        return;
    }

    if (sleepTime_ == -1) {
        // time sleep according to data packet (add check that no backward jumps?)
        auto diffBetweenSamples = timestamp - lastDataPacketTimestamp_;
        LOG_DEBUG << "according to data packet diff, sleeping "
                  << diffBetweenSamples << " ms.";

        std::this_thread::sleep_for(
            std::chrono::milliseconds(diffBetweenSamples));

    }
    else if (sleepTime_ > 0) {
        // apply manual sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime_));
    } // else as fast as possible

    // update timestamp
    lastDataPacketTimestamp_ = timestamp;
}
