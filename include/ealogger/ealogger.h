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
#include "global.h"
#include "sink_console.h"
#include "sink_syslog.h"
#include "sink_file.h"
#include "logmessage.h"
#include "logqueue.h"

namespace con = ealogger_constants;

// Define macros for all log levels and call public member write_log()

/**
 * @def debug(msg)
 * Write a debug message
 */
#define debug(msg) \
    write_log(msg, con::LOG_LEVEL::DEBUG, __FILE__, __LINE__, __func__)
/**
 * @def info(msg)
 * Write a info message
 */
#define info(msg) \
    write_log(msg, con::LOG_LEVEL::INFO, __FILE__, __LINE__, __func__)
/**
 * @def warn(msg)
 * Write a warning message
 */
#define warn(msg) \
    write_log(msg, con::LOG_LEVEL::WARNING, __FILE__, __LINE__, __func__)
/**
 * @def error(msg)
 * Write an error message
 */
#define error(msg) \
    write_log(msg, con::LOG_LEVEL::ERROR, __FILE__, __LINE__, __func__)
/**
 * @def fatal(msg)
 * Write a fatal message
 */
#define fatal(msg) \
    write_log(msg, con::LOG_LEVEL::FATAL, __FILE__, __LINE__, __func__)
/**
 * @def stack()
 * Write a stack message
 */
#define stack() \
    write_log("", con::LOG_LEVEL::STACK, __FILE__, __LINE__, __func__)

/**
 * @brief ealogger main header
 * @author Christian Rapp (crapp)
 *
 * @details
 * The ealogger class provides all the functionality you need for your application
 * to log messages. It can be as simple as in the following example.
 *
 * @code
 * #include "ealogger.h"
 * int main() {
 *     EALogger logger;
 *     logger.debug("My application is just awesome");
 *     return 0;
 * }
 * @endcode
 *
 * ealogger uses sinks to write to different targets. Each sink supports different
 * message conversion pattern and datetime pattern. Each sink can be enabled or
 * disabled independently. To change the configuration you have to provide a SinkConfig
 * object for the corresponding Sink.
 *
 * To make it easy to write messages with a specific severity there are some macro
 * functions for each log level and one for stacktrace.
 * #debug(msg)
 * #info(msg)
 *
 * ealogger and its sinks are threadsafe. Meaning if you use the same instance all over
 * your application it will make sure only one message at a time is written to an
 * iostream for example.
 */
class EALogger
{
public:
    /**
     * @brief EALogger constructor
     * @param async Boolean if activated ealogger uses a background thread to
     * write messages to a stream
     *
     * Use the Parameter async to activate a background logger thread. This way
     * logging will no longer slow down your application which is important for high
     * performance or time critical events. The only overhead is creating a LogMessage
     * object and pushing it in a Queue.
     *
     * The logfile parameter must contain the path and the filename of the logfile.
     * Make sure your application has write permissions at the specified location.
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
     * mylogger.write_log("This is a warning", con::LOG_LEVEL::WARNING,
     *                    __FILE__, __LINE__, __func__)
     * @endcode
     */
    void write_log(std::string msg, con::LOG_LEVEL lvl, std::string file,
                   int lnumber, std::string func);

    std::shared_ptr<SinkConfig> get_sink_config(con::LOGGER_SINK sink);
    void set_sink_config(con::LOGGER_SINK sink, std::shared_ptr<SinkConfig> cfg);

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

    std::map<con::LOGGER_SINK, std::shared_ptr<Sink>> logger_sink_map;

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

#endif  // EALOGGER_H
