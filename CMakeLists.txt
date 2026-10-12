cmake_minimum_required(VERSION 3.15)
set(PROJECT_NAME "5p")
set(PROJECT_NAME_LIB "5plib")
set(TEST_NAME "5p_tests")

if (WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00)
endif()

add_definitions(-DBOOST_ALL_NO_LIB)

project(${PROJECT_NAME} CXX)

find_package(CLI11 REQUIRED)
find_package(Boost REQUIRED)
find_package(PcapPlusPlus REQUIRED)
find_package(GTest REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(SOURCE_FILES_5P
    src/cli.cpp
    src/sender.cpp
    src/pcapreader.cpp
    src/rawreplay.cpp
    src/packethandler.cpp
    src/sleepchecker.cpp
)

add_executable(${PROJECT_NAME}
	src/main.cpp
)

add_library(${PROJECT_NAME_LIB}
    ${SOURCE_FILES_5P}
)

add_executable(${TEST_NAME}
    tests/tests_main.cpp
    tests/tests_config.cpp
    tests/tests_rawreplay.cpp
)

if (UNIX)
    add_executable(5p_integration_tests tests/integration/replay.cpp)
    add_dependencies(5p_integration_tests 5p)
    target_link_libraries(5p_integration_tests gtest::gtest boost::boost PcapPlusPlus::PcapPlusPlus)
    target_compile_definitions(5p_integration_tests PRIVATE
        REPLAY_EXECUTABLE="$<TARGET_FILE:5p>"
        CAPTURE_PATH="${CMAKE_SOURCE_DIR}/resources/tests/hart_ip.pcap"
    )
endif()

set(LIBRARIES_5P
    CLI11::CLI11
    boost::boost
    PcapPlusPlus::PcapPlusPlus
)

set(INCLUDES_5P
    ${CMAKE_SOURCE_DIR}/includes
    ${PcapPlusPlus_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${CLI11_INCLUDE_DIRS}
)

target_link_libraries(${TEST_NAME}
    gtest::gtest
    ${LIBRARIES_5P}
    ${PROJECT_NAME_LIB}
)

target_link_libraries(${PROJECT_NAME}
    ${LIBRARIES_5P}
	${PROJECT_NAME_LIB}
)

# includes redundant since otherwise vs studio
# had problems with intellisense
target_include_directories(${PROJECT_NAME} PRIVATE
    ${INCLUDES_5P}
)
include_directories(
    ${INCLUDES_5P}
)
target_include_directories(${TEST_NAME} PRIVATE
    ${INCLUDES_5P}
)
include_directories(
    ${INCLUDES_5P}
)

enable_testing()
add_test(NAME "5p_tests" COMMAND ${TEST_NAME})
if (UNIX)
    add_test(NAME "5p_integration_tests" COMMAND 5p_integration_tests)
    set_tests_properties("5p_integration_tests" PROPERTIES TIMEOUT 15)
endif()