#include <memory>

#include "simplelogger.h"

int main(int argc, const char* argv[])
{
    std::unique_ptr<SimpleLogger> log = std::unique_ptr<SimpleLogger>(
                new SimpleLogger(SimpleLogger::logLevels::INFO,
                                 true,
                                 true,
                                 "%H:%M:%S",
                                 "logToMe.log"));

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
    log->writeLog(SimpleLogger::logLevels::INFO,
                 "Releasing Logtester unique pointer");
    //release the unique pointer
    log.release();
    try
    {
        //this should fail if your application does not have write permissions
        log = std::unique_ptr<SimpleLogger>(
                        new SimpleLogger(SimpleLogger::logLevels::DEBUG,
                                         true,
                                         true,
                                         "%H:%M:%S",
                                         "/var/log/logtester.log"));
    }
    catch(const std::runtime_error &ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
