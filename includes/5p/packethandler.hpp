#pragma once

#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING

#include <iostream>
#include <string>
#include <unordered_map>

#include "5p/common.hpp"
#include "5p/sender.hpp"
namespace packethandler {

class PacketHandler {
    public:

    PacketHandler(const common::config& cfg);
    ~PacketHandler();

    /*
     * clean complete senders map
     */
    void CleanMap();

    bool AddSender(const common::ProtocolType& protocol, const std::string& ip, const uint16_t port);
    int64_t Send(const common::ProtocolType& protocol, const uint16_t port,
                 const uint8_t* data, const uint16_t size);
    
    private:

    /*
     * for each protocol save for each member a DataSender
    */
    std::unordered_map<common::ProtocolType, std::unordered_map<uint16_t, sender::DataSender*>> senders_;

    // any value specified from config 
    // will be statically set for all senders
    uint16_t static_port_;
    common::ProtocolType static_protocol_type_;
	

};

}    // namespace handler