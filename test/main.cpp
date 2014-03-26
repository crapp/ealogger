#include <memory>
#include <chrono>

#include "simplelogger.h"

int main(int argc, const char* argv[])
{
    std::unique_ptr<SimpleLogger> log = std::unique_ptr<SimpleLogger>(
                new SimpleLogger(SimpleLogger::logLevels::INFO,
                                 true,
                                 true,
                                 false,
                                 "%H:%M:%S",
                                 "logToMe.log"));
    std::chrono::steady_clock::time_point t = std::chrono::steady_clock::now();

    log->writeLog(SimpleLogger::logLevels::INFO, "Logtester is starting");
    log->writeLog(SimpleLogger::logLevels::INFO,
                 "Next message has lower severity than defined minimum");

    log->writeLog(SimpleLogger::logLevels::DEBUG,
                 "Do you see me?");
    log->writeLog(SimpleLogger::logLevels::WARNING,
                 "A warning message");
    log->writeLog(SimpleLogger::logLevels::ERROR,
                 "An error message");
    log->writeLog(SimpleLogger::logLevels::FATAL,
                 "A fatal message");

    log->setLogToSTDOUT(false);
    log->writeLog(SimpleLogger::logLevels::INFO,
                 "Logging to stdout deactivated");
    log->setLogToFile(false);
    log->writeLog(SimpleLogger::logLevels::INFO,
                 "Logging to file deactivated");
    log->setLogToSTDOUT(true);
    log->writeLog(SimpleLogger::logLevels::INFO,
                 "Logging to stdout reactivated");
    log->setLogToFile(true);
    log->writeLog(SimpleLogger::logLevels::INFO,
                 "Logging to file reactivated");

    log->writeLog(SimpleLogger::logLevels::INFO,
                 "Changing date time format specifiers to %A %r");
    log->setDateTimeFormat("%A %r");
    log->writeLog(SimpleLogger::logLevels::INFO,
                 "Format specifiers changed");
    log->printStackTrace(10);

    int msNormal = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - t).count();
    log->writeLog(SimpleLogger::logLevels::INFO,
                 "Logger needed " + std::to_string(msNormal) + "µs to do the logging");
    //now we try the same in multithreading mode
    log = std::unique_ptr<SimpleLogger>(
                new SimpleLogger(SimpleLogger::logLevels::INFO,
                                 true,
                                 true,
                                 true,
                                 "%H:%M:%S",
                                 "logToMe.log"));
    t = std::chrono::steady_clock::now();

    log->writeLog(SimpleLogger::logLevels::INFO, "Logtester is starting");
    log->writeLog(SimpleLogger::logLevels::INFO,
                 "Next message has lower severity than defined minimum");

    log->writeLog(SimpleLogger::logLevels::DEBUG,
                 "Do you see me?");
    log->writeLog(SimpleLogger::logLevels::WARNING,
                 "A warning message");
    log->writeLog(SimpleLogger::logLevels::ERROR,
                 "An error message");
    log->writeLog(SimpleLogger::logLevels::FATAL,
                 "A fatal message");

    log->setLogToSTDOUT(false);
    log->writeLog(SimpleLogger::logLevels::INFO,
                 "Logging to stdout deactivated");
    log->setLogToFile(false);
    log->writeLog(SimpleLogger::logLevels::INFO,
                 "Logging to file deactivated");
    log->setLogToSTDOUT(true);
    log->writeLog(SimpleLogger::logLevels::INFO,
                 "Logging to stdout reactivated");
    log->setLogToFile(true);
    log->writeLog(SimpleLogger::logLevels::INFO,
                 "Logging to file reactivated");

    log->writeLog(SimpleLogger::logLevels::INFO,
                 "Changing date time format specifiers to %A %r");
    log->setDateTimeFormat("%A %r");
    log->writeLog(SimpleLogger::logLevels::INFO,
                 "Format specifiers changed");
    log->printStackTrace(10);

    int msMulti = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now()- t).count();
    log->writeLog(SimpleLogger::logLevels::INFO,
                 "Logger needed " + std::to_string(msMulti) +
                  "µs to do the logging in multithreading mode");

    log->writeLog(SimpleLogger::logLevels::INFO,
                 "SimpleLog tester finished, bye bye");
    return 0;
}
