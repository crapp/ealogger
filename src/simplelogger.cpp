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

#include "simplelogger.h"

SimpleLogger::SimpleLogger(SimpleLogger::logLevels minLvl, bool logToSTDOUT,
                           bool logToFile, std::string dateTimeFormat,
                           std::string logfile) :
    minLevel(minLvl), logToSTDOUT(logToSTDOUT), logToFile(logToFile),
    dateTimeFormat(dateTimeFormat), logfilePath(logfile)
{
    //TODO: Make registration of signal handler configurable
    SimpleLogger::receivedSIGUSR1 = false;
    if (signal(SIGUSR1, SimpleLogger::logrotate) == SIG_ERR)
        throw std::runtime_error("Could not create signal handler for SIGUSR1");

    this->logFile.open(this->logfilePath, std::ios::out | std::ios::app);
    if (!this->logFile)
        throw std::runtime_error("Can not open logfile: " + this->logfilePath);
    //set exception mask for the file stream
    this->logFile.exceptions(std::ifstream::badbit | std::ifstream::failbit);
}

SimpleLogger::~SimpleLogger()
{
    if (this->logFile.is_open())
    {
        this->logFile.flush();
        this->logFile.close();
    }
}

void SimpleLogger::writeLog(SimpleLogger::logLevels lvl, std::string msg)
{
    if (SimpleLogger::receivedSIGUSR1)
    {
        SimpleLogger::receivedSIGUSR1 = false;
        this->logFile.flush();
        this->logFile.close();
        this->logFile.open(this->logfilePath, std::ios::out | std::ios::app);
        if (!this->logFile)
            throw std::runtime_error("Can not open logfile: " + this->logfilePath);
        //set exception mask for the file stream
        this->logFile.exceptions(std::ifstream::badbit | std::ifstream::failbit);
    }
    //lock mutex because std::cout is not threadsafe
    //TODO: Use ScopeLocks to lock mutexes as it is much saver.
    this->mtx.lock();
    if (lvl >= this->minLevel)
    {
        try 
        {
            switch (lvl)
            {
            case SimpleLogger::logLevels::DEBUG:
                if (this->logToSTDOUT)
                    std::cout << "[" << helpers::getFormattedTimeString(this->dateTimeFormat) <<
                            "] DEBUG: " << msg << std::endl;
                if (this->logToFile)
                    this->logFile << "[" << helpers::getFormattedTimeString(this->dateTimeFormat) <<
                            "] DEBUG: " << msg << std::endl;
                break;
            case SimpleLogger::logLevels::INFO:
                if (this->logToSTDOUT)
                    std::cout << "[" << helpers::getFormattedTimeString(this->dateTimeFormat) <<
                            "] INFO: " << msg << std::endl;
                if (this->logToFile)
                    this->logFile << "[" << helpers::getFormattedTimeString(this->dateTimeFormat) <<
                            "] INFO: " << msg << std::endl;
                break;
            case SimpleLogger::logLevels::WARNING:
                if (this->logToSTDOUT)
                    std::cout << "[" << helpers::getFormattedTimeString(this->dateTimeFormat) <<
                            "] WARNING: " << msg << std::endl;
                if (this->logToFile)
                    this->logFile << "[" << helpers::getFormattedTimeString(this->dateTimeFormat) <<
                            "] WARNING: " << msg << std::endl;
                break;
            case SimpleLogger::logLevels::ERROR:
                if (this->logToSTDOUT)
                    std::cout << "[" << helpers::getFormattedTimeString(this->dateTimeFormat) <<
                            "] ERROR: " << msg << std::endl;
                if (this->logToFile)
                    this->logFile << "[" << helpers::getFormattedTimeString(this->dateTimeFormat) <<
                            "] ERROR: " << msg << std::endl;
                break;
            case SimpleLogger::logLevels::FATAL:
                if (this->logToSTDOUT)
                    std::cout << "[" << helpers::getFormattedTimeString(this->dateTimeFormat) <<
                            "] FATAL: " << msg << std::endl;
                if (this->logToFile)
                    this->logFile << "[" << helpers::getFormattedTimeString(this->dateTimeFormat) <<
                            "] FATAL: " << msg << std::endl;
                break;
            default:
                break;
            }
        }
        catch (const std::ios_base::failure &fail)
        {
            std::cerr << "Can not write to Logfile stream" << std::endl;
            //std::cerr << "Error: " << fail << std::endl;
        }
    }
    this->mtx.unlock();
}

void SimpleLogger::logrotate(int signo)
{
    if (signo == SIGUSR1)
    {
        SimpleLogger::receivedSIGUSR1 = true;
    }
}
bool SimpleLogger::receivedSIGUSR1;
