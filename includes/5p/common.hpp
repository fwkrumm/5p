#pragma once

#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING

#include <iostream>

namespace common {

// log levels
enum class LogLevel {
    TRACE_LEVEL = 0,
    DEBUG_LEVEL = 1,
    INFO_LEVEL = 2,
    WARNING_LEVEL = 3,
    ERROR_LEVEL = 4,
    EXCEPTOIN_LEVEL = 5,
    NONE_LEVEL = 6
};

// protocol types for data send/forward
enum class ProtocolType { PACKET_PROTOCOLS = 0, TCP = 1, UDP = 2 };

// reduce data to protocal type and payload
struct DataPacket {
    uint16_t payloadLength = 0U;
    uint8_t* payload = nullptr;
    uint64_t timestamp = 0U;
    uint16_t port = 0U;
    ProtocolType protocol = ProtocolType::PACKET_PROTOCOLS; // unspecified
};

// default values for config
const std::string INIT_IP = "127.0.0.1";
constexpr uint16_t INIT_PORT = 0U;
constexpr ProtocolType INIT_PROTOCOL_TYPE = ProtocolType::PACKET_PROTOCOLS;

// gathers all important data for the program from parameters
struct config {
    // path to pcap(ng) file
    std::string path;

    // ip address and port to send data to
    std::string ip;
    uint16_t port;
    // protocol to use for data sending/forwarding
    ProtocolType protocol;

    // BPF filter for pcap replay
    std::string filter;

    // log level
    LogLevel level;

    // skip samples
    uint32_t skip;

    // sleep time in ms;
    int32_t sleep;

    // specify default values here
    config()
        : level(LogLevel::INFO_LEVEL),
          ip(INIT_IP),
          port(INIT_PORT),
          protocol(ProtocolType::PACKET_PROTOCOLS),
          sleep(-1),
          skip(0U) {}
};


// LEGACY only used to identify protocol types when the target protocol
// was determined by packet protocol. At the moment since the protocol
// is determined by the user, this is not needed anymore.
/*
// map number type to human-readable protocol type
static std::unordered_map<pcpp::ProtocolType, std::string> protocolTypeToString
= { { pcpp::Ethernet, "Ethernet" }, { pcpp::IPv4, "IPv4" }, { pcpp::IPv6, "IPv6"
}, { pcpp::TCP, "TCP" }, { pcpp::UDP, "UDP" }, { pcpp::HTTP, "HTTP" }, {
pcpp::ARP, "ARP" }, { pcpp::VLAN, "VLAN" }, { pcpp::ICMP, "ICMP" }, {
pcpp::ICMPv6, "ICMPv6" }, { pcpp::DNS, "DNS" }, { pcpp::MPLS, "MPLS" }, {
pcpp::SLL, "SLL" }, { pcpp::PPPoE, "PPPoE" }, { pcpp::GRE, "GRE" }, { pcpp::SIP,
"SIP" }, { pcpp::SDP, "SDP" }, { pcpp::Radius, "Radius" }, { pcpp::DHCP, "DHCP"
}, { pcpp::BGP, "BGP" },
    // more available at pcapplusplus/ProtocolType.h
};
*/

} // namespace common
