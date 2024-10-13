#include <5p/packethandler.hpp>

using namespace packethandler;

PacketHandler::PacketHandler(const common::config& cfg) 
    : static_port_(cfg.port), static_protocol_type_(cfg.protocol) {

    if (static_port_ != common::INIT_PORT) {
        LOG_INFO << "port persistently set to " << static_port_;
    }

    if (static_protocol_type_ != common::INIT_PROTOCOL_TYPE) {
        LOG_INFO << "protocol type persistently set to "
                 << static_cast<int32_t>(static_protocol_type_);
    }

}

PacketHandler::~PacketHandler() { 
    CleanMap(); 
}

bool PacketHandler::AddSender(const common::ProtocolType& protocol,
                              const std::string& ip,
                              const uint16_t port) {

    // check if dynamic parameters from packet are supposed to be used or
    // statically set
    auto check = CheckPortAndProtocol(protocol, port);

    bool does_sender_already_exist =
        senders_[check.first][check.second] != nullptr;

    if (does_sender_already_exist) {
        // sender already there
        return true;
    }


    // note that we could do some checks here whether the ip is valid etc
    // but we let the sockets decide whether or not the parameters are valid
    // the Init() function will return false if anything fails

    senders_[check.first][check.second] =
        new sender::DataSender(check.first, ip, check.second);

    auto rc = senders_[check.first][check.second]->Init();
    if (!rc) {
        senders_[check.first][check.second]->Shutdown();
        delete senders_[check.first][check.second];
        senders_[check.first][check.second] = nullptr;
    }

    LOG_DEBUG << "Sender added for protocol " 
        << static_cast<int32_t>(check.first) << " and " 
        << ip << ":" << check.second << " returned: " << rc;
    return rc;
}


int64_t PacketHandler::Send(const common::ProtocolType& protocol,
                            const uint16_t port,
                            const uint8_t* data, const uint16_t size) {

    auto check = CheckPortAndProtocol(protocol, port);

    // check if sender has been initialized
    auto& sender = senders_[check.first][check.second];

    if (!sender) {
        return 0;
    }

    // return send result
    return sender->Send(data, size);
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
