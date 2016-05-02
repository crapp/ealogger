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

#include <memory>
#include <chrono>
#include <sstream>
#include <string>

#include "ealogger/ealogger.h"
#include "config.h"

int main(int argc, const char* argv[])
{
    std::unique_ptr<EALogger> log =
        std::unique_ptr<EALogger>(new EALogger(true));
    log->init_console_sink();
    log->set_enabled(con::LOGGER_SINK::CONSOLES, false);
    log->init_file_sink();

    std::stringstream version;
    version << EALOGGER_VERSION_MAJOR << "." << EALOGGER_VERSION_MINOR;
    if (std::string(EALOGGER_VERSION_PATCH) != "0") {
        version << "." << EALOGGER_VERSION_PATCH;
    }

    // log->debug("A debug message");
    // log->info("Info was here");
    // log->warn("Warning");
    // log->error("Error");
    // log->fatal("Alert, system in fatal state");
    // log->stack();

    // log->set_min_lvl(con::LOGGER_SINK::CONSOLES, con::LOG_LEVEL::WARNING);

    // log->info("Info was here 2");

    std::chrono::system_clock::time_point t = std::chrono::system_clock::now();
    for (int i = 0; i < 1000000; i++) {
        log->info("Hello Africa, tell me how you are doing " +
                  std::to_string(i));
    }
    std::chrono::system_clock::time_point tstop =
        std::chrono::system_clock::now();

    while (!log->queue_empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "Time messages on queue: "
              << std::chrono::duration_cast<std::chrono::microseconds>(tstop - t)
                         .count() /
                     1000
              << "ms" << std::endl;
    std::cout << "Time till end: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::system_clock::now() - t)
                     .count()
              << "ms" << std::endl;
    // std::chrono::system_clock::time_point t =
    // std::chrono::system_clock::now();
    //log->info("Logtester of ealogger " + version.str() + " is starting");
    // log->info("Next message has lower severity than defined minimum");

    // log->debug("Do you see me?");
    // log->warn("A warning message");
    // log->error("An error message");
    // log->fatal("A fatal message");

    // log->set_log_to_console(false);
    // log->info("Logging to console deactivated");
    // log->set_log_to_file(false);
    // log->info("Logging to file deactivated");
    // log->set_log_to_console(true);
    // log->info("Logging to console reactivated");
    // log->set_log_to_file(true);
    // log->info("Logging to file reactivated");

    // log->info("Changing date time format specifiers to %A %r");
    // log->set_dt_format("%A %r");
    // log->info("Format specifiers changed");
    // log->stack_trace(10);

    // int msNormal = std::chrono::duration_cast<std::chrono::microseconds>(
    // std::chrono::system_clock::now() - t)
    //.count();
    // log->info("Application needed " + std::to_string(msNormal) +
    //"µs to do the logging");

    // std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    //// now we try the same in async mode
    // log = std::unique_ptr<EALogger>(new EALogger(con::LOG_LEVEL::INFO, true,
    // true, false, true, "%H:%M:%S",
    //"logToMe.log"));
    // t = std::chrono::system_clock::now();

    //log->info("Logtester of ealogger " + version.str() + " is starting");
    // log->info("Next message has lower severity than defined minimum");

    // log->debug("Do you see me?");
    // log->warn("A warning message");
    // log->error("An error message");
    // log->fatal("A fatal message");

    // log->set_log_to_console(false);
    // log->info("Logging to console deactivated");
    // log->set_log_to_file(false);
    // log->info("Logging to file deactivated");
    // log->set_log_to_console(true);
    // log->info("Logging to console reactivated");
    // log->set_log_to_file(true);
    // log->info("Logging to file reactivated");

    // log->info("Changing date time format specifiers to %A %r");
    // log->set_dt_format("%A %r");
    // log->info("Format specifiers changed");
    // log->stack_trace(10);

    // int msMulti = std::chrono::duration_cast<std::chrono::microseconds>(
    // std::chrono::system_clock::now() - t)
    //.count();
    // log->info("Application needed " + std::to_string(msMulti) +
    //"µs to do the logging in async mode");

    // log->info(
    //"The Tester will now write to the System Log. This only works "
    //"on Linux/BSD.");

    // std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    // log->set_log_to_syslog(true);
    // log->set_log_to_file(false);
    // log->set_log_to_console(false);

    // log->info("Hello World Syslog");
    // log->debug("This is a debug message");
    // log->info("This is an info message");
    // log->warn("This is a warning message");
    // log->error("This is an error message");
    // log->fatal("This is a fatal message");
    // log->stack_trace(10);

    // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // log->set_log_to_file(true);
    // log->set_log_to_console(true);
    // log->set_log_to_syslog(false);

    // log->info("ealogger tester finished, bye bye");
    return 0;
}
