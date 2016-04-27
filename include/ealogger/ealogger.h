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
// Check for backtrace function
#ifdef __GNUC__
#include <execinfo.h>
#include <cxxabi.h>
#endif

#include "config.h"
/*
 * Check if syslog was detected by CMake
 */
#ifdef SYSLOG
#include <syslog.h>
#endif

#include "logmessage.h"
#include "logqueue.h"

/**
 * @brief ealogger main header
 * @author Christian Rapp (crapp)
 *
 * The ealogger class provides all the functionality you need for your application
 * to log messages.
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
        DEBUG = 0,   /**< Debug message */
        INFO = 1,    /**< Info message */
        WARNING = 2, /**< Warning message */
        ERROR = 3,   /**< Error message */
        FATAL = 4,   /**< Fatal Message */
        INTERNAL = 5 /**< Internal Message, do not use this loglevel yourself */
    };

    /**
     * @brief EALogger constructor
     * @param min_level Minim loglevel, all messages with a lower severity will be discarded
     * @param logToSTDOUT Boolean to indicate whether or not to write to stdout. Can be changed on runtime
     * @param logToFile Boolean to indicate whether or not to write to a logfile. Can be changed on runtime
     * @param logToSyslog Boolean to indicate whether or not to use syslog (for example systemd) on Linux/BSD
     * @param async Boolean if activated ealogger uses a background thread to write messages to a stream
     * @param dt_format The Date Time format specifiers.
     * @param logfile The logfile to use
     *
     * Internally std::strftime is used from header ctime to create formatted time strings.
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
             bool logToSTDOUT = true, bool logToFile = false,
             bool logToSyslog = false, bool async = true,
             bool printThreadID = false, std::string dt_format = "%F %T",
             std::string logfile = "");
    ~EALogger();

    /**
     * @brief Issue a Logmessage
     * @param lvl The severity of the message, EALogger#log_level
     * @param msg The message text
     */
    void write_log(EALogger::log_level lvl, std::string msg);
    // template <typename T>
    // void write_log(EALogger::log_level lvl, T msg);

    /**
     * @brief Debug log message
     *
     * @param msg The message text
     * @details
     * This is a convenience method to directly write a debug message
     */
    void debug(std::string msg);
    /**
     * @brief Info log message
     *
     * @param msg The message text
     * @details
     * This is a convenience method to directly write an info message
     */
    void info(std::string msg);
    /**
     * @brief Warning log message
     *
     * @param msg The message text
     * @details
     * This is a convenience method to directly write a warning message
     */
    void warn(std::string msg);
    /**
     * @brief Error log message
     *
     * @param msg The message text
     * @details
     * This is a convenience method to directly write an error message
     */
    void error(std::string msg);
    /**
     * @brief Fatal log message
     *
     * @param msg The message text
     * @details
     * This is a convenience method to directly write a fatal message
     */
    void fatal(std::string msg);

    /**
     * @brief Print a demangled stacktrace
     * @param size How many elements of the stack you wish to be printed.
     *
     * The method only works with gcc/llvm compiled software.
     */
    void stack_trace(unsigned int size);

    // getters and setters for private members we want to expose

    /**
     * @brief Set the Date Time format specifier
     * @param fmt Format specifier string
     */
    void set_dt_format(std::string fmt);
    std::string get_dt_format();

    /**
     * @brief Enable/Disable logging to stdout
     * @param b
     */
    void setLogToSTDOUT(bool b);
    bool getLogToSTDOUT();

    /**
     * @brief Enable/Disable logging to logfile
     * @param b
     */
    void setLogToFile(bool b);
    bool getLogToFile();

    /**
     * @brief Enable/Disable logging to syslog sink
     * @param b
     */
    void setLogToSyslog(bool b);
    bool getLogToSyslog();

    /**
     * @brief Enable/Disable the priting of the Thread ID.
     * @param b
     */
    void setPrintThreadID(bool b);
    bool getPrintThreadID();

private:
    /** Mutex used when not in async mode */
    std::mutex mtx_log;
    std::mutex mtx_logToSTDOUT;
    std::mutex mtx_logToFile;
    std::mutex mtx_logToSyslog;
    std::mutex mtx_pThreadID;
    std::mutex mtx_dt_format;
    std::mutex mtx_backgroundLoggerStop;

    static bool receivedSIGUSR1;

    /** Minimum severity that is handled */
    EALogger::log_level min_level;

    bool logToSTDOUT;
    bool logToFile;
    bool logToSyslog;
    bool async;
    bool printThreadID;

    std::string dt_format;
    std::string logfilePath;
    std::ofstream logFile;

    /** Threadsafe queue for async mode */
    LogQueue logDataQueue;
    /** Background thread */
    std::thread backgroundLogger;
    /** Controls background logger thread */
    bool backgroundLoggerStop;

    /** lookup table for logelevel Strings */
    std::map<EALogger::log_level, std::string> loglevelStringMap;
    /** map syslog message priority to our loglevels */
    std::map<EALogger::log_level, int> loglevelSyslogMap;

    /** Static Method to be registered for logrotate signal */
    static void logrotate(int signo);
    void logThreadFunc();
    void internalLogRoutine(std::shared_ptr<LogMessage> m);
    std::string getFormattedTimeString(const std::string &timeFormat);
    std::string getFormattedTimeString(std::time_t t,
                                       const std::string &timeFormat);
    /*
     * So far controlling the background logger thread is only possible for the logger
     * object itself.
     */
    bool getBackgroundLoggerStop();
    void setBackgroundLoggerStop(bool stop);
};

#endif  // EALOGGER_H
