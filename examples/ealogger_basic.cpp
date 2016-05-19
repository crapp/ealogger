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

#include <chrono>
#include <memory>
#include <sstream>
#include <string>

#include <ealogger/ealogger.h>

int main(void)
{
    namespace eal = ealogger;
    namespace con = ealogger::constants;

    std::unique_ptr<eal::Logger> log =
        std::unique_ptr<eal::Logger>(new eal::Logger(true));
    log->init_console_sink();

    std::stringstream version;
    version << EALOGGER_VERSION_MAJOR << "." << EALOGGER_VERSION_MINOR;
    if (std::string(EALOGGER_VERSION_PATCH) != "0") {
        version << "." << EALOGGER_VERSION_PATCH;
    }

    log->info("This is an ealogger " + version.str() + " example application");
    log->debug("A debug message");
    log->info("Info was here");
    log->warn("Warning");
    log->error("Error");
    log->fatal("Alert, system in fatal state");

    // As we are in async mode you might want to check whether the internal log
    // messages queue is empty.
    while (!log->queue_empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // change minimum severity for the console sink to warning
    log->set_min_lvl(con::LOGGER_SINK::CONSOLES, con::LOG_LEVEL::WARNING);

    // this message will not appear in the console
    log->info("Info is not visible because minimum severity is WARNING");

    // wait again
    while (!log->queue_empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // set severity to info
    log->set_min_lvl(con::LOGGER_SINK::CONSOLES, con::LOG_LEVEL::INFO);
    // the next message will be visible again
    log->info("This message should be visible in the console");

    // change the datetime conversion pattern
    log->set_datetime_pattern(con::LOGGER_SINK::CONSOLES, "%A %r");
    log->info("You should now see the new datetime conversion pattern");

    // change the message template for the console sink
    // datetime [file name:"line number" "function name"] severity: log message
    log->set_msg_template(con::LOGGER_SINK::CONSOLES, "%d [%f:%l %u] %s: %m");
    log->info("You should now see a new message template in use");
    log->info("The change will affect all messages currently in the queue");

    return 0;
}
