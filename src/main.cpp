#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING

#include "5p_main.hpp"

// helper to log return code at info level before returning it
#define RETURN_WITH_CODE(rc)                      \
    do {                                          \
        LOG_INFO << "returning with code " << rc; \
        return rc;                                \
    } while (0)


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
     * Create reader and apply filter if specified; if trace level is specified, enable debug logging for pcapplusplus library
     */
    pcapreader::Reader reader(config.level == common::LogLevel::TRACE_LEVEL);

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

    // to record run times
    int64_t start = std::chrono::duration_cast<std::chrono::microseconds>(
                        std::chrono::system_clock::now().time_since_epoch())
                        .count();
    int64_t end = 0;

    while (reader.NextPackage(packet)) {

        // skip samples if specified; apparently there
        // is not API for this
        if (++counter < config.skip) {
            continue;
        }

        // extract data packet
        common::DataPacket dataPacket = reader.ToDataPacket(packet);

        end = std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();

        if (dataPacket.timestamp == 0U) {
            LOG_DEBUG << "skipping timestamp 0 packet (potentially not correctly reassembled).";
            continue;
        }

        // apply sleep if required
        sleep_checker.CheckSleep(dataPacket.timestamp, end - start);

        start = std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();

        // send data
        if (packerHandler.AddSender(dataPacket.protocol, config.ip,
                                    dataPacket.port)) {
            auto rv = packerHandler.Send(dataPacket.protocol, dataPacket.port,
                                    dataPacket.payload, dataPacket.payloadLength);
            LOG_DEBUG << "Sending packet of size " << dataPacket.payloadLength
                      << " returned: " << rv;
        }


    }

    if (counter == 0U) {
        LOG_WARNING << "no packets found in pcap file. Probably there are not packets for the "
                       "specified filter or the file is corrupt. You might want to try to open the pcap "
                       "file (with e.g. Wireshark) and cut away the first ~5 packets and safe this as a new file.";
    }

    LOG_INFO << "ending with SUCCESS";
    RETURN_WITH_CODE(static_cast<int>(returns::ReturnCodes::SUCCESS));
}
