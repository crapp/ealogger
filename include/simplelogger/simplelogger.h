/*  This is a very simple logger for c++
    Copyright (C) 2013, 2014 Christian Rapp

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
#include <mutex>
#include <iostream>
#include <fstream>
#include <csignal>
#include <stdexcept>
#include <queue>

#include "helpers.h"

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
    enum logLevels
    {
        DEBUG = 0,
        INFO = 1,
        WARNING = 2,
        ERROR = 3,
        FATAL = 4
    };

    typedef logmessage std::pair<SimpleLogger::logLevels, std::string>;


    /**
     * @brief SimpleLogger constructor
     * @param minLvl Minim loglevel, all messages with a lower severity will be discarded
     * @param logToSTDOUT Boolean to indicate whether or not to write to stdout. Can be changed on runtime
     * @param logToFile Boolean to indicate whether or not to write to a logfile. Can be changed on runtime
     * @param dateTimeFormat The Date Time format specifiers.
     * @param logfile The logfile to use
     *
     * Internally std::strftime is used from header ctime to create time string.
     * So for parameter dateTimeFormat you have to use format specifiers that are
     * recognised by strftime. See [en.cppreference.com](http://en.cppreference.com/w/cpp/chrono/c/strftime)
     * for details.
     *
     * For example "%H:%M:%S" returns a 24-hour based time string like 20:12:01
     *
     * The logfile parameter must contain the path and the filename of the logfile.
     * Make sure your application ahs write permissions at the specified location.
     *
     * The constructor automatically registers a signal handler for SIGUSR1. This
     * allows logrotation with logrotate
     */
    SimpleLogger(logLevels minLvl, bool logToSTDOUT, bool logToFile,
                 std::string dateTimeFormat,
                 std::string logfile);
    ~SimpleLogger();

    /**
     * @brief Issue a Logmessage
     * @param lvl The severity of the message
     * @param msg The message text
     */
    void writeLog(SimpleLogger::logLevels lvl, std::string msg);

    //getters and setters for private members we want to expose

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

private:
    std::mutex mtx;
    static bool receivedSIGUSR1;

    SimpleLogger::logLevels minLevel;

    bool logToSTDOUT;
    bool logToFile;

    std::string dateTimeFormat;
    std::string logfilePath;
    std::ofstream logFile;

    std::queue<std::pair<SimpleLogger::logLevels, std::string> messageQueue;

    static void logrotate(int signo);
};

#endif // SIMPLELOGGER_H
