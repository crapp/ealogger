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
#include <map>
/*
 * Background logger thread
 */
#include <thread>
#include <vector>
#ifdef __GNUC__
#include <execinfo.h>
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
    enum logLevels {
        DEBUG = 0,   /**< Debug message */
        INFO = 1,    /**< Info message */
        WARNING = 2, /**< Warning message */
        ERROR = 3,   /**< Error message */
        FATAL = 4,   /**< Fatal Message */
        INTERNAL = 5 /**< Internal Message, do not use this loglevel yourself */
    };

    /**
     * @brief EALogger constructor
     * @param minLvl Minim loglevel, all messages with a lower severity will be discarded
     * @param logToSTDOUT Boolean to indicate whether or not to write to stdout. Can be changed on runtime
     * @param logToFile Boolean to indicate whether or not to write to a logfile. Can be changed on runtime
     * @param logToSyslog Boolean to indicate whether or not to use syslog (for example systemd) on Linux/BSD
     * @param multithreading Boolean if activated ealogger uses a background thread to write messages to a stream
     * @param dateTimeFormat The Date Time format specifiers.
     * @param logfile The logfile to use
     *
     * Internally std::strftime is used from header ctime to create formatted time strings.
     * So for parameter dateTimeFormat you have to use format specifiers that are
     * recognised by strftime. See [en.cppreference.com](http://en.cppreference.com/w/cpp/chrono/c/strftime)
     * for details.
     *
     * For example "%H:%M:%S" returns a 24-hour based time string like 20:12:01
     *
     * You can swith logging to stdout or a logfile on and off seperatly.
     *
     * Use the Parameter multithreading to activate a background logger thread. This way
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
    EALogger(logLevels minLvl = EALogger::logLevels::DEBUG,
             bool logToSTDOUT = true, bool logToFile = false,
             bool logToSyslog = false, bool multithreading = true,
             bool printThreadID = false, std::string dateTimeFormat = "%F %T",
             std::string logfile = "");
    ~EALogger();

    /**
     * @brief Issue a Logmessage
     * @param lvl The severity of the message, EALogger#logLevels
     * @param msg The message text
     */
    void writeLog(EALogger::logLevels lvl, std::string msg);

    /**
     * @brief Print a Stacktrace
     * @param size How many elements of the stack you wish to be printed.
     *
     * So far no demangeling is implemented yet. The method only works with gcc/llvm
     * compiled software.
     */
    void printStackTrace(uint size);

    // getters and setters for private members we want to expose

    /**
     * @brief Set the Date Time format specifier
     * @param s Format specifier string
     */
    void setDateTimeFormat(const std::string s);
    std::string getDateTimeFormat();

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
    /** Mutex used when not in multithreading mode */
    std::mutex mtx_log;
    std::mutex mtx_logToSTDOUT;
    std::mutex mtx_logToFile;
    std::mutex mtx_logToSyslog;
    std::mutex mtx_pThreadID;
    std::mutex mtx_dateTimeFormat;
    std::mutex mtx_backgroundLoggerStop;

    static bool receivedSIGUSR1;

    /** Minimum severity that is handled */
    EALogger::logLevels minLevel;

    bool logToSTDOUT;
    bool logToFile;
    bool logToSyslog;
    bool multithreading;
    bool printThreadID;

    std::string dateTimeFormat;
    std::string logfilePath;
    std::ofstream logFile;

    /** Threadsafe queue for multithreading mode */
    LogQueue logDataQueue;
    /** Background thread */
    std::thread backgroundLogger;
    /** Controls background logger thread */
    bool backgroundLoggerStop;

    /** lookup table for logelevel Strings */
    std::map<EALogger::logLevels, std::string> loglevelStringMap;
    /** map syslog message priority to our loglevels */
    std::map<EALogger::logLevels, int> loglevelSyslogMap;

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
