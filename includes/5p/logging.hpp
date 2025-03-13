#pragma once

#include <5p/common.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <chrono>
#include <iostream>

namespace logging = boost::log;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

// get filename from __FILE__ for logging makros
inline const std::string filename(const char* path) {
    return boost::filesystem::path(path).filename().string();
}

// define logging format
#define LOG_INFO            \
    BOOST_LOG_TRIVIAL(info) \
        << " [" << filename(__FILE__) << ":" << __LINE__ << "] "
#define LOG_WARNING            \
    BOOST_LOG_TRIVIAL(warning) \
        << " [" << filename(__FILE__) << ":" << __LINE__ << "] "
#define LOG_DEBUG            \
    BOOST_LOG_TRIVIAL(debug) \
        << " [" << filename(__FILE__) << ":" << __LINE__ << "] "
#define LOG_EXCEPTION        \
    BOOST_LOG_TRIVIAL(fatal) \
        << " [" << filename(__FILE__) << ":" << __LINE__ << "] "
#define LOG_ERROR            \
    BOOST_LOG_TRIVIAL(error) \
        << " [" << filename(__FILE__) << ":" << __LINE__ << "] "

namespace logging_5p {

// in case for throw log first and then throw
#ifndef FLUSH_BEFORE_THROW
#define FLUSH_BEFORE_THROW(EXCEPTION_TYPE, MESSAGE) \
    {                                               \
        auto core = boost::log::core::get();        \
        core->flush();                              \
        throw EXCEPTION_TYPE(MESSAGE);              \
    }
#endif

/*
 * Set up logger with file and console sink. log level to file
 * is always debug, log level to console is set by parameter.
 * @param level: minimum log level to be logged
 * @return void
 */
static void SetUpLogger(common::LogLevel level) {
    int64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();

    // remove sinks
    boost::log::core::get()->remove_all_sinks();
    logging::add_common_attributes();

    // set up logging to file
    auto file_sink = logging::add_file_log(
        keywords::file_name = "5pLog_" + std::to_string(timestamp) + ".log",
        keywords::open_mode =
            std::ios_base::app,    // if already exists -> append
        keywords::format =
            (expr::stream << expr::format_date_time<boost::posix_time::ptime>(
                                 "TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
                          << " " << logging::trivial::severity
                          << expr::smessage));
    // to file -> always debug
    file_sink->set_filter(logging::trivial::severity >=
                          logging::trivial::debug);

    // log to console
    auto console_sink = logging::add_console_log(
        std::cout,
        keywords::format =
            (expr::stream << expr::format_date_time<boost::posix_time::ptime>(
                                 "TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
                          << " " << logging::trivial::severity
                          << expr::smessage));

    console_sink->set_filter(logging::trivial::severity >=
                             static_cast<uint16_t>(level));
}

}    // namespace logging_5p