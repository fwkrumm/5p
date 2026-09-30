#include <gtest/gtest.h>

#include "5p/cli.hpp"
#include "5p/rawreplay.hpp"

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <string>
#include <vector>

namespace {

int Parse(std::initializer_list<const char*> arguments, common::config& config) {
    std::vector<char*> argv;
    std::transform(arguments.begin(), arguments.end(), std::back_inserter(argv),
                   [](const char* argument) { return const_cast<char*>(argument); });
    return cli::GetParameters(static_cast<int>(argv.size()), argv.data(), config);
}

}    // namespace

TEST(ReplayMode, DefaultKeepsSocketMode) {
    common::config config;
    EXPECT_EQ(Parse({"5p", "trace.pcapng"}, config), 0);
    EXPECT_EQ(config.mode, "socket");
    EXPECT_TRUE(config.interfaceName.empty());
}

TEST(ReplayMode, RawRequiresInterface) {
    common::config config;
    EXPECT_NE(Parse({"5p", "trace.pcapng", "--mode", "raw"}, config), 0);
}

TEST(ReplayMode, RawAcceptsInterface) {
    common::config config;
    EXPECT_EQ(Parse({"5p", "trace.pcapng", "--mode", "raw", "--interface", "eth0"}, config), 0);
    EXPECT_EQ(config.interfaceName, "eth0");
}

TEST(ReplayMode, RawRejectsSocketOverrides) {
    for (const char* option : {"--ip", "--port", "--protocol"}) {
        common::config config;
        const char* value = option == std::string("--ip") ? "127.0.0.1" : "1";
        EXPECT_NE(Parse({"5p", "trace.pcapng", "--mode", "raw", "--interface", "eth0", option, value}, config), 0)
            << option;
    }
}

TEST(ReplayMode, SocketRejectsInterface) {
    common::config config;
    EXPECT_NE(Parse({"5p", "trace.pcapng", "--interface", "eth0"}, config), 0);
}

TEST(ReplayMode, RejectsUnknownMode) {
    common::config config;
    EXPECT_NE(Parse({"5p", "trace.pcapng", "--mode", "unknown"}, config), 0);
}

TEST(RawReplay, PreservesCompleteEthernetFrame) {
    const uint8_t frame[64] = {};
    pcpp::RawPacket packet(frame, sizeof(frame), timespec{}, false);
    EXPECT_TRUE(rawreplay::IsReplayableFrame(packet, pcpp::LINKTYPE_ETHERNET));
    EXPECT_EQ(packet.getRawDataLen(), static_cast<int>(sizeof(frame)));
    EXPECT_EQ(packet.getRawData()[0], frame[0]);
}

TEST(RawReplay, RejectsNonEthernetFrameAndInterface) {
    const uint8_t frame[64] = {};
    pcpp::RawPacket ethernet(frame, sizeof(frame), timespec{}, false);
    pcpp::RawPacket other(frame, sizeof(frame), timespec{}, false, pcpp::LINKTYPE_RAW);
    EXPECT_FALSE(rawreplay::IsReplayableFrame(ethernet, pcpp::LINKTYPE_RAW));
    EXPECT_FALSE(rawreplay::IsReplayableFrame(other, pcpp::LINKTYPE_ETHERNET));
}

TEST(RawReplay, RejectsTruncatedFrame) {
    const uint8_t frame[64] = {};
    pcpp::RawPacket packet(frame, sizeof(frame), timespec{}, false);
    ASSERT_TRUE(packet.setRawData(frame, sizeof(frame), timespec{},
                                  pcpp::LINKTYPE_ETHERNET, sizeof(frame) + 1));
    EXPECT_FALSE(rawreplay::IsReplayableFrame(packet, pcpp::LINKTYPE_ETHERNET));
}