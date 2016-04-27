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

#include "ealogger.h"

EALogger::EALogger(EALogger::logLevels minLvl, bool logToSTDOUT, bool logToFile,
                   bool logToSyslog, bool multithreading, bool printThreadID,
                   std::string dateTimeFormat, std::string logfile)
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
    EALogger::receivedSIGUSR1 = false;
#ifdef __linux__
    if (signal(SIGUSR1, EALogger::logrotate) == SIG_ERR)
        throw std::runtime_error("Could not create signal handler for SIGUSR1");
#endif

    this->loglevelStringMap = {{EALogger::logLevels::DEBUG, " DEBUG: "},
                               {EALogger::logLevels::INFO, " INFO: "},
                               {EALogger::logLevels::WARNING, " WARNING: "},
                               {EALogger::logLevels::ERROR, " ERROR: "},
                               {EALogger::logLevels::FATAL, " FATAL: "},
                               {EALogger::logLevels::INTERNAL, " INTERNAL: "}};
#ifdef SYSLOG
    this->loglevelSyslogMap = {{EALogger::logLevels::DEBUG, LOG_DEBUG},
                               {EALogger::logLevels::INFO, LOG_INFO},
                               {EALogger::logLevels::WARNING, LOG_WARNING},
                               {EALogger::logLevels::ERROR, LOG_ERR},
                               {EALogger::logLevels::FATAL, LOG_CRIT},
                               {EALogger::logLevels::INTERNAL,
                                LOG_DEBUG}};  // mapping internal to syslog debug
#else
    this->loglevelSyslogMap = {};
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
        backgroundLogger = std::thread(&EALogger::logThreadFunc, this);
    }
}

