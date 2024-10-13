#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING

#include "5p_main.hpp"

#define RETURN_WITH_CODE(rc)                      \
    do {                                          \
        LOG_INFO << "returning with code " << rc; \
        return rc;                                \
    } while (0)

// ToDo refactor
int main(int argc, char** argv) {


    /*
     * Read Parameters and set up logger
     */
    common::config config;
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
    pcapreader::Reader reader;

    if (!reader.SetPcapFile(config.path)) {
        RETURN_WITH_CODE(static_cast<int>(returns::ReturnCodes::PCAP_FILE_NOT_FOUND));
    }
    if (config.filter != "" && !reader.ApplyFilter(config.filter)) {
        RETURN_WITH_CODE(static_cast<int>(returns::ReturnCodes::INVALID_FILTER));
    }

    
    /*
     * Create packet handler which forwards the data
     * via boost sockets to desired destination
     */
    packethandler::PacketHandler packerHandler(config);

    /*
     * Hides sleep check logic (whether specified or live
     * timing of packets should be used)
     */
    sleepchecker::SleepChecker sleep_checker(config);

    /*
     * read packets from pcap(ng) file. 
     * Loop until end of file reached.
     */
    pcpp::Packet packet;
    uint64_t counter = 0U;
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

        // send data
        if (packerHandler.AddSender(dataPacket.protocol, config.ip,
                                    dataPacket.port)) {
            packerHandler.Send(dataPacket.protocol, dataPacket.port,
                                dataPacket.payload, dataPacket.payloadLength);
        }
        

    }

    LOG_INFO << "ending with SUCCESS";
    RETURN_WITH_CODE(static_cast<int>(returns::ReturnCodes::SUCCESS));
}
