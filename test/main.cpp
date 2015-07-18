#include <memory>
#include <chrono>
#include <sstream>
#include <string>

#include "simplelogger.h"
#include "config.h"

int main(int argc, const char* argv[])
{
    std::unique_ptr<SimpleLogger> log = std::unique_ptr<SimpleLogger>(
        new SimpleLogger(SimpleLogger::logLevels::INFO, true, true, false, false,
                         true, "%H:%M:%S", "logToMe.log"));
    std::chrono::steady_clock::time_point t = std::chrono::steady_clock::now();

    std::stringstream simpleLoggerVersion;
    simpleLoggerVersion << VERSION_MAJOR << "." << VERSION_MINOR << "."
                        << VERSION_PATCH;

    log->writeLog(SimpleLogger::logLevels::INFO, "Logtester of SimpleLogger " +
                                                     simpleLoggerVersion.str() +
                                                     " is starting");
    log->writeLog(SimpleLogger::logLevels::INFO,
                  "Next message has lower severity than defined minimum");

    log->writeLog(SimpleLogger::logLevels::DEBUG, "Do you see me?");
    log->writeLog(SimpleLogger::logLevels::WARNING, "A warning message");
    log->writeLog(SimpleLogger::logLevels::ERROR, "An error message");
    log->writeLog(SimpleLogger::logLevels::FATAL, "A fatal message");

    log->setLogToSTDOUT(false);
    log->writeLog(SimpleLogger::logLevels::INFO,
                  "Logging to stdout deactivated");
    log->setLogToFile(false);
    log->writeLog(SimpleLogger::logLevels::INFO, "Logging to file deactivated");
    log->setLogToSTDOUT(true);
    log->writeLog(SimpleLogger::logLevels::INFO,
                  "Logging to stdout reactivated");
    log->setLogToFile(true);
    log->writeLog(SimpleLogger::logLevels::INFO, "Logging to file reactivated");

    log->writeLog(SimpleLogger::logLevels::INFO,
                  "Changing date time format specifiers to %A %r");
    log->setDateTimeFormat("%A %r");
    log->writeLog(SimpleLogger::logLevels::INFO, "Format specifiers changed");
    log->printStackTrace(10);

    int msNormal = std::chrono::duration_cast<std::chrono::microseconds>(
                       std::chrono::steady_clock::now() - t).count();
    log->writeLog(
        SimpleLogger::logLevels::INFO,
        "Logger needed " + std::to_string(msNormal) + "µs to do the logging");

    std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    // now we try the same in multithreading mode
    log = std::unique_ptr<SimpleLogger>(
        new SimpleLogger(SimpleLogger::logLevels::INFO, true, true, false, true,
                         true, "%H:%M:%S", "logToMe.log"));
    t = std::chrono::steady_clock::now();

    log->writeLog(SimpleLogger::logLevels::INFO, "Logtester is starting");
    log->writeLog(SimpleLogger::logLevels::INFO,
                  "Next message has lower severity than defined minimum");

    log->writeLog(SimpleLogger::logLevels::DEBUG, "Do you see me?");
    log->writeLog(SimpleLogger::logLevels::WARNING, "A warning message");
    log->writeLog(SimpleLogger::logLevels::ERROR, "An error message");
    log->writeLog(SimpleLogger::logLevels::FATAL, "A fatal message");

    log->setLogToSTDOUT(false);
    log->writeLog(SimpleLogger::logLevels::INFO,
                  "Logging to stdout deactivated");
    log->setLogToFile(false);
    log->writeLog(SimpleLogger::logLevels::INFO, "Logging to file deactivated");
    log->setLogToSTDOUT(true);
    log->writeLog(SimpleLogger::logLevels::INFO,
                  "Logging to stdout reactivated");
    log->setLogToFile(true);
    log->writeLog(SimpleLogger::logLevels::INFO, "Logging to file reactivated");

    log->writeLog(SimpleLogger::logLevels::INFO,
                  "Changing date time format specifiers to %A %r");
    log->setDateTimeFormat("%A %r");
    log->writeLog(SimpleLogger::logLevels::INFO, "Format specifiers changed");
    log->printStackTrace(10);

    int msMulti = std::chrono::duration_cast<std::chrono::microseconds>(
                      std::chrono::steady_clock::now() - t).count();
    log->writeLog(SimpleLogger::logLevels::INFO,
                  "Logger needed " + std::to_string(msMulti) +
                      "µs to do the logging in multithreading mode");

    log->writeLog(SimpleLogger::logLevels::INFO,
                  "The Tester will now write to the System Log. This only works "
                  "on Linux/BSD.");
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    log->setLogToSyslog(true);
    log->setLogToFile(false);
    log->setLogToSTDOUT(false);

    log->writeLog(SimpleLogger::logLevels::INFO, "Hello World Syslog");
    log->writeLog(SimpleLogger::logLevels::DEBUG, "This is a debug message");
    log->writeLog(SimpleLogger::logLevels::INFO, "This is an info message");
    log->writeLog(SimpleLogger::logLevels::WARNING, "This is a warning message");
    log->writeLog(SimpleLogger::logLevels::ERROR, "This is an error message");
    log->writeLog(SimpleLogger::logLevels::FATAL, "This is a fatal message");
    log->printStackTrace(10);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    log->setLogToFile(true);
    log->setLogToSTDOUT(true);
    log->setLogToSyslog(false);

    log->writeLog(SimpleLogger::logLevels::INFO,
                  "SimpleLog tester finished, bye bye");
    return 0;
}
