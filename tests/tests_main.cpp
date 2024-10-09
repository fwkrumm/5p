#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING


#include <gtest/gtest.h>
#include "5p_main.hpp"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}