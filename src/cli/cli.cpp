#include <5p/cli.hpp>

namespace cli {

static int WrapReturnValue(CLI::App& app, const int argc, char** const argv) {
    CLI11_PARSE(app, argc, argv);
    return 0; // if no parse error
}

int GetParameters(const int argc, char** const argv, config& cfg) {
    CLI::App app{"Parameters"};

    // mandatory parameters
    app.add_option(
        "path_to_pcap_file", cfg.path,
        "path to pcap(ng) file")->required();

    // optional parameters
    app.add_option(
        "--ip", cfg.ip,
        "ip address to forward data. Default is " + cfg.ip);

    app.add_option(
        "--port", cfg.port,
        "port to forward data. Default is " + std::to_string(cfg.port));

    app.add_option(
        "--filter", cfg.filter,
        "Define a filter in Berkeley Packet Filters (BPF) format for network packages");

    app.add_option(
        "--level", cfg.level,
        "select a log level. 0/1 -- DEBUG, 2 -- INFO, 3 -- WARNING, 4 "
        "-- ERROR, 5 -- EXCEPTION, 6 -- OFF. Default is " +
            std::to_string(static_cast<uint16_t>(cfg.level)));

    app.add_option(
        "--protocol", cfg.protocol,
        "select protocol to use for socket forwarding. 1 -- TCP, 2 -- "
        "UDP. Default is " +
            std::to_string(static_cast<uint16_t>(cfg.protocol)));

    app.add_option(
        "--sleep", cfg.sleep,
        "select sleep time between samples. -1 means use live "
        "time (within clock precision) Default is " +
            std::to_string(cfg.sleep));

    app.add_option(
        "--skip", cfg.skip,
        "skecify number of samples to be skipped at "
        "the begining of the file. Default is " +
            std::to_string(cfg.skip));

    // prase commands
    auto rc = WrapReturnValue(app, argc, argv);

    if (app.get_help_ptr()->count() > 0) {
        // prevent program execution after help print.
        // NOTE that also CLI11_PARSE might return -1
        return -1;
    }

    // return cli 11 parse error code or 0 if no parse error occured
    return rc; 

}

}    // namespace cli