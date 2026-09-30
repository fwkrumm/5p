#include <gtest/gtest.h>

#include <boost/asio.hpp>
#include <pcapplusplus/Packet.h>
#include <pcapplusplus/PayloadLayer.h>
#include <pcapplusplus/PcapFileDevice.h>
#include <pcapplusplus/UdpLayer.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <future>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace {

std::string CaptureFile(const std::string& name) {
    const std::string path = CAPTURE_PATH;
    return path.substr(0, path.find_last_of('/')) + "/" + name;
}

int RunReplay(const std::vector<std::string>& arguments) {
    const pid_t child = fork();
    if (child == 0) {
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(REPLAY_EXECUTABLE));
        std::transform(arguments.begin(), arguments.end(), std::back_inserter(argv),
                       [](const std::string& argument) { return const_cast<char*>(argument.c_str()); });
        argv.push_back(nullptr);
        execv(REPLAY_EXECUTABLE, argv.data());
        _exit(127);
    }
    if (child < 0) {
        return -1;
    }

    int status = 0;
    if (waitpid(child, &status, 0) != child || !WIFEXITED(status)) {
        return -1;
    }
    return WEXITSTATUS(status);
}

std::vector<std::vector<uint8_t>> CapturedPayloads(const std::string& capture,
                                                   const std::string& filter) {
    std::unique_ptr<pcpp::IFileReaderDevice> reader(pcpp::IFileReaderDevice::getReader(capture));
    if (!reader || !reader->open()) {
        return {};
    }
    if (!reader->setFilter(filter)) {
        return {};
    }

    std::vector<std::vector<uint8_t>> payloads;
    pcpp::RawPacket raw;
    while (reader->getNextPacket(raw)) {
        pcpp::Packet packet(&raw);
        const auto* udp = packet.getLayerOfType<pcpp::UdpLayer>();
        if (udp != nullptr && udp->getLayerPayloadSize() > 0) {
            payloads.emplace_back(udp->getLayerPayload(),
                                  udp->getLayerPayload() + udp->getLayerPayloadSize());
            continue;
        }
        const auto* layer = packet.getLayerOfType<pcpp::PayloadLayer>();
        if (layer != nullptr && layer->getPayloadLen() > 0) {
            payloads.emplace_back(layer->getPayload(), layer->getPayload() + layer->getPayloadLen());
        }
    }
    return payloads;
}

std::vector<uint8_t> CapturedTcpPayload(const std::string& capture, const std::string& filter) {
    std::vector<uint8_t> payload;
    for (const auto& segment : CapturedPayloads(capture, filter)) {
        payload.insert(payload.end(), segment.begin(), segment.end());
    }
    return payload;
}

}  // namespace

TEST(SampleReplay, SendsChargenUdpDatagramsToLoopback) {
    const std::string capture = CaptureFile("chargen-udp.pcap");
    const auto expected = CapturedPayloads(capture, "udp");
    ASSERT_EQ(expected.size(), 2);

    boost::asio::io_context context;
    boost::asio::ip::udp::socket receiver(context, {boost::asio::ip::address_v4::loopback(), 0});
    receiver.non_blocking(true);
    ASSERT_EQ(RunReplay({capture, "--filter", "udp", "--ip", "127.0.0.1", "--port",
                         std::to_string(receiver.local_endpoint().port()), "--sleep", "0"}), 0);

    std::array<uint8_t, 65536> buffer{};
    boost::asio::ip::udp::endpoint sender;
    for (const auto& datagram : expected) {
        boost::system::error_code error;
        const size_t size = receiver.receive_from(boost::asio::buffer(buffer), sender, 0, error);
        ASSERT_FALSE(error) << error.message();
        ASSERT_EQ(size, datagram.size());
        ASSERT_TRUE(std::equal(datagram.begin(), datagram.end(), buffer.begin()));
    }
    std::cout << "verified: UDP loopback received " << expected.size()
              << " chargen datagrams matching the capture\n";
}

TEST(SampleReplay, SendsDnsQueriesToLoopback) {
    const std::string capture = CaptureFile("dns.cap");
    const auto expected = CapturedPayloads(capture, "udp and dst port 53");
    ASSERT_EQ(expected.size(), 19);

    boost::asio::io_context context;
    boost::asio::ip::udp::socket receiver(context, {boost::asio::ip::address_v4::loopback(), 0});
    receiver.non_blocking(true);
    ASSERT_EQ(RunReplay({capture, "--filter", "udp and dst port 53", "--ip", "127.0.0.1",
                         "--port", std::to_string(receiver.local_endpoint().port()), "--sleep", "0"}), 0);

    std::array<uint8_t, 65536> buffer{};
    boost::asio::ip::udp::endpoint sender;
    for (const auto& datagram : expected) {
        boost::system::error_code error;
        const size_t size = receiver.receive_from(boost::asio::buffer(buffer), sender, 0, error);
        ASSERT_FALSE(error) << error.message();
        ASSERT_EQ(size, datagram.size());
        ASSERT_TRUE(std::equal(datagram.begin(), datagram.end(), buffer.begin()));
    }
    std::cout << "verified: UDP loopback received " << expected.size()
              << " DNS queries in capture order\n";
}

