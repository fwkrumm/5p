#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING

#include "5p_main.hpp"

int main(int argc, char** argv) {
    cli::config config;

    // get config from parameters
    auto rc = cli::GetParameters(argc, argv, config);

    // check for mandatory parameters; this also returns if --help is printed.
    if (rc == -1) {
        return static_cast<int>(returns::ReturnCodes::HELP_PRINTED);
    } else if (rc != 0) {
        return static_cast<int>(returns::ReturnCodes::MISSING_PARAMETERS);
    }

    // set log level
    logging_5p::SetUpLogger(config.level);

    // create reader and set pcap file
    ppppp::Reader reader;
    if (!reader.SetPcapFile(config.path)) {
        return static_cast<int>(returns::ReturnCodes::PCAP_FILE_NOT_FOUND);
    }

    // apply filter if one was set
    if (config.filter != "" && !reader.ApplyFilter(config.filter)) {
        return static_cast<int>(returns::ReturnCodes::INVALID_FILTER);
    }

    // init sender
    sender::DataSender sender(config.protocol, config.ip,
                              config.port);    // for testing only udp
    if (!sender.Init()) {
        LOG_ERROR << "socket init failed.";
        return static_cast<int>(returns::ReturnCodes::SOCKET_FAILURE);
    }

    // read packets from pcap file
    pcpp::Packet packet;
    while (reader.NextPackage(packet)) {
        ppppp::DataPacket dataPacket = reader.ToDataPacket(packet);

        sender.Send(dataPacket.payload, dataPacket.payloadLength);
    }

    LOG_INFO << "ending cleanly";
    return static_cast<int>(returns::ReturnCodes::SUCCESS);
}
