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
#include <fstream>
#include <sstream>
#include <csignal>
#include <stdexcept>
#include <regex>
#include <map>
#include <ctime>
/*
 * Background logger thread
 */
#include <thread>
#include <vector>

#include "config.h"
#include "global.h"
#include "sink_console.h"
#include "logmessage.h"
#include "logqueue.h"

namespace con = ealogger_constants;

// Define macros for all log levels and call public member write_log()

#define debug(msg) \
    write_log(msg, EALogger::log_level::DEBUG, __FILE__, __LINE__, __func__)
#define info(msg) \
    write_log(msg, EALogger::log_level::INFO, __FILE__, __LINE__, __func__)
#define warn(msg) \
    write_log(msg, EALogger::log_level::WARNING, __FILE__, __LINE__, __func__)
#define error(msg) \
    write_log(msg, EALogger::log_level::ERROR, __FILE__, __LINE__, __func__)
#define fatal(msg) \
    write_log(msg, EALogger::log_level::FATAL, __FILE__, __LINE__, __func__)
#define stack(msg) \
    write_log(msg, EALogger::log_level::STACK, __FILE__, __LINE__, __func__)

/**
 * @brief ealogger main header
 * @author Christian Rapp (crapp)
 *
 * @details
 * The ealogger class provides all the functionality you need for your application
 * to log messages. It can be as simple as in the following example.
 *
 * @code
 * EALogger logger;
 * logger.debug("My application is just awesome");
 * @endcode
 *
 * First you have to decide if you want to write to the console, a file or syslog.
 * The asynchronuous can be turned on or off and you can of course change the date
 * time format. The EALogger::EALogger constructor has additional details on the
 * different parameters and options.
 *
 * ealogger itself is threadsafe. Meaning if you use the same instance all over
 * your application it will make sure only one message at a time is written to an
 * iostream for example.
 */
class EALogger
{
public:
    /**
     * @brief An enumaration representing the supported loglevels.
     *
     * This enum is used to define the severity of a log message and to set the
     * minimum loglevel.
     */
    enum log_level {
        DEBUG = 0, /**< Debug message */
        INFO,      /**< Info message */
        WARNING,   /**< Warning message */
        ERROR,     /**< Error message */
        FATAL,     /**< Fatal Message */
        STACK,     /**< Stack log message */
        INTERNAL   /**< Internal Message, do not use this loglevel yourself */
    };

    /**
     * @brief EALogger constructor
     * @param min_level Minimum loglevel, all messages with a lower severity will
     * be discarded
     * @param log_to_console Boolean to indicate whether or not to write to console.
     * Can be changed on runtime
     * @param log_to_file Boolean to indicate whether or not to write to a file.
     * Can be changed on runtime
     * @param log_to_syslog Boolean to indicate whether or not to use syslog
     * (for example systemd) on Linux/BSD
     * @param async Boolean if activated ealogger uses a background thread to
     * write messages to a stream
     * @param dt_format The Date Time format specifiers
     * @param logfile The logfile to use
     *
     * Internally std::strftime is used from header ctime to create formatted time
     * strings.
     * So for parameter dt_format you have to use format specifiers that are
     * recognised by strftime. See [en.cppreference.com](http://en.cppreference.com/w/cpp/chrono/c/strftime)
     * for details.
     *
     * For example "%H:%M:%S" returns a 24-hour based time string like 20:12:01
     *
     * You can switch logging to all streams and sinks on and off seperatly.
     *
     * Use the Parameter async to activate a background logger thread. This way
     * logging will no longer slow down your application which is important for high
     * performance or time critical events. The only overhead is creating a LogMessage
     * object and pushing it in a Queue.
     *
     * The logfile parameter must contain the path and the filename of the logfile.
     * Make sure your application has write permissions at the specified location.
     *
     * The constructor automatically registers a signal handler for SIGUSR1.
     * This allows logrotation with logrotate on supported systems
     */
    EALogger(log_level min_level = EALogger::log_level::DEBUG,
             bool log_to_console = true, bool log_to_file = false,
             bool log_to_syslog = false, bool async = true,
             std::string dt_format = "%F %T", std::string logfile = "");
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
     * This mehtod is called by the macros that are defined in this header file
     * for the different log levels. You can of course call this method yourself
     * @code
     * mylogger.write_log("This is a warning", EALogger::log_level::WARNING,
     *                    __FILE__, __LINE__, __func__)
     * @endcode
     */
    void write_log(std::string msg, EALogger::log_level lvl, std::string file,
                   int lnumber, std::string func);

    // getters and setters for private members we want to expose
    /**
     * @brief Set the Date Time format specifier
     * @param fmt Format specifier string
     */
    void set_dt_format(std::string fmt);
    std::string get_dt_format();

    /**
     * @brief Enable/Disable logging to stdo
     * @param b
     */
    void set_log_to_console(bool b);
    bool get_log_to_console();

    /**
     * @brief Enable/Disable logging to logfile
     * @param b
     */
    void set_log_to_file(bool b);
    bool get_log_to_file();

    /**
     * @brief Enable/Disable logging to syslog sink
     * @param b
     */
    void set_log_to_syslog(bool b);
    bool get_log_to_syslog();

private:
    /** Mutex used when not in async mode */
    std::mutex mtx_log;
    std::mutex mtx_log_console;
    std::mutex mtx_log_file;
    std::mutex mtx_log_syslog;
    std::mutex mtx_dt_format;
    std::mutex mtx_logger_stop;

    static bool signal_SIGUSR1;

    /** Minimum severity that is handled */
    EALogger::log_level min_level;

    bool log_to_console;
    bool log_to_file;
    bool log_to_syslog;
    bool async;

    std::string dt_format;
    std::string logfile_path;
    std::ofstream logfile_stream;

    /** Threadsafe queue for async mode */
    LogQueue log_msg_queue;
    /** Background thread */
    std::thread logger_thread;
    /** Controls background logger thread */
    bool logger_thread_stop;

    /** lookup table for loglevel Strings */
    std::map<EALogger::log_level, std::string> loglevel_lookup;
    /** map syslog message priority to our loglevels */
    std::map<EALogger::log_level, int> loglevel_syslog_lookup;

    std::map<con::LOGGER_SINK, Sink> map_logger_sink;

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
