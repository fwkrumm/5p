#pragma once

#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING

#include <string>
#include <boost/asio.hpp>

#include "5p/common.hpp"
#include "5p/pcapreader.hpp"

namespace sender {


/*
 * wrapper class to unify tcp and udp socket logic 
 */
class DataSender {
    public:
    /*
     * Constructor
     * @param protocol: protocol type to use
     * @param ip: ip address to send data to
     * @param port: port to send data to
     * @return DataSender object
     */
    DataSender(common::ProtocolType protocol, const std::string& ip,
               const uint16_t port);

    /*
     * Initialize the sender
     * @return true if successful, false otherwise
     * @note: will return true if sockets were initialized successfully or if
     * not protocol was selected
     * @note: will return false if an error occurs
     */
    bool Init();

    /*
     * return if sender is initialized 
     */
    const bool IsInitialized() const;

    /*
     * shutdown open socket (tcp or udp)
     */
    void Shutdown();

    /*
     * Send data
     * @param data: data to send
     * @param size: size of data
     * @return number of bytes sent
     * @note: will return -1 if no socket is open
     */
    int64_t Send(const uint8_t* data, const uint16_t size);

    /*
     * Destructor
     * @note: will close sockets
     */
    ~DataSender();

    private:
    /*
     * Send data over UDP
     * @param data: data to send
     * @param size: size of data
     * @return number of bytes sent
     */
    size_t SendUdp_(const uint8_t* data, const uint16_t size);

    /*
     * Send data over TCP
     * @param data: data to send
     * @param size: size of data
     * @return number of bytes sent
     */
    size_t SendTcp_(const uint8_t* data, const uint16_t size);

    /*
     * small helper function to log an error message and automatically
     * include specified ip and port
     */
    inline void LogError_(const std::string& msg, const char* err);

    // ip and port
    std::string ip_;
    uint16_t port_;
    // tcp or udp protocol
    common::ProtocolType protocol_;

    bool initialized_;

    // boost asio objects
    boost::asio::io_context io_context_;
    boost::asio::ip::udp::socket udp_socket_;
    boost::asio::ip::tcp::socket tcp_socket_;
    boost::asio::ip::udp::endpoint udp_endpoint_;
    boost::asio::ip::tcp::endpoint tcp_endpoint_;
};

}    // namespace sender