EALogger::~EALogger()
{
    if (this->multithreading) {
        // wait for queue to be emptied. after 1 second we will exit the background logger thread
        int i = 0;
        // TODO: Make this wait for queue to be empty optional
        while (!logDataQueue.empty()) {
            if (i == 100)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            i++;
        }
        this->setBackgroundLoggerStop(true);
        // we need to write one more log message to wakeup the background logger
        // thread it will pop the last message from the queue.
        this->write_log(EALogger::logLevels::INTERNAL, "Logger exit");
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

void EALogger::write_log(EALogger::logLevels lvl, std::string msg)
{
    std::shared_ptr<LogMessage> m;
    if (multithreading) {
        m = std::make_shared<LogMessage>(lvl, std::move(msg),
                                         LogMessage::LOGTYPE::DEFAULT);
        this->logDataQueue.push(m);
    } else {
        m = std::make_shared<LogMessage>(lvl, std::move(msg),
                                         LogMessage::LOGTYPE::DEFAULT);
        this->internalLogRoutine(m);
    }
}

void EALogger::debug(std::string msg)
{
    this->write_log(EALogger::logLevels::DEBUG, std::move(msg));
}

void EALogger::info(std::string msg)
{
    this->write_log(EALogger::logLevels::INFO, std::move(msg));
}

void EALogger::warn(std::string msg)
{
    this->write_log(EALogger::logLevels::WARNING, std::move(msg));
}

void EALogger::error(std::string msg)
{
    this->write_log(EALogger::logLevels::ERROR, std::move(msg));
}

void EALogger::fatal(std::string msg)
{
    this->write_log(EALogger::logLevels::FATAL, std::move(msg));
}

void EALogger::stack_trace(unsigned int size)
{
#ifdef __GLIBC__
    void *addrlist[size + 1];

    size_t noOfStackAddresses =
        backtrace(addrlist, sizeof(addrlist) / sizeof(void *));
    char **tempSymbols = backtrace_symbols(addrlist, noOfStackAddresses);

    // initialize a vector of string with the char**
    std::vector<std::string> symbollist(tempSymbols,
                                        tempSymbols + noOfStackAddresses);
    std::vector<std::string> stackvec;
    // tempSymbols has to be freed
    free(tempSymbols);

    //std::regex rgx("\\(.*\\+|\\+0[xX][0-9a-fA-F]+\\)");
    std::regex rgx("(^.*\\()(.*\\+)(0[xX][0-9a-fA-F]+\\))");

    // TODO: This always prints printStacktrace as first symbol. Should we omit
    // this?
    for (const auto &symbol : symbollist) {
        std::smatch match;
        if (!std::regex_search(symbol, match, rgx)) {
            stackvec.push_back(symbol);
            continue;
        }
        // get teh regex matches and create the 3 strings we need
        std::string file = match[1].str();
        file = file.substr(0, file.size() - 1);
        std::string mangled_name = match[2].str();
        mangled_name = mangled_name.substr(0, mangled_name.size() - 1);
        std::string caller = match[3].str();
        caller = caller.substr(0, caller.size() - 1);

        // demangle status
        int status = 0;
        // must be freed
        char *realname =
            abi::__cxa_demangle(mangled_name.c_str(), 0, 0, &status);
        if (status == 0) {
            stackvec.emplace_back(file + " : " + std::string(realname) + "+" +
                                  caller);
        } else {
            stackvec.emplace_back(file + " : " + mangled_name + "()" + "+" +
                                  caller);
        }
        free(realname);
    }
    std::shared_ptr<LogMessage> m = std::make_shared<LogMessage>(
        EALogger::logLevels::DEBUG, stackvec, LogMessage::LOGTYPE::STACK);
    if (multithreading) {
        this->logDataQueue.push(m);
    } else {
        this->internalLogRoutine(m);
    }
#endif
}

void EALogger::setDateTimeFormat(std::string fmt)
{
    std::lock_guard<std::mutex> guard(this->mtx_dateTimeFormat);
    this->dateTimeFormat = fmt;
}

std::string EALogger::getDateTimeFormat()
{
    std::lock_guard<std::mutex> guard(this->mtx_dateTimeFormat);
    return this->dateTimeFormat;
}

void EALogger::setLogToSTDOUT(bool b)
{
    std::lock_guard<std::mutex> guard(this->mtx_logToSTDOUT);
    this->logToSTDOUT = b;
}

bool EALogger::getLogToSTDOUT()
{
    std::lock_guard<std::mutex> guard(this->mtx_logToSTDOUT);
    return this->logToSTDOUT;
}

void EALogger::setLogToFile(bool b)
{
    std::lock_guard<std::mutex> guard(this->mtx_logToFile);
    this->logToFile = b;
}

bool EALogger::getLogToFile()
{
    std::lock_guard<std::mutex> guard(this->mtx_logToFile);
    return this->logToFile;
}

void EALogger::setLogToSyslog(bool b)
{
    std::lock_guard<std::mutex> guard(this->mtx_logToSyslog);
    this->logToSyslog = b;
}

bool EALogger::getLogToSyslog()
{
    std::lock_guard<std::mutex> guard(this->mtx_logToSyslog);
    return this->logToSyslog;
}

void EALogger::setPrintThreadID(bool b)
{
    std::lock_guard<std::mutex> guard(this->mtx_pThreadID);
    this->printThreadID = b;
}

bool EALogger::getPrintThreadID()
{
    std::lock_guard<std::mutex> guard(this->mtx_pThreadID);
    return this->printThreadID;
}

void EALogger::logrotate(int signo)
{
#ifdef __linux__
    if (signo == SIGUSR1) {
        EALogger::receivedSIGUSR1 = true;
    }
#endif
}

void EALogger::logThreadFunc()
{
    while (!this->getBackgroundLoggerStop()) {
        std::shared_ptr<LogMessage> m = this->logDataQueue.pop();
        this->internalLogRoutine(m);
    }
}

void EALogger::internalLogRoutine(std::shared_ptr<LogMessage> m)
{
    // lock mutex because iostreams or fstreams are not threadsafe
    if (!this->multithreading)
        std::lock_guard<std::mutex> lock(this->mtx_log);
    if (EALogger::receivedSIGUSR1) {
        EALogger::receivedSIGUSR1 = false;
        this->logFile.flush();
        this->logFile.close();
        this->logFile.open(this->logfilePath, std::ios::out | std::ios::app);
        if (!this->logFile)
            throw std::runtime_error("Can not open logfile: " +
                                     this->logfilePath);
        // set exception mask for the file stream
        this->logFile.exceptions(std::ifstream::badbit | std::ifstream::failbit);
    }
    EALogger::logLevels msgLevel =
        static_cast<EALogger::logLevels>(m->getSeverity());
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
#ifdef SYSLOG
                if (this->getLogToSyslog()) {
                    syslog(this->loglevelSyslogMap.at(msgLevel), "Stacktrace: ");
                }
#endif
                // TODO: Old style vector iteration. c++11 range based for loop
                // would be nice
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
                    }
#endif
                }
            } else {
#ifndef PRINT_INTERNAL_MESSAGES
                // Print INTERNAL messages only when defined
                if (msgLevel == EALogger::logLevels::INTERNAL) {
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

std::string EALogger::getFormattedTimeString(const std::string &timeFormat)
{
    // get raw time
    time_t rawtime;
    // time struct
    struct tm *timeinfo;
    // buffer where we store the formatted time string
    char buffer[80];

    std::time(&rawtime);
    timeinfo = std::localtime(&rawtime);
    //    localtime_r

    std::strftime(buffer, 80, timeFormat.c_str(), timeinfo);
    return (std::string(buffer));
}

std::string EALogger::getFormattedTimeString(std::time_t t,
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

bool EALogger::getBackgroundLoggerStop()
{
    std::lock_guard<std::mutex> guard(this->mtx_backgroundLoggerStop);
    return this->backgroundLoggerStop;
}

void EALogger::setBackgroundLoggerStop(bool stop)
{
    std::lock_guard<std::mutex> guard(this->mtx_backgroundLoggerStop);
    this->backgroundLoggerStop = stop;
}

bool EALogger::receivedSIGUSR1;
