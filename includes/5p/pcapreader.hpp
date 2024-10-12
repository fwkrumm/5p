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

#include "5p/common.hpp"
#include "5p/logging.hpp"

namespace pcapreader {


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
    common::DataPacket ToDataPacket(const pcpp::Packet& packet);
};

}    // namespace ppppp