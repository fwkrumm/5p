cmake_minimum_required(VERSION 3.15)
set(PROJECT_NAME "5p")

if (WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00)
endif()

project(${PROJECT_NAME} CXX)


find_package(CLI11 REQUIRED)
find_package(Boost REQUIRED)
find_package(PcapPlusPlus REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/cli/cli.cpp
	src/sender/sender.cpp
    src/pcapreader/pcapreader.cpp
)


target_link_libraries(${PROJECT_NAME}
    CLI11::CLI11
    boost::boost
    PcapPlusPlus::PcapPlusPlus
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/includes
    ${PcapPlusPlus_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${CLI11_INCLUDE_DIRS}
)
include_directories(
    ${PcapPlusPlus_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${CLI11_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/includes
)
