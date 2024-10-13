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

common::DataPacket Reader::ToDataPacket(const pcpp::Packet& packet) {

   common::DataPacket dataPacket;

    // timestamp of package in ms
    dataPacket.timestamp =
        packet.getRawPacket()->getPacketTimeStamp().tv_sec * 1e3 +
        static_cast<uint64_t>(packet.getRawPacket()->getPacketTimeStamp().tv_nsec / 1e6);


    if (packet.isPacketOfType(pcpp::IPv4)) {
        pcpp::IPv4Layer* ipLayer = packet.getLayerOfType<pcpp::IPv4Layer>();
        if (ipLayer != nullptr) {
            pcpp::IPv4Address destIp = ipLayer->getDstIPv4Address();
            dataPacket.ip = destIp.toString();
            LOG_DEBUG << "Destination IPV4 Address: " << dataPacket.ip
                      << std::endl;
        }
    } else if (packet.isPacketOfType(pcpp::IPv6)) {
        pcpp::IPv6Layer* ipLayer = packet.getLayerOfType<pcpp::IPv6Layer>();
        if (ipLayer != nullptr) {
            pcpp::IPv6Address destIp = ipLayer->getDstIPv6Address();
            dataPacket.ip = destIp.toString();
            LOG_DEBUG << "Destination IPV6 Address: " << dataPacket.ip
                      << std::endl;
        }
    } else {
        LOG_WARNING << "no destination ip found.";
    }

    // extract payload
    pcpp::PayloadLayer* payloadLayer =
        packet.getLayerOfType<pcpp::PayloadLayer>();
    if (payloadLayer != nullptr) {
        dataPacket.payload = payloadLayer->getPayload();
        dataPacket.payloadLength =
            static_cast<uint16_t>(payloadLayer->getPayloadLen());
        
        LOG_DEBUG << "extracted payload of size " << dataPacket.payloadLength;
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
