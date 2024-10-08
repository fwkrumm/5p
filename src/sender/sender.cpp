#include <5p/sender.hpp>

using namespace sender;

DataSender::DataSender(ppppp::ProtocolType protocol,
                       const std::string& ip,
                       const uint16_t port)
            : protocol_(protocol),
            ip_(ip),
            port_(port),
            io_context_(),
            udp_socket_(io_context_),
            tcp_socket_(io_context_) {
}

bool DataSender::Init() {
    try {
        if (protocol_ == ppppp::ProtocolType::UDP) {
            LOG_DEBUG << "trying to open udp socket";
            udp_endpoint_ = boost::asio::ip::udp::endpoint(
                boost::asio::ip::address::from_string(ip_), port_);
            udp_socket_.open(boost::asio::ip::udp::v4());
        } else if (protocol_ == ppppp::ProtocolType::TCP) {
            LOG_DEBUG << "trying to connect tcp socket";
            tcp_endpoint_ = boost::asio::ip::tcp::endpoint(
                boost::asio::ip::address::from_string(ip_), port_);
            tcp_socket_.connect(tcp_endpoint_);
        } else {
            LOG_WARNING
                << "no protocol selected. Data will not be forwarded.";
            // will still return true since it is not an error; this might
            // be intentional
        }
        return true;
    } catch (std::exception& e) {
        LOG_ERROR << "Error initializing DataSender: " << e.what();
    }
    return false;
}

int64_t DataSender::Send(uint8_t* data, uint16_t size) {
    if (udp_socket_.is_open()) {
        return static_cast<int64_t>(SendUdp_(data, size));
    } else if (tcp_socket_.is_open()) {
        return static_cast<int64_t>(SendTcp_(data, size));
    }
    return -1;    // no suitable socket open
}

DataSender::~DataSender() {
    // close sockets
    if (udp_socket_.is_open()) {
        udp_socket_.close();
    }
    if (tcp_socket_.is_open()) {
        tcp_socket_.close();
    }
}

size_t DataSender::SendUdp_(uint8_t* data, uint16_t size) {
    auto rc =
        udp_socket_.send_to(boost::asio::buffer(data, size), udp_endpoint_);
    LOG_DEBUG << "UDP data sent successfully, rc : " << rc;
    return rc;
}

size_t DataSender::SendTcp_(uint8_t* data, uint16_t size) {
    auto rc =
        boost::asio::write(tcp_socket_, boost::asio::buffer(data, size));
    LOG_DEBUG << "TCP data sent successfully, rc: " << rc;
    return rc;
}