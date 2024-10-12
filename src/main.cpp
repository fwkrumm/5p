#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING

#include "5p_main.hpp"

int main(int argc, char** argv) {
    common::config config;

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
    pcapreader::Reader reader;
    if (!reader.SetPcapFile(config.path)) {
        return static_cast<int>(returns::ReturnCodes::PCAP_FILE_NOT_FOUND);
    }

    // apply filter if one was set
    if (config.filter != "" && !reader.ApplyFilter(config.filter)) {
        return static_cast<int>(returns::ReturnCodes::INVALID_FILTER);
    }

    // for each uint16_t port a sender
    // TODO enable this also for different protocols? i.e. nested map?
    std::unordered_map<uint16_t, sender::DataSender*> senders;
    
    if (config.port > 0U) {
        senders[config.port] = new sender::DataSender(config.protocol, config.ip, config.port);
        if (!senders[config.port]->Init()) {
            LOG_ERROR << "socket init on port " << config.port << " failed.";
            return static_cast<int>(returns::ReturnCodes::SOCKET_FAILURE);
        }
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
       common::DataPacket dataPacket = reader.ToDataPacket(packet);

        // check if sleep is desired; skip sleep until at least one packet was processed
        if (lastDataPacketTimestamp > 0U) {

            // sleep == -1 i.e. use approx. real time from packet timestamp
            if (config.sleep == -1) {
                // time sleep according to data packet
                auto diffBetweenSamples =
                    dataPacket.timestamp - lastDataPacketTimestamp;
                LOG_DEBUG << "according to data packet diff, sleeping "
                          << diffBetweenSamples << " ms.";
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(diffBetweenSamples));
            } else if (config.sleep > 0) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(config.sleep));
            }    
            // else no sleep, as fast as possible
        }

        if (config.port > 0U) {
            // port manually specified, socket init was checked before.
            senders[config.port]->Send(dataPacket.payload,
                                       dataPacket.payloadLength);
        } else if (dataPacket.port > 0U) {

            // TODO add function and exclude some ports

            if (senders.count(dataPacket.port) == 0) {
                // if key does not yet exist -> add socket for this port to map
                senders[dataPacket.port] = new sender::DataSender(config.protocol,
                                                                  config.ip,
                                                                  dataPacket.port);
                if (!senders[dataPacket.port]->Init()) {
                    LOG_ERROR << "socket init on port " << dataPacket.port
                              << " failed.";
                    return static_cast<int>(
                        returns::ReturnCodes::SOCKET_FAILURE);
                }
            }
            
            // send on data packet port
            senders[dataPacket.port]->Send(dataPacket.payload,
                                       dataPacket.payloadLength);
        }

        // save timestamp from last data packet
        lastDataPacketTimestamp = dataPacket.timestamp;
    }

    // clean up
    for (auto& pair : senders) {
        delete pair.second;
    }
    senders.clear();

    LOG_INFO << "ending cleanly";
    return static_cast<int>(returns::ReturnCodes::SUCCESS);
}
