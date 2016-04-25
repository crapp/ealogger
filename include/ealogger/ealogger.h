/*  This is a simple yet powerful logger library for c++
    Copyright (C) 2013 - 2015 Christian Rapp

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SIMPLELOGGER_H
#define SIMPLELOGGER_H

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
 * @brief The heart of Simple Logger
 * @author Christian Rapp (crapp)
 *
 * The SimpleLogger class provides all the functionality you need for your application
 * to log messages to stdout and in a logfile. The logger uses a mutex for mutual
 * exclusion. This way thread safety is guaranteed (as long as you only use one
 * instance of this class)
 */
class SimpleLogger
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
     * @brief SimpleLogger constructor
     * @param minLvl Minim loglevel, all messages with a lower severity will be discarded
     * @param logToSTDOUT Boolean to indicate whether or not to write to stdout. Can be changed on runtime
     * @param logToFile Boolean to indicate whether or not to write to a logfile. Can be changed on runtime
     * @param logToSyslog Boolean to indicate whether or not to use syslog (for example systemd) on Linux/BSD
     * @param multithreading Boolean if activated simplelogger uses a background thread to write messages to streams
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
    SimpleLogger(logLevels minLvl = SimpleLogger::logLevels::DEBUG,
                 bool logToSTDOUT = true, bool logToFile = false,
                 bool logToSyslog = false, bool multithreading = true,
                 bool printThreadID = false,
                 std::string dateTimeFormat = "%F %T", std::string logfile = "");
    ~SimpleLogger();

    /**
     * @brief Issue a Logmessage
     * @param lvl The severity of the message, SimpleLogger#logLevels
     * @param msg The message text
     */
    void writeLog(SimpleLogger::logLevels lvl, std::string msg);

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
    SimpleLogger::logLevels minLevel;

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
    std::map<SimpleLogger::logLevels, std::string> loglevelStringMap;
    /** map syslog message priority to our loglevels */
    std::map<SimpleLogger::logLevels, int> loglevelSyslogMap;

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

#endif  // SIMPLELOGGER_H
