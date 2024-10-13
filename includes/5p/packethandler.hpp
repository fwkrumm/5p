#pragma once

#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING

#include <iostream>
#include <string>
#include <unordered_map>

#include "5p/common.hpp"
#include "5p/sender.hpp"
namespace packethandler {

/*
 * PacketHandler class to handle packets and forward them
 * to the desired destination via boost sockets
 */
class PacketHandler {
    public:

    explicit PacketHandler(const common::config& cfg);
    ~PacketHandler();

    /*
     * clean complete senders map
     */
    void CleanMap();

    /*
     * Add a sender to the senders map
     * @param protocol: the protocol to use
     * @param ip: the ip to send to
     * @param port: the port to send to
     * @return true if the sender was added successfully or
     *       if it already exists. Either way, if this function returns true,
     *       the sender is ready to send data via Send(...) function
     *       false if the sender could not be added
     */
    bool AddSender(const common::ProtocolType& protocol, const std::string& ip, const uint16_t port);

    /*
     * Send data to the desired destination
     * @param protocol: the protocol to use
     * @param port: the port to send to
     * @param data: the data to send
     * @param size: the size of the data
     * @return the number of bytes sent. Returns -1 if the sender
     *      could not be found (socket not open) or if the sender
     *      was not initialized
     */
    int64_t Send(const common::ProtocolType& protocol, const uint16_t port,
                 const uint8_t* data, const uint16_t size);

    private:

    /*
     * Check if the member variables of the port and protocol
     * are set to default values if not, return the values
     * set in the config i.e. the user
     * specified values manually
     * @param protocol: the protocol to test
     * @param port: the port to test
     * @return the protocol and port to use
     *     for the sender
     *    first: the protocol to use
     *   second: the port to use
     */
    inline std::pair <common::ProtocolType, uint16_t> CheckPortAndProtocol(
                            const common::ProtocolType& protocol, const uint16_t port) {
        return {(static_protocol_type_ == common::INIT_PROTOCOL_TYPE)
                    ? protocol
                    : static_protocol_type_,
                (static_port_ == common::INIT_PORT) ? port : static_port_};
    }

    // for each protocol and port we have a DataSender
    std::unordered_map<common::ProtocolType, std::unordered_map<uint16_t, sender::DataSender*>> senders_;

    // any value specified from config
    // will be statically set for all senders
    uint16_t static_port_;
    common::ProtocolType static_protocol_type_;


};

}    // namespace handler