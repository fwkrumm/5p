#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING

#include <gtest/gtest.h>

#include "5p_main.hpp"

// check that config is correctly created from argument string
TEST(ArgumentTest, AssertionTrue) {
    
    // fake some input parameters
    int argc = 16;
    char arg1[] = "5p";
    char arg2[] = "pcap_trace.pcapng";
    char arg3[] = "--sleep";
    char arg4[] = "123";
    char arg5[] = "--skip";
    char arg6[] = "312";
    char arg7[] = "--ip";
    char arg8[] = "8.8.8.8";
    char arg9[] = "--port";
    char arg10[] = "9999";
    char arg11[] = "--filter";
    char arg12[] = "not a valid filter";
    char arg13[] = "--protocol";
    char arg14[] = "1";
    char arg15[] = "--level";
    char arg16[] = "5";
    char* argv[] = {arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, nullptr};

    // create config struct
    common::config cfg;
    cli::GetParameters(argc, argv, cfg);

    // check if values correctly transformed to config struct
    ASSERT_EQ(cfg.sleep, 123);
    ASSERT_EQ(cfg.skip, 312);
    ASSERT_EQ(cfg.port, 9999);
    ASSERT_EQ(cfg.protocol,common::ProtocolType::TCP);
    ASSERT_EQ(cfg.level, common::LogLevel::EXCEPTOIN_LEVEL); // log level 5
    ASSERT_EQ(cfg.ip, "8.8.8.8");
    ASSERT_EQ(cfg.path, "pcap_trace.pcapng");
    ASSERT_EQ(cfg.filter, "not a valid filter");

}
