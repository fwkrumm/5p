#pragma once

#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING

#include <pcapplusplus/IPv4Layer.h>
#include <pcapplusplus/IPv6Layer.h>
#include <pcapplusplus/Packet.h>
#include <pcapplusplus/PayloadLayer.h>
#include <pcapplusplus/PcapFileDevice.h>
#include <pcapplusplus/PcapFilter.h>
#include <pcapplusplus/ProtocolType.h>
#include <pcapplusplus/RawPacket.h>
#include <pcapplusplus/TcpLayer.h>
#include <pcapplusplus/UdpLayer.h>

#include <boost/filesystem.hpp>
#include <iostream>
#include <unordered_map>

#include "5p/logging.hpp"

namespace ppppp {

// protocol types for data send/forward
enum class ProtocolType { INIT_UNSPECIFIED = 0, TCP = 1, UDP = 2 };

// reduce data to protocal type and payload
struct DataPacket {
    uint16_t payloadLength = 0U;
    uint8_t* payload = nullptr;
    uint64_t timestamp = 0U;
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

class Reader {
   private:
    pcpp::IFileReaderDevice* reader_;

   public:
    Reader();
    ~Reader();

    /*
     * Set the pcap file to read
     * @param path: path to the pcap file
     * @return true if the file was opened successfully
     *       false if the file could not be opened
     */
    bool SetPcapFile(const std::string& path);

    /*
     * Apply a filter to the pcap file
     * @param filter: the filter to apply
     * @return true if the filter was applied successfully
     *       false if the filter could not be applied
     */
    bool ApplyFilter(const std::string& filter);

    /*
     * Get the next packet from the pcap file
     * @param packet: the packet to be filled
     * @return true if a packet was read successfully
     *        false if no more packets are available
     */
    bool NextPackage(pcpp::Packet& packet);

    /*
     * Get DataPacket from pcpp_packet
     * @param packet: the pcpp packet
     * @return DataPacket which contains protocol
     *       type and payload (+ its size)
     */
    DataPacket ToDataPacket(const pcpp::Packet& packet);
};

}    // namespace ppppp