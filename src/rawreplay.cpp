#include "5p/rawreplay.hpp"

#include <pcapplusplus/PcapLiveDeviceList.h>

#include <chrono>
#include <thread>

namespace rawreplay {
namespace {

void WaitForPacket(const pcpp::RawPacket& packet, const timespec& firstTimestamp,
                   const std::chrono::steady_clock::time_point& start,
                   int32_t sleepMilliseconds, bool firstPacket) {
    if (firstPacket) {
        return;
    }
    if (sleepMilliseconds >= 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepMilliseconds));
        return;
    }
    if (sleepMilliseconds < -1) {
        return;
    }

    const timespec timestamp = packet.getPacketTimeStamp();
    const auto offset = std::chrono::seconds(timestamp.tv_sec - firstTimestamp.tv_sec) +
                        std::chrono::nanoseconds(timestamp.tv_nsec - firstTimestamp.tv_nsec);
    std::this_thread::sleep_until(start + offset);
}

}    // namespace

bool IsReplayableFrame(const pcpp::RawPacket& packet, pcpp::LinkLayerType deviceLinkType) {
    return packet.getLinkLayerType() == pcpp::LINKTYPE_ETHERNET &&
           deviceLinkType == pcpp::LINKTYPE_ETHERNET &&
           packet.getRawDataLen() > 0 &&
           packet.getFrameLength() <= packet.getRawDataLen();
}

bool Replay(pcapreader::Reader& reader, const common::config& config) {
    pcpp::PcapLiveDevice* device =
        pcpp::PcapLiveDeviceList::getInstance().getPcapLiveDeviceByName(config.interfaceName);
    if (device == nullptr || !device->open()) {
        LOG_ERROR << "cannot open raw replay interface " << config.interfaceName;
        return false;
    }

    pcpp::RawPacket packet;
    uint64_t count = 0;
    bool firstPacket = true;
    timespec firstTimestamp{};
    std::chrono::steady_clock::time_point start;
    bool success = true;

    while (reader.NextRawPacket(packet)) {
        if (++count < config.skip) {
            continue;
        }
        if (firstPacket) {
            firstTimestamp = packet.getPacketTimeStamp();
            start = std::chrono::steady_clock::now();
        }
        if (!IsReplayableFrame(packet, device->getLinkType())) {
            LOG_ERROR << "raw replay requires complete Ethernet frames on an Ethernet interface";
            success = false;
            break;
        }
        WaitForPacket(packet, firstTimestamp, start, config.sleep, firstPacket);
        if (!device->sendPacket(packet)) {
            LOG_ERROR << "failed to inject raw packet";
            success = false;
            break;
        }
        firstPacket = false;
    }

    device->close();
    return success;
}

}    // namespace rawreplay