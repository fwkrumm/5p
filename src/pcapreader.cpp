#include <5p/pcapreader.hpp>

using namespace pcapreader;

Reader::Reader() : reader_(nullptr) {}

bool Reader::SetPcapFile(const std::string& path) {
    // check if file exists
    if (!boost::filesystem::exists(path)) {
        LOG_ERROR << "file " << path << " does not exist";
        return false;
    }

    // check if already open
    if (reader_ && reader_->open()) {
        LOG_WARNING << "pcap reader was already open, "
                    << "closing it to open new file";
        reader_->close();
    }

    // create reader instance
    reader_ = pcpp::IFileReaderDevice::getReader(path);

    // check if opened successfully
    if (!reader_->open()) {
        LOG_ERROR << "Could not open file " << path;
        return false;
    }

    return true;
}

bool Reader::ApplyFilter(const std::string& filter) {
    if (!reader_->setFilter(filter)) {
        LOG_ERROR << "Error setting the filter " << filter;
        return false;
    }

    LOG_DEBUG << "applied filter: " << filter;
    return true;
}

bool Reader::NextPackage(pcpp::Packet& packet) {
    pcpp::RawPacket raw;

    if (!reader_->getNextPacket(raw)) {
        return false;
    }

    packet = pcpp::Packet(&raw);

    return true;
}


bool Reader::checkFragmentation(pcpp::Packet& packet) {
    pcpp::IPv4Layer* ipv4Layer = packet.getLayerOfType<pcpp::IPv4Layer>();

    if (ipv4Layer != nullptr) {

        uint16_t fragmentOffset = ipv4Layer->getFragmentOffset();
        bool moreFragments = ipv4Layer->isFragment();

        if (fragmentOffset != 0 || moreFragments) {
            LOG_INFO << "IPv4 packet requires reassembly: ID = "
                     << ipv4Layer->getIPv4Header()->ipId
                     << ", Offset = " << fragmentOffset
                     << ", MoreFragments = " << moreFragments;
            return true;
        } 
    } else {

        pcpp::IPv6Layer* ipv6Layer = packet.getLayerOfType<pcpp::IPv6Layer>();
        if (ipv6Layer != nullptr) {
            // Check for IPv6 fragmentation header
            pcpp::IPv6FragmentationHeader* fragHeader =
                ipv6Layer->getExtensionOfType<pcpp::IPv6FragmentationHeader>();
            if (fragHeader != nullptr) {
                LOG_INFO << "IPv6 packet requires reassembly: Fragment offset = "
                         << fragHeader->getFragmentOffset()
                         << ", isMoreFragments = " << fragHeader->isMoreFragments();
                return true;
            }
        } else {
            LOG_INFO << "packet does not have IPv4 or IPv6 layers. Assuming not fragmented";
        }
    }

    // no fragmented fragments
    return false;
}

std::string toHexString(const uint8_t* data, size_t length) {
    std::ostringstream oss;
    for (size_t i = 0; i < length; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(data[i]) << " ";
    }
    return oss.str();
}

