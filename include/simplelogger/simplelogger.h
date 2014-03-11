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

#include "helpers.h"

class SimpleLogger
{
public:
    enum logLevels
    {
        DEBUG = 0,
        INFO = 1,
        WARNING = 2,
        ERROR = 3,
        FATAL = 4
    };

    SimpleLogger(logLevels minLvl, bool logToSTDOUT, bool logToFile,
                 std::string logfile);
    ~SimpleLogger();

    void writeLog(SimpleLogger::logLevels lvl, std::string msg);

private:
    std::mutex mtx;
    static bool receivedSIGUSR1;

    SimpleLogger::logLevels minLevel;

    bool logToSTDOUT;
    bool logToFile;

    std::string logfilePath;
    std::ofstream logFile;

    static void logrotate(int signo);
};

#endif // SIMPLELOGGER_H
