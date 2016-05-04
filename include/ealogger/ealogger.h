//   ealogger is a simple, asynchronous and powerful logger library for c++
//   Copyright 2013 - 2016 Christian Rapp
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#ifndef EALOGGER_H
#define EALOGGER_H

/**
 * @file ealogger.h
 */

#include <string>
/*
 * Mutual exclusion for threadsafe logger
 */
#include <mutex>
#include <iostream>
#include <csignal>
#include <stdexcept>
/*
 * Background logger thread
 */
#include <thread>
#include <vector>
#include <map>

#include "config.h"
#include "ealogger/global.h"
#include "ealogger/sink_console.h"
#include "ealogger/sink_syslog.h"
#include "ealogger/sink_file.h"
#include "ealogger/logmessage.h"
#include "ealogger/logqueue.h"

// Define macros for all log levels and call public member write_log()
/**
 * @defgroup EALOGGER_GROUP
 *
 * @{
 */

/**
 * @def debug(msg)
 * @brief Write a debug message
 */
#define debug(msg)                                                           \
    write_log(msg, ealogger_constants::LOG_LEVEL::DEBUG, __FILE__, __LINE__, \
              __func__)
/**
 * @def info(msg)
 * @brief Write a info message
 */
#define info(msg)                                                           \
    write_log(msg, ealogger_constants::LOG_LEVEL::INFO, __FILE__, __LINE__, \
              __func__)
/**
 * @def warn(msg)
 * Write a warning message
 */
#define warn(msg)                                                              \
    write_log(msg, ealogger_constants::LOG_LEVEL::WARNING, __FILE__, __LINE__, \
              __func__)
/**
 * @def error(msg)
 * Write an error message
 */
#define error(msg)                                                           \
    write_log(msg, ealogger_constants::LOG_LEVEL::ERROR, __FILE__, __LINE__, \
              __func__)
/**
 * @def fatal(msg)
 * Write a fatal message
 */
#define fatal(msg)                                                           \
    write_log(msg, ealogger_constants::LOG_LEVEL::FATAL, __FILE__, __LINE__, \
              __func__)
/**
 * @def stack()
 * Write a stack message
 */
#define stack()                                                             \
    write_log("", ealogger_constants::LOG_LEVEL::STACK, __FILE__, __LINE__, \
              __func__)

/**
 * @brief ealogger main class
 * @author Christian Rapp (crapp)
 *
 * @details
 * The EALogger class provides all the functionality you need for your application
 * to log messages. It can be as simple as in the following example.
 *
 * @code
 * #include "ealogger/ealogger.h"
 * int main() {
 *     EALogger logger;
 *     logger.init_console_sink();
 *     logger.debug("My application is just awesome");
 *     return 0;
 * }
 * @endcode
 *
 * This will print *My application is awesome* to the console.
 *
 * ealogger uses sinks to write to different targets. Each Sink supports different
 * message [conversion patterns](@ref ConversionPattern) and a datetime pattern.
 * Each sink can be enabled or disabled independently and the minimum severity can be set.
 * You have to use EALogger::init_console_* to init a sink. This methods define sane
 * defaults for their options. But if course you can change them to whatever you
 * want.
 *
 * The Methods set_msg_pattern, set_datetime_pattern, set_enabled and set_min_lvl
 * allow you to change the configuration of a sink. If the sink supports more
 * options you have to use the dedicated init method to reinitialize the sink.
 *
 * To make it easy to write messages with a specific severity there are some macro
 * functions for each log level and one for stacktrace.
 * #debug(msg)
 * #info(msg)
 * EALogger::write_log allows you to write log messages without using these macros.
 *
 * ealogger and its sinks are threadsafe. Meaning if you use the same instance all
 * over your application it will make sure only one message at a time is written
 * to an iostream and the internal message queue is synchronized.
 */
class EALogger
{
public:
    /**
     * @brief EALogger constructor
     * @param async Boolean if activated ealogger uses a background thread to
     * write messages to a Sink
     *
     * Use the Parameter async to activate a background logger thread. This way
     * logging will no longer slow down your application which is important for high
     * performance or time critical events. The only overhead is creating a LogMessage
     * object and pushing it on a std::queue.
     *
     *
     */
    EALogger(bool async = true);
    ~EALogger();

    /**
     * @brief Issue a Logmessage
     * @param lvl The severity of the message, EALogger#log_level
     * @param msg The message text
     */

