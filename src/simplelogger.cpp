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

SimpleLogger::SimpleLogger(SimpleLogger::logLevels minLvl,
                           bool logToSTDOUT,
                           bool logToFile,
                           bool multithreading,
                           std::string dateTimeFormat,
                           std::string logfile) :
    minLevel(minLvl), logToSTDOUT(logToSTDOUT),
    logToFile(logToFile),
    multithreading(multithreading),
    dateTimeFormat(dateTimeFormat),
    logfilePath(logfile)
{
    //TODO: Make registration of signal handler configurable
    SimpleLogger::receivedSIGUSR1 = false;
#ifdef __linux__
    if (signal(SIGUSR1, SimpleLogger::logrotate) == SIG_ERR)
        throw std::runtime_error("Could not create signal handler for SIGUSR1");
#endif

    this->logFile.open(this->logfilePath, std::ios::out | std::ios::app);
    if (!this->logFile)
        throw std::runtime_error("Can not open logfile: " + this->logfilePath);
    //set exception mask for the file stream
    this->logFile.exceptions(std::ifstream::badbit | std::ifstream::failbit);

    if(this->multithreading)
    {
        backgroundLoggerStop = false;
        backgroundLogger = std::thread(&SimpleLogger::logThreadFunc, this);
    }
}

SimpleLogger::~SimpleLogger()
{
    if(this->multithreading)
    {
        // wait for queue to be emptied. after 1 second we will exit the background logger thread
        int i = 0;
        while(!logDataQueue.empty())
        {
            if (i == 100)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            i++;
        }
        this->backgroundLoggerStop = true;
        this->writeLog(SimpleLogger::logLevels::INFO, "Logger exit");
        try {
            backgroundLogger.join();
        }
        catch(const std::system_error &ex)
        {
            std::cerr << "Could not join with logger background thread: " <<
                         ex.what() << std::endl;
        }
    }
    if (this->logFile.is_open())
    {
        this->logFile.flush();
        this->logFile.close();
    }
}

void SimpleLogger::writeLog(SimpleLogger::logLevels lvl, std::string msg)
{
    std::shared_ptr<LogMessage> m;
    if (multithreading)
    {
        m = std::make_shared<LogMessage>(lvl, msg, LogMessage::LOGTYPE::DEFAULT);
        this->logDataQueue.push(m);
    } else {
        m = std::make_shared<LogMessage>(lvl, msg, LogMessage::LOGTYPE::DEFAULT);
        this->internalLogRoutine(m);
    }
}

void SimpleLogger::printStackTrace(uint size)
{
    void *addrlist[size+1];

    size_t noOfStackAddresses = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));
    char **tempSymbols = backtrace_symbols(addrlist, noOfStackAddresses);

    std::vector<std::string> symbollist(tempSymbols, tempSymbols + noOfStackAddresses);
    // tempSymbols has to be freed
    free(tempSymbols);
    std::shared_ptr<LogMessage> m = std::make_shared<LogMessage>(
                SimpleLogger::logLevels::DEBUG,
                symbollist,
                LogMessage::LOGTYPE::STACK);
    this->logDataQueue.push(m);

}

void SimpleLogger::setDateTimeFormat(const std::string s)
{
    this->dateTimeFormat = s;
}

std::string SimpleLogger::getDateTimeFormat()
{
    return this->dateTimeFormat;
}

void SimpleLogger::setLogToSTDOUT(bool b)
{
    this->logToSTDOUT = b;
}

bool SimpleLogger::getLogToSTDOUT()
{
    return this->logToSTDOUT;
}

void SimpleLogger::setLogToFile(bool b)
{
    this->logToFile = b;
}

bool SimpleLogger::getLogToFile()
{
    return this->logToFile;
}

void SimpleLogger::logrotate(int signo)
{
#ifdef __linux__
    if (signo == SIGUSR1)
    {
        SimpleLogger::receivedSIGUSR1 = true;
    }
#endif
}

