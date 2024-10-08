#include <5p/pcapreader.hpp>

using namespace ppppp;

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

DataPacket Reader::ToDataPacket(const pcpp::Packet& packet) {
    // timestamp not used yet, could/will be used to determine real-time replay
    // speed
    /*
    uint64_t timestamp_ns =
        packet.getRawPacket()->getPacketTimeStamp().tv_sec * 1e9 +
        packet.getRawPacket()->getPacketTimeStamp().tv_nsec;    // ns
    */

    ppppp::DataPacket dataPacket;

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
        // dataPacket.payloadLength = 0U; // already initialized to 0U
    }

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
