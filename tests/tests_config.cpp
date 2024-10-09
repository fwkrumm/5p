#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING

#include <gtest/gtest.h>

#include "5p_main.hpp"

// Example test
TEST(SampleTest, AssertionTrue) {
    ASSERT_TRUE(true);

    int argc = 3;
    char arg1[] = "program";
    char arg2[] = "42";
    char arg3[] = "test";
    char* argv[] = {arg1, arg2, arg3, nullptr};
    cli::config cfg;
    cli::GetParameters(argc, argv, cfg);
}
