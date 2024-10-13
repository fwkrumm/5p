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

    PacketHandler();
    ~PacketHandler();

    /*
     * clean complete senders map
     */
    void CleanMap();

    /*
     * set lock to true or false to forbid or allow sender adding
     */
    void SetLockTo(const bool lock);
    const bool IsLocked() const { return locked_; };

    bool AddSender(const common::ProtocolType& protocol, const std::string& ip, const uint16_t port);
    int64_t Send(const common::ProtocolType& protocol, const uint16_t port,
                 const uint8_t* data, const uint16_t size);
    const bool DoesSenderExist (const common::ProtocolType& protocol,
                         const uint16_t port);
    
    private:

    /*
     * for each protocol save for each member a DataSender
    */
    std::unordered_map<common::ProtocolType, std::unordered_map<uint16_t, sender::DataSender*>> senders_;

    // will be set to true to prevent adding of further senders
    bool locked_;
	

};

}    // namespace handler