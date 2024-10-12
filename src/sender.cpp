#include <5p/sender.hpp>

using namespace sender;

DataSender::DataSender(common::ProtocolType protocol,
                       const std::string& ip,
                       const uint16_t port)
            : protocol_(protocol),
            ip_(ip),
            port_(port),
            io_context_(),
            udp_socket_(io_context_),
            tcp_socket_(io_context_),
            initialized_ (false){}

bool DataSender::Init() {
    try {
        if (protocol_ == common::ProtocolType::UDP) {
            LOG_DEBUG << "trying to open udp socket on port " << port_;
            udp_endpoint_ = boost::asio::ip::udp::endpoint(
                boost::asio::ip::address::from_string(ip_), port_);
            udp_socket_.open(boost::asio::ip::udp::v4());
        } else if (protocol_ == common::ProtocolType::TCP) {
            LOG_DEBUG << "trying to connect tcp socket on port " << port_;
            tcp_endpoint_ = boost::asio::ip::tcp::endpoint(
                boost::asio::ip::address::from_string(ip_), port_);
            tcp_socket_.connect(tcp_endpoint_);
        } else {
            LOG_ERROR
                << "no valid protocol selected. Data will not be forwarded.";
            return false;
        }

        initialized_ = true;

        // return actual value of socket has been opened
        return IsInitialized();
    } catch (std::exception& e) {
        LOG_ERROR << "Error initializing DataSender: " << e.what();
    }
    return false;
}

const bool DataSender::IsInitialized() const {
    return initialized_ && (udp_socket_.is_open() || tcp_socket_.is_open());
}

int64_t DataSender::Send(const uint8_t* data, const uint16_t size) {
    if (initialized_ && udp_socket_.is_open()) {
        return static_cast<int64_t>(SendUdp_(data, size));
    } else if (initialized_ && tcp_socket_.is_open()) {
        return static_cast<int64_t>(SendTcp_(data, size));
    }
    return -1; // no suitable socket open
}

void DataSender::Shutdown() {
    // close sockets
    if (udp_socket_.is_open()) {
        udp_socket_.close();
    }
    if (tcp_socket_.is_open()) {
        tcp_socket_.close();
    }

    initialized_ = false;
}

DataSender::~DataSender() { 
    Shutdown();
}

size_t DataSender::SendUdp_(const uint8_t* data, const uint16_t size) {
    auto rc =
        udp_socket_.send_to(boost::asio::buffer(data, size), udp_endpoint_);
    LOG_DEBUG << "UDP data sent successfully, rc : " << rc;
    return rc;
}

size_t DataSender::SendTcp_(const uint8_t* data, const uint16_t size) {
    auto rc =
        boost::asio::write(tcp_socket_, boost::asio::buffer(data, size));
    LOG_DEBUG << "TCP data sent successfully, rc: " << rc;
    return rc;
}
