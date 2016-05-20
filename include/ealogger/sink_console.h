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

#ifndef SINK_CONSOLE_H
#define SINK_CONSOLE_H

/** @file sink_console.h */

#include <iostream>

#include <ealogger/sink.h>

namespace ealogger
{
/**
 * @addtogroup SINK_GROUP
 * @{
 */
/**
 * @brief Sink to write to a console
 */
class SinkConsole : public Sink
{
public:
    /**
     * @brief Console sink constructor
     *
     * @param msg_template Message template based on conversion patterns
     * @param datetime_pattern Date and time conversion specifiers
     * @param enabled Whether or not this sink is enabled
     * @param min_lvl Minimum severity for this sink
     */
    SinkConsole(std::string msg_template, std::string datetime_pattern,
                bool enabled, ealogger::constants::LOG_LEVEL min_lvl);
    virtual ~SinkConsole();

private:
    std::mutex mtx_console;

    void write_message(const std::string &msg);
    void config_changed();
};
/** @} */
}

#endif /* SINK_CONSOLE_H */
