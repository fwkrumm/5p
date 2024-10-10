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
	src/main.cpp
    src/cli/cli.cpp
	src/sender/sender.cpp
    src/pcapreader/pcapreader.cpp
)

add_executable(${PROJECT_NAME}
    ${SOURCE_FILES_5P}
)

add_library(${PROJECT_NAME_LIB}
    ${SOURCE_FILES_5P}
)

add_executable(${TEST_NAME}
    tests/tests_main.cpp
    tests/tests_config.cpp
)

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