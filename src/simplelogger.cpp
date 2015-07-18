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

#include "simplelogger.h"

SimpleLogger::SimpleLogger(SimpleLogger::logLevels minLvl, bool logToSTDOUT,
                           bool logToFile, bool logToSyslog, bool multithreading,
                           bool printThreadID, std::string dateTimeFormat,
                           std::string logfile)
    : minLevel(minLvl),
      logToSTDOUT(logToSTDOUT),
      logToFile(logToFile),
      logToSyslog(logToSyslog),
      multithreading(multithreading),
      printThreadID(printThreadID),
      dateTimeFormat(dateTimeFormat),
      logfilePath(logfile)
{
    // TODO: Make registration of signal handler configurable
    SimpleLogger::receivedSIGUSR1 = false;
#ifdef __linux__
    if (signal(SIGUSR1, SimpleLogger::logrotate) == SIG_ERR)
        throw std::runtime_error("Could not create signal handler for SIGUSR1");
#endif

    this->loglevelStringMap = {
        {SimpleLogger::logLevels::DEBUG, " DEBUG: "},
        {SimpleLogger::logLevels::INFO, " INFO: "},
        {SimpleLogger::logLevels::WARNING, " WARNING: "},
        {SimpleLogger::logLevels::ERROR, " ERROR: "},
        {SimpleLogger::logLevels::FATAL, " FATAL: "},
        {SimpleLogger::logLevels::INTERNAL, " INTERNAL: "}};
#ifdef SYSLOG
    this->loglevelSyslogMap = {{SimpleLogger::logLevels::DEBUG, LOG_DEBUG},
                               {SimpleLogger::logLevels::INFO, LOG_INFO},
                               {SimpleLogger::logLevels::WARNING, LOG_WARNING},
                               {SimpleLogger::logLevels::ERROR, LOG_ERR},
                               {SimpleLogger::logLevels::FATAL, LOG_CRIT},
                               {SimpleLogger::logLevels::INTERNAL,
                                LOG_DEBUG}};  // mapping internal to syslog debug
#elif
    this->loglevelSyslogMap = {};
    std::cout << "Map empty" << std::endl;
#endif

    if (this->getLogToFile()) {
        this->logFile.open(this->logfilePath, std::ios::out | std::ios::app);
        if (!this->logFile)
            throw std::runtime_error("Can not open logfile: " +
                                     this->logfilePath);
        // set exception mask for the file stream
        this->logFile.exceptions(std::ifstream::badbit | std::ifstream::failbit);
    }

    if (this->multithreading) {
        backgroundLoggerStop = false;
        backgroundLogger = std::thread(&SimpleLogger::logThreadFunc, this);
    }
}

SimpleLogger::~SimpleLogger()
{
    if (this->multithreading) {
        // wait for queue to be emptied. after 1 second we will exit the background logger thread
        int i = 0;
        while (!logDataQueue.empty()) {
            if (i == 100)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            i++;
        }
        this->setBackgroundLoggerStop(true);
        // we need to write one more log message to wakeup the background logger
        // thread it will pop the last message from the queue.
        this->writeLog(SimpleLogger::logLevels::INTERNAL, "Logger exit");
        try {
            backgroundLogger.join();
        } catch (const std::system_error &ex) {
            std::cerr << "Could not join with logger background thread: "
                      << ex.what() << std::endl;
        }
    }
    if (this->logFile && this->logFile.is_open()) {
        this->logFile.flush();
        this->logFile.close();
    }
}

void SimpleLogger::writeLog(SimpleLogger::logLevels lvl, std::string msg)
{
    std::shared_ptr<LogMessage> m;
    if (multithreading) {
        m = std::make_shared<LogMessage>(lvl, msg, LogMessage::LOGTYPE::DEFAULT);
        this->logDataQueue.push(m);
    } else {
        m = std::make_shared<LogMessage>(lvl, msg, LogMessage::LOGTYPE::DEFAULT);
        this->internalLogRoutine(m);
    }
}

void SimpleLogger::printStackTrace(uint size)
{
    void *addrlist[size + 1];

    size_t noOfStackAddresses =
        backtrace(addrlist, sizeof(addrlist) / sizeof(void *));
    char **tempSymbols = backtrace_symbols(addrlist, noOfStackAddresses);

    std::vector<std::string> symbollist(tempSymbols,
                                        tempSymbols + noOfStackAddresses);
    // tempSymbols has to be freed
    free(tempSymbols);
    std::shared_ptr<LogMessage> m = std::make_shared<LogMessage>(
        SimpleLogger::logLevels::DEBUG, symbollist, LogMessage::LOGTYPE::STACK);
    if (multithreading) {
        this->logDataQueue.push(m);
    } else {
        this->internalLogRoutine(m);
    }
}

void SimpleLogger::setDateTimeFormat(const std::string s)
{
    std::lock_guard<std::mutex> guard(this->mtx_dateTimeFormat);
    this->dateTimeFormat = s;
}

std::string SimpleLogger::getDateTimeFormat()
{
    std::lock_guard<std::mutex> guard(this->mtx_dateTimeFormat);
    return this->dateTimeFormat;
}

void SimpleLogger::setLogToSTDOUT(bool b)
{
    std::lock_guard<std::mutex> guard(this->mtx_logToSTDOUT);
    this->logToSTDOUT = b;
}

bool SimpleLogger::getLogToSTDOUT()
{
    std::lock_guard<std::mutex> guard(this->mtx_logToSTDOUT);
    return this->logToSTDOUT;
}

void SimpleLogger::setLogToFile(bool b)
{
    std::lock_guard<std::mutex> guard(this->mtx_logToFile);
    this->logToFile = b;
}

bool SimpleLogger::getLogToFile()
{
    std::lock_guard<std::mutex> guard(this->mtx_logToFile);
    return this->logToFile;
}

void SimpleLogger::setLogToSyslog(bool b)
{
    std::lock_guard<std::mutex> guard(this->mtx_logToSyslog);
    this->logToSyslog = b;
}

bool SimpleLogger::getLogToSyslog()
{
    std::lock_guard<std::mutex> guard(this->mtx_logToSyslog);
    return this->logToSyslog;
}

void SimpleLogger::setPrintThreadID(bool b)
{
    std::lock_guard<std::mutex> guard(this->mtx_pThreadID);
    this->printThreadID = b;
}

bool SimpleLogger::getPrintThreadID()
{
    std::lock_guard<std::mutex> guard(this->mtx_pThreadID);
    return this->printThreadID;
}

void SimpleLogger::logrotate(int signo)
{
#ifdef __linux__
    if (signo == SIGUSR1) {
        SimpleLogger::receivedSIGUSR1 = true;
    }
#endif
}

void SimpleLogger::logThreadFunc()
{
    while (!this->getBackgroundLoggerStop()) {
        std::shared_ptr<LogMessage> m = this->logDataQueue.pop();
        this->internalLogRoutine(m);
    }
}

void SimpleLogger::internalLogRoutine(std::shared_ptr<LogMessage> m)
{
    // lock mutex because iostreams or fstreams are not threadsafe
    if (!this->multithreading)
        std::lock_guard<std::mutex> lock(this->mtx_log);
    if (SimpleLogger::receivedSIGUSR1) {
        SimpleLogger::receivedSIGUSR1 = false;
        this->logFile.flush();
        this->logFile.close();
        this->logFile.open(this->logfilePath, std::ios::out | std::ios::app);
        if (!this->logFile)
            throw std::runtime_error("Can not open logfile: " +
                                     this->logfilePath);
        // set exception mask for the file stream
        this->logFile.exceptions(std::ifstream::badbit | std::ifstream::failbit);
    }
    SimpleLogger::logLevels msgLevel =
        static_cast<SimpleLogger::logLevels>(m->getSeverity());
    if (msgLevel >= this->minLevel ||
        m->getLogType() == LogMessage::LOGTYPE::STACK) {
        try {
            std::string tID = "";
            if (this->printThreadID) {
                std::stringstream ss;
                ss << std::this_thread::get_id();
                tID = " " + ss.str();
            }

            if (m->getLogType() == LogMessage::LOGTYPE::STACK) {
                std::stringstream stackLogMessage;
                stackLogMessage
                    << "["
                    << this->getFormattedTimeString(this->getDateTimeFormat())
                    << "]" << tID << " Stacktrace: " << std::endl;
                if (this->getLogToSTDOUT()) {
                    std::cout << stackLogMessage.str();
                }
                if (this->getLogToFile()) {
                    this->logFile << stackLogMessage.str();
                }
// FIXME: Using
#ifdef SYSLOG
                if (this->getLogToSyslog()) {
                    syslog(this->loglevelSyslogMap.at(msgLevel), "Stacktrace: ");
                }
#endif
                for (std::vector<std::string>::const_iterator it =
                         m->getStackElementsBegin();
                     it != m->getStackElementsEnd(); it++) {
                    if (this->getLogToSTDOUT())
                        std::cout << "\t" << *it << std::endl;
                    if (this->getLogToFile())
                        this->logFile << "\t" << *it << std::endl;
#ifdef SYSLOG
                    if (this->getLogToSyslog()) {
                        syslog(this->loglevelSyslogMap.at(msgLevel), "\t %s",
                               it->c_str());
#endif
                    }
                }
            } else {
#ifndef PRINT_INTERNAL_MESSAGES
                // Print INTERNAL messages only when defined
                if (msgLevel == SimpleLogger::logLevels::INTERNAL) {
                    return;
                }
#endif
                std::string msgLevelString =
                    this->loglevelStringMap.at(msgLevel);
                std::stringstream logStringStream;
                logStringStream
                    << "["
                    << this->getFormattedTimeString(this->getDateTimeFormat())
                    << "]" << tID << msgLevelString << m->getMessage()
                    << std::endl;
                if (this->getLogToSTDOUT())
                    std::cout << logStringStream.str();
                if (this->getLogToFile())
                    this->logFile << logStringStream.str();
#ifdef SYSLOG
                if (this->getLogToSyslog())
                    syslog(this->loglevelSyslogMap.at(msgLevel), "%s",
                           m->getMessage().c_str());
#endif
            }

        } catch (const std::ios_base::failure &fail) {
            std::cerr << "Can not write to Logfile stream" << std::endl;
            // std::cerr << "Error: " << fail << std::endl;
        }
    }
}

std::string SimpleLogger::getFormattedTimeString(const std::string &timeFormat)
{
    // get raw time
    time_t rawtime;
    // time struct
    struct tm *timeinfo;
    // buffer where we store the formatted time string
    char buffer[80];

    std::time(&rawtime);
    timeinfo = std::localtime(&rawtime);

    std::strftime(buffer, 80, timeFormat.c_str(), timeinfo);
    return (std::string(buffer));
}

std::string SimpleLogger::getFormattedTimeString(std::time_t t,
                                                 const std::string &timeFormat)
{
    // time struct
    struct tm *timeinfo;
    // buffer where we store the formatted time string
    char buffer[80];

    timeinfo = std::localtime(&t);

    std::strftime(buffer, 80, timeFormat.c_str(), timeinfo);
    return (std::string(buffer));
}

bool SimpleLogger::getBackgroundLoggerStop()
{
    std::lock_guard<std::mutex> guard(this->mtx_backgroundLoggerStop);
    return this->backgroundLoggerStop;
}

void SimpleLogger::setBackgroundLoggerStop(bool stop)
{
    std::lock_guard<std::mutex> guard(this->mtx_backgroundLoggerStop);
    this->backgroundLoggerStop = stop;
}

bool SimpleLogger::receivedSIGUSR1;