void SimpleLogger::logThreadFunc()
{
    //while(!this->backgroundLoggerStop && !logDataQueue.empty())
    while(!this->backgroundLoggerStop)
    {
        std::shared_ptr<LogMessage> m = this->logDataQueue.pop();
        this->internalLogRoutine(m);
    }
}

void SimpleLogger::internalLogRoutine(std::shared_ptr<LogMessage> m)
{
    //lock mutex because iostreams or fstreams are not threadsafe
    if(!this->multithreading)
        std::lock_guard<std::mutex> lock(this->mtx);
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
    if (static_cast<SimpleLogger::logLevels>(m->getSeverity()) >= this->minLevel ||
            m->getLogType() == LogMessage::LOGTYPE::STACK)
    {
        try
        {
            if (m->getLogType() == LogMessage::LOGTYPE::STACK)
            {
                if (this->logToSTDOUT)
                {
                    std::cout << "[" << helpers::getFormattedTimeString(this->dateTimeFormat) <<
                            "] Stacktrace: " << std::endl;
                }
                if (this->logToFile)
                {
                    this->logFile << "[" << helpers::getFormattedTimeString(this->dateTimeFormat) <<
                            "] Stacktrace: " << std::endl;
                }
                for (std::vector<std::string>::const_iterator it = m->getStackElementsBegin();
                     it != m->getStackElementsEnd(); it++)
                {
                    if (this->logToSTDOUT)
                        std::cout << "\t" << *it << std::endl;
                    if(this->logToFile)
                        this->logFile << "\t" << *it << std::endl;
                }
            } else {
                switch (m->getSeverity())
                {
                case SimpleLogger::logLevels::DEBUG:
                    if (this->logToSTDOUT)
                        std::cout << "[" << helpers::getFormattedTimeString(this->dateTimeFormat) <<
                                "] DEBUG: " << m->getMessage() << std::endl;
                    if (this->logToFile)
                        this->logFile << "[" << helpers::getFormattedTimeString(this->dateTimeFormat) <<
                                "] DEBUG: " << m->getMessage() << std::endl;
                    break;
                case SimpleLogger::logLevels::INFO:
                    if (this->logToSTDOUT)
                        std::cout << "[" << helpers::getFormattedTimeString(this->dateTimeFormat) <<
                                "] INFO: " << m->getMessage() << std::endl;
                    if (this->logToFile)
                        this->logFile << "[" << helpers::getFormattedTimeString(this->dateTimeFormat) <<
                                "] INFO: " << m->getMessage() << std::endl;
                    break;
                case SimpleLogger::logLevels::WARNING:
                    if (this->logToSTDOUT)
                        std::cout << "[" << helpers::getFormattedTimeString(this->dateTimeFormat) <<
                                "] WARNING: " << m->getMessage() << std::endl;
                    if (this->logToFile)
                        this->logFile << "[" << helpers::getFormattedTimeString(this->dateTimeFormat) <<
                                "] WARNING: " << m->getMessage() << std::endl;
                    break;
                case SimpleLogger::logLevels::ERROR:
                    if (this->logToSTDOUT)
                        std::cout << "[" << helpers::getFormattedTimeString(this->dateTimeFormat) <<
                                "] ERROR: " << m->getMessage() << std::endl;
                    if (this->logToFile)
                        this->logFile << "[" << helpers::getFormattedTimeString(this->dateTimeFormat) <<
                                "] ERROR: " << m->getMessage() << std::endl;
                    break;
                case SimpleLogger::logLevels::FATAL:
                    if (this->logToSTDOUT)
                        std::cout << "[" << helpers::getFormattedTimeString(this->dateTimeFormat) <<
                                "] FATAL: " << m->getMessage() << std::endl;
                    if (this->logToFile)
                        this->logFile << "[" << helpers::getFormattedTimeString(this->dateTimeFormat) <<
                                "] FATAL: " << m->getMessage() << std::endl;
                    break;
                default:
                    break;
                }
            }

        }
        catch (const std::ios_base::failure &fail)
        {
            std::cerr << "Can not write to Logfile stream" << std::endl;
            //std::cerr << "Error: " << fail << std::endl;
        }
    }
}
bool SimpleLogger::receivedSIGUSR1;