TEST(SampleReplay, SendsSegmentedHttpPostToLoopback) {
    const std::string capture = CaptureFile("tcp-ethereal-file1.trace");
    const std::string filter = "tcp and dst port 80";
    const auto segments = CapturedPayloads(capture, filter);
    ASSERT_EQ(segments.size(), 130);
    const auto expected = CapturedTcpPayload(capture, filter);
    ASSERT_EQ(expected.size(), 152372);

    boost::asio::io_context context;
    boost::asio::ip::tcp::acceptor receiver(context, {boost::asio::ip::address_v4::loopback(), 0});
    auto received = std::async(std::launch::async, [&] {
        boost::asio::ip::tcp::socket connection(context);
        receiver.accept(connection);
        std::vector<uint8_t> bytes(expected.size());
        boost::asio::read(connection, boost::asio::buffer(bytes));
        return bytes;
    });

    const int status = RunReplay({capture, "--filter", filter, "--ip", "127.0.0.1", "--port",
                                  std::to_string(receiver.local_endpoint().port()), "--sleep", "0"});
    if (status != 0) {
        boost::asio::ip::tcp::socket wake(context);
        boost::system::error_code error;
        wake.connect(receiver.local_endpoint(), error);
    }
    ASSERT_EQ(status, 0);
    ASSERT_EQ(received.get(), expected);
    std::cout << "verified: TCP loopback received " << expected.size()
              << " bytes from " << segments.size() << " captured HTTP segments\n";
}

TEST(HartReplay, SendsCapturedUdpPayloadToLoopback) {
    boost::asio::io_context context;
    boost::asio::ip::udp::socket receiver(
        context, {boost::asio::ip::address_v4::loopback(), 0});
    receiver.non_blocking(true);

    ASSERT_EQ(RunReplay({CAPTURE_PATH, "--filter", "udp and dst port 5094",
                         "--ip", "127.0.0.1", "--port",
                         std::to_string(receiver.local_endpoint().port()),
                         "--sleep", "0"}), 0);

    std::array<uint8_t, 2048> buffer{};
    boost::asio::ip::udp::endpoint sender;
    boost::system::error_code error;
    const size_t received = receiver.receive_from(boost::asio::buffer(buffer), sender, 0, error);
    ASSERT_FALSE(error) << error.message();
    ASSERT_EQ(sender.address(), boost::asio::ip::address_v4::loopback());
    const std::array<uint8_t, 13> expected = {
        0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,
        0x0d, 0x01, 0x00, 0x00, 0x75, 0x30};
    ASSERT_EQ(received, expected.size());
    ASSERT_TRUE(std::equal(expected.begin(), expected.end(), buffer.begin()));
    std::cout << "verified: UDP loopback received the 13-byte HART-IP payload from the capture\n";
}

TEST(HartReplay, SendsCapturedTcpStreamToLoopback) {
    const auto expected = CapturedTcpPayload(CAPTURE_PATH, "tcp and dst port 5094");
    ASSERT_FALSE(expected.empty());

    boost::asio::io_context context;
    boost::asio::ip::tcp::acceptor receiver(
        context, {boost::asio::ip::address_v4::loopback(), 0});
    receiver.non_blocking(true);

    ASSERT_EQ(RunReplay({CAPTURE_PATH, "--filter", "tcp and dst port 5094",
                         "--ip", "127.0.0.1", "--port",
                         std::to_string(receiver.local_endpoint().port()),
                         "--sleep", "0"}), 0);

    boost::system::error_code error;
    boost::asio::ip::tcp::socket connection(context);
    receiver.accept(connection, error);
    ASSERT_FALSE(error) << error.message();

    std::vector<uint8_t> received(expected.size());
    const size_t size = boost::asio::read(connection, boost::asio::buffer(received), error);
    ASSERT_FALSE(error) << error.message();
    ASSERT_EQ(size, expected.size());
    ASSERT_EQ(received, expected);
    std::cout << "verified: TCP loopback received " << size
              << " bytes matching the captured HART-IP stream\n";
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}