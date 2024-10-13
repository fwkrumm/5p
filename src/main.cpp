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

    packethandler::PacketHandler packerHandler;
    bool manual_socket =
        config.port > 0 &&
        config.protocol != common::ProtocolType::PACKET_PROTOCOLS;

    // if port has specified and protocol -> init sender and lock adding of new
    // senders since via parameters only one protocol + port has been specified
    if (manual_socket) {
        if (!packerHandler.AddSender(config.protocol, config.ip, config.port)) {
            packerHandler.CleanMap();
            return static_cast<int>(returns::ReturnCodes::SOCKET_FAILURE);
        }
        packerHandler.SetLockTo(true);
    }

    
    // read packets from pcap file
    pcpp::Packet packet;
    uint64_t counter = 0U;

    sleepchecker::SleepChecker sleep_checker(config);

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

       // apply sleep if required
       sleep_checker.CheckSleep(dataPacket.timestamp); 

       if (manual_socket) {
           packerHandler.Send(config.protocol, config.port, dataPacket.payload,
                              dataPacket.payloadLength);
           continue;
       } 

       // use data packet socket
       if (!packerHandler.DoesSenderExist(dataPacket.protocol,
                                          dataPacket.port)) {
           packerHandler.AddSender(dataPacket.protocol, config.ip,
                                   dataPacket.port);
       }

        // send
        packerHandler.Send(dataPacket.protocol, dataPacket.port,
                            dataPacket.payload,
                            dataPacket.payloadLength);

    }

    LOG_INFO << "ending cleanly";
    return static_cast<int>(returns::ReturnCodes::SUCCESS);
}
