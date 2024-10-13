#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING

#include "5p_main.hpp"

#define RETURN_WITH_CODE(rc)                      \
    do {                                          \
        LOG_INFO << "returning with code " << rc; \
        return rc;                                \
    } while (0)

// ToDo refactor
int main(int argc, char** argv) {

    // parameters -> config
    common::config config;

    // read network packets from pcap(ng) file
    pcapreader::Reader reader;

    // handle network packets -> boost sockets
    packethandler::PacketHandler packerHandler;

    

    /*
     * Read Parameters and set up logger
     */

    auto rc = cli::GetParameters(argc, argv, config);

    // check for mandatory parameters; this also returns if --help is printed.
    if (rc == -1) {
        RETURN_WITH_CODE(static_cast<int>(returns::ReturnCodes::HELP_PRINTED));
    } else if (rc != 0) {
        RETURN_WITH_CODE(static_cast<int>(returns::ReturnCodes::MISSING_PARAMETERS));
    }
    
    logging_5p::SetUpLogger(config.level);


    /*
     * Create reader and apply filter if specified
     */
    if (!reader.SetPcapFile(config.path)) {
        RETURN_WITH_CODE(static_cast<int>(returns::ReturnCodes::PCAP_FILE_NOT_FOUND));
    }
    if (config.filter != "" && !reader.ApplyFilter(config.filter)) {
        RETURN_WITH_CODE(static_cast<int>(returns::ReturnCodes::INVALID_FILTER));
    }

    
    /*
     * ToDO hide functionality in class?
     * ToDo add that one can also specify ONLY port or ONLY protocol
     */

    bool manual_socket =
        config.port > 0 &&
        config.protocol != common::ProtocolType::PACKET_PROTOCOLS;

    // if port has specified AND protocol -> init sender and lock adding of new
    // senders, since via parameters only one protocol + port is disired. if socket
    // initialization fails -> exit here
    if (manual_socket) {
        if (!packerHandler.AddSender(config.protocol, config.ip, config.port)) {
            packerHandler.CleanMap();
            RETURN_WITH_CODE(static_cast<int>(returns::ReturnCodes::SOCKET_FAILURE));
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

    LOG_INFO << "ending with SUCCESS";
    RETURN_WITH_CODE(static_cast<int>(returns::ReturnCodes::SUCCESS));
}
