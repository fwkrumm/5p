#include <5p/sleepchecker.hpp>

using namespace sleepchecker;

SleepChecker::SleepChecker(const common::config& cfg)
    : lastDataPacketTimestamp_(0U), sleepTime_(cfg.sleep) {
}

SleepChecker::~SleepChecker() {
}



void SleepChecker::PreciseSleep(double seconds) {

    // source:
    // https://blat-blatnik.github.io/computerBear/making-accurate-sleep-function/

    LOG_DEBUG << "executing PreciseSleep with " << seconds << " seconds";

    if (seconds <= 0) return;

    static double estimate = 5e-3;
    static double mean = 5e-3;
    static double m2 = 0;
    static int64_t count = 1;

    while (seconds > estimate) {
        auto start = std::chrono::high_resolution_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        auto end = std::chrono::high_resolution_clock::now();

        double observed = (end - start).count() / 1e9;
        seconds -= observed;

        ++count;
        double delta = observed - mean;
        mean += delta / count;
        m2 += delta * (observed - mean);
        double stddev = sqrt(m2 / (count - 1));
        estimate = mean + stddev;
    }

    // spin lock
    auto start = std::chrono::high_resolution_clock::now();
    while ((std::chrono::high_resolution_clock::now() - start).count() / 1e9 <
           seconds) {
        // pass/wait
    }
}

void SleepChecker::CheckSleep(const uint64_t timestamp, const int64_t runtimeUs) {

    // prevent initial sleep
    if (lastDataPacketTimestamp_ == 0U) {
        lastDataPacketTimestamp_ = timestamp;
        return;
    }

    double runtimeMs = runtimeUs / 1e3;    // us -> ms
    LOG_DEBUG << "run time ms = " << runtimeMs;

    if (sleepTime_ == -1) {
        // time sleep according to data packet (add check that no backward jumps?)
        auto diffBetweenSamples = timestamp - lastDataPacketTimestamp_;
        LOG_DEBUG << "according to data packet diff, sleeping "
                  << diffBetweenSamples << " ms.";

        double sleepTimeS =
            (static_cast<double>(diffBetweenSamples) - runtimeMs) / 1e3; // ms -> s

        PreciseSleep(sleepTimeS);
    }
    else if (sleepTime_ > 0) {

        // apply manual sleep
        double sleepTimeS = (static_cast<double>(sleepTime_) - runtimeMs) / 1e3; // ms -> s

        PreciseSleep(sleepTimeS);
    } // else as fast as possible (no sleep at all)

    // update timestamp
    lastDataPacketTimestamp_ = timestamp;
}
