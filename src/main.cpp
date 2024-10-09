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
    sender::DataSender sender(config.protocol, config.ip, config.port);
    if (!sender.Init()) {
        LOG_ERROR << "socket init failed.";
        return static_cast<int>(returns::ReturnCodes::SOCKET_FAILURE);
    }

    // read packets from pcap file
    pcpp::Packet packet;
    uint64_t lastDataPacketTimestamp = 0U;
    uint64_t counter = 0U;

    // loop until end of file reached
    while (reader.NextPackage(packet)) {

        // skip samples if specified; not yet opimal since each
        // packet is read from file, check if pcapplusplus offers
        // an api for this task
        if (++counter < config.skip) {
            continue;
        }

        // extract data packet
        ppppp::DataPacket dataPacket = reader.ToDataPacket(packet);

        // check if sleep interval as been specified
        if (config.sleep == -1 && lastDataPacketTimestamp > 0U) {
            // time sleep according to data packet
            auto diffBetweenSamples =
                dataPacket.timestamp - lastDataPacketTimestamp;
            LOG_DEBUG << "according to data packet diff, sleeping "
                      << diffBetweenSamples << " ms.";
            std::this_thread::sleep_for(std::chrono::milliseconds(diffBetweenSamples));
        } else if (config.sleep > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(config.sleep));
        } // else no sleep, as fast as possible

        // send via socket
        sender.Send(dataPacket.payload, dataPacket.payloadLength);

        // save timestamp from last data packet
        lastDataPacketTimestamp = dataPacket.timestamp;
    }

    LOG_INFO << "ending cleanly";
    return static_cast<int>(returns::ReturnCodes::SUCCESS);
}
