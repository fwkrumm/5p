#include <5p/packethandler.hpp>

using namespace packethandler;

PacketHandler::PacketHandler() : locked_(false) {
}

PacketHandler::~PacketHandler() { 
    CleanMap(); 
}

bool PacketHandler::AddSender(const common::ProtocolType& protocol,
                              const std::string& ip,
                              const uint16_t port) {

    bool does_sender_already_exist = senders_[protocol][port] != nullptr;
    bool is_protocol_valid = protocol == common::ProtocolType::UDP ||
                             protocol == common::ProtocolType::TCP;
    bool is_port_valid = port > 0U;
    // check if map is locked or if sender already added or if protocol not supported or port is invalid
    if (locked_ || does_sender_already_exist || !is_protocol_valid ||
        !is_port_valid) {
        // here one could add some point add port exclusions
        return false;
    }

    senders_[protocol][port] = new sender::DataSender(protocol, ip, port);

    auto rc = senders_[protocol][port]->Init();
    if (!rc) {
        senders_[protocol][port]->Shutdown();
        delete senders_[protocol][port];
        senders_[protocol][port] = nullptr;
    }
    return rc;
}

const bool PacketHandler::DoesSenderExist(const common::ProtocolType& protocol,
                                          const uint16_t port) {
    return (senders_[protocol][port] != nullptr);
}

int64_t PacketHandler::Send(const common::ProtocolType& protocol,
                            const uint16_t port,
                            const uint8_t* data, const uint16_t size) {

    // check if sender has been initialized
    auto& sender = senders_[protocol][port];

    if (!sender) {
        return 0;
    }

    // return send result
    return sender->Send(data, size);
}

void PacketHandler::SetLockTo(const bool lock) {
    locked_ = lock;
}

void PacketHandler::CleanMap() {
    try {
        for (auto& protocolPair : senders_) {
            for (auto& senderPair : protocolPair.second) {
                if (!senderPair.second) {
                    continue;
                }
                senderPair.second->Shutdown();
                delete senderPair.second;    // delete the data pointer
            }
            protocolPair.second.clear();    // Clear the inner unordered_map
        }
        senders_.clear();    // Clear the outer unordered_map
    } catch (std::exception& e) {
        LOG_ERROR << "Error during clean map: " << e.what();
    }
}