    /**
     * @brief Write a log message
     *
     * @param msg Message text
     * @param lvl Severity of the message, EALogger#log_level
     * @param file File from where the mehtod was called
     * @param lnumber Line number
     * @param func Function name
     *
     * @details
     *
     * This method is called by the macros that are defined in this header file
     * for the different log levels. You can of course call this method yourself
     * @code
     * mylogger.write_log("This is a warning", ealogger_constants::LOG_LEVEL::WARNING,
     *                    __FILE__, __LINE__, __func__)
     * @endcode
     */
    void write_log(std::string msg, ealogger_constants::LOG_LEVEL lvl,
                   std::string file, int lnumber, std::string func);

    void init_syslog_sink(bool enabled = true,
                          ealogger_constants::LOG_LEVEL min_lvl =
                              ealogger_constants::LOG_LEVEL::DEBUG,
                          std::string msg_pattern = "%s: %m",
                          std::string datetime_pattern = "%F %T");
    void init_console_sink(bool enabled = true,
                           ealogger_constants::LOG_LEVEL min_lvl =
                               ealogger_constants::LOG_LEVEL::DEBUG,
                           std::string msg_pattern = "%d %s: %m",
                           std::string datetime_pattern = "%F %T");

    void init_file_sink(bool enabled = true,
                        ealogger_constants::LOG_LEVEL min_lvl =
                            ealogger_constants::LOG_LEVEL::DEBUG,
                        std::string msg_pattern = "%d %s [%f:%l] %m",
                        std::string datetime_pattern = "%F %T",
                        std::string logfile = "ealogger_logfile.log");
    void init_file_sink_rotating(bool enabled,
                                 ealogger_constants::LOG_LEVEL min_lvl,
                                 std::string msg_pattern,
                                 std::string datetime_pattern,
                                 std::string logfile);
    /**
     * @brief Discard a Sink and delete the object
     *
     * @param sink
     * @details
     *
     * You can discard a Sink this will reduce some overhead and can speed logging
     * a little bit.
     */
    void discard_sink(ealogger_constants::LOGGER_SINK sink);

    /**
     * @brief Set message conversion pattern for a Sink
     *
     * @param sink
     * @param msg_pattern
     */
    void set_msg_pattern(ealogger_constants::LOGGER_SINK sink,
                         std::string msg_pattern);
    /**
     * @brief Set datetime conversion pattern for a Sink
     *
     * @param sink
     * @param datetime_pattern
     *
     * @details
     * Date time conversion is based on [std::strftime](http://en.cppreference.com/w/cpp/chrono/c/strftime)
     * You can use all the conversion specifiers mentioned there.
     */
    void set_datetime_pattern(ealogger_constants::LOGGER_SINK sink,
                              std::string datetime_pattern);
    /**
     * @brief Activate or deactivate a Sink
     *
     * @param sink
     * @param enabled
     */
    void set_enabled(ealogger_constants::LOGGER_SINK sink, bool enabled);
    /**
     * @brief Set the minimum log message severity for a Sink
     *
     * @param sink
     * @param min_level
     */
    void set_min_lvl(ealogger_constants::LOGGER_SINK sink,
                     ealogger_constants::LOG_LEVEL min_level);

    /**
     * @brief Check if the message queue is empty
     *
     * @return True if the message is empty, otherwise false.
     */
    bool queue_empty();

private:
    /** Mutex used when not in async mode */
    std::mutex mtx_logger_stop;

    static bool signal_SIGUSR1;

    bool async;

    /** Threadsafe queue for async mode */
    LogQueue log_msg_queue;
    /** Background thread */
    std::thread logger_thread;
    /** Controls background logger thread */
    bool logger_thread_stop;

    std::map<ealogger_constants::LOGGER_SINK, std::shared_ptr<Sink>>
        logger_sink_map;
    std::map<ealogger_constants::LOGGER_SINK, std::unique_ptr<std::mutex>>
        logger_mutex_map;

    /** Static Method to be registered for logrotate signal */
    static void logrotate(int signo);

    void thread_entry_point();

    /**
     * @brief This method writes the LogMessage to all activated sinks
     *
     * @param m LogMessage
     */
    void internal_log_routine(std::shared_ptr<LogMessage> m);

    /*
     * So far controlling the background logger thread is only possible for the logger
     * object itself.
     */
    bool get_logger_thread_stop();
    void set_logger_thread_stop(bool stop);
};

/**
 *
 * @}
 *
 */

#endif  // EALOGGER_H