common::DataPacket Reader::ToDataPacket(pcpp::Packet& packet) {

   common::DataPacket dataPacket;

    // timestamp of package in ms
    dataPacket.timestamp =
        packet.getRawPacket()->getPacketTimeStamp().tv_sec * 1e3 +
        static_cast<uint64_t>(packet.getRawPacket()->getPacketTimeStamp().tv_nsec / 1e6);

    if (checkFragmentation(packet)) {
        pcpp::IPReassembly::ReassemblyStatus status;
        pcpp::Packet* reassembledPacket =
            ip_reassembly_.processPacket(&packet, status);

        if (status == pcpp::IPReassembly::REASSEMBLED) {
            LOG_INFO << "Packet reassembled successfully.";
            pcpp::PayloadLayer* testLayer =
                reassembledPacket->getLayerOfType<pcpp::PayloadLayer>();
            if (testLayer != nullptr) {
                LOG_INFO << "Reassembled packet of size "
                         << testLayer->getPayloadLen();
                std::string payloadString(
                    reinterpret_cast<const char*>(testLayer->getPayload()),
                    testLayer->getPayloadLen());
                LOG_INFO << "Payload (string): " << payloadString;
                LOG_INFO << "Payload (hex): "
                         << toHexString(testLayer->getPayload(),
                                        testLayer->getPayloadLen());
            } else {
                LOG_INFO << "No payload layer found in reassembled packet.";
            }
        } else {
            LOG_INFO << "Packet reassembly status: " << status;
            if (status == pcpp::IPReassembly::FRAGMENT ||
                status == pcpp::IPReassembly::FIRST_FRAGMENT) {
                pcpp::IPv4Layer* ipv4Layer =
                    packet.getLayerOfType<pcpp::IPv4Layer>();
                if (ipv4Layer != nullptr) {
                    LOG_INFO << "Fragment offset: "
                             << ipv4Layer->getFragmentOffset();
                    pcpp::PayloadLayer* fragmentLayer =
                        packet.getLayerOfType<pcpp::PayloadLayer>();
                    if (fragmentLayer != nullptr) {
                        LOG_INFO << "Fragment payload (hex): "
                                 << toHexString(fragmentLayer->getPayload(),
                                                fragmentLayer->getPayloadLen());
                    }
                }
            }
        }

        #if (0)
        if (reassembledPacket != nullptr) {
            // Process the reassembled packet
            // For example, write it to a new pcap file
            pcpp::PayloadLayer* testLayer =
                reassembledPacket->getLayerOfType<pcpp::PayloadLayer>();
            LOG_INFO << "reassembled packet of size "
                     << testLayer->getPayloadLen() << " with status " << status;
            std::string payloadString(
                reinterpret_cast<const char*>(testLayer->getPayload()),
                testLayer->getPayloadLen());
            LOG_INFO << payloadString;
            LOG_INFO << payloadString;
            LOG_INFO << payloadString;
            packet = *reassembledPacket;
        } else {
            LOG_WARNING << "reassembledPacket failed, Packet will probably contain nonsense! status = " << status;
        }
#endif
    }

    // extract payload
    pcpp::PayloadLayer* payloadLayer =
        packet.getLayerOfType<pcpp::PayloadLayer>();
    if (payloadLayer != nullptr) {
        dataPacket.payload = payloadLayer->getPayload();
        dataPacket.payloadLength =
            static_cast<uint16_t>(payloadLayer->getPayloadLen());
        
        LOG_DEBUG << "extracted payload of size " << dataPacket.payloadLength;
#if (0)
         std::string payloadString(
            reinterpret_cast<const char*>(payloadLayer->getPayload()),
            payloadLayer->getPayloadLen());
        LOG_INFO << payloadString;
        LOG_INFO << payloadString;
        LOG_INFO << payloadString;
#endif

    } else {
        LOG_WARNING << "no payload layer found.";
    }

    // extract transport layer
    pcpp::TcpLayer* tcpLayer = packet.getLayerOfType<pcpp::TcpLayer>();
    pcpp::UdpLayer* udpLayer = packet.getLayerOfType<pcpp::UdpLayer>();

    if (tcpLayer != nullptr) {

        LOG_DEBUG << "protocol: tcp; source port: " << tcpLayer->getSrcPort()
                  << "; destination port : " << tcpLayer->getDstPort();

        dataPacket.port = tcpLayer->getDstPort();
        dataPacket.protocol = common::ProtocolType::TCP;
    }
    else if (udpLayer != nullptr) {

        LOG_DEBUG << "protocol: udp; source port : " << udpLayer->getSrcPort()
                  << "; destination port : " << udpLayer->getDstPort();

        dataPacket.port = udpLayer->getDstPort();
        dataPacket.protocol = common::ProtocolType::UDP;
    } else {
        pcpp::Layer* layer = packet.getFirstLayer();

        while (layer != nullptr)
        {
            pcpp::ProtocolType protocolType = layer->getProtocol();

            LOG_DEBUG << "Unknown layer: " << layer->toString() 
                      << " cannot extract port, if none has specified "
                         "this data will not be forwarded.";

            layer = layer->getNextLayer();
        }
    }
    // else unknown protocol, target port remains 0U

    return dataPacket;
}

Reader::~Reader() {
    // check if initialized
    if (!reader_) {
        return;
    }

    // close if open
    if (reader_->open()) {
        reader_->close();
    }

    // delete and set to nullptr
    delete reader_;
    reader_ = nullptr;
}
