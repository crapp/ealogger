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

#ifndef SINK_SYSLOG_H
#define SINK_SYSLOG_H

/**
 * @file sink_syslog.h
 */

/**
 * @addtogroup SINK_GROUP
 * @{
 */

#ifdef EALOGGER_SYSLOG
#include <syslog.h>
#endif

#include "ealogger/sink.h"

namespace ealogger
{
/**
 * @brief Sink writing to unix syslog
 */
class SinkSyslog : public Sink
{
public:
    SinkSyslog(std::string msg_template, std::string datetime_pattern,
               bool enabled, ealogger::constants::LOG_LEVEL min_lvl);
    virtual ~SinkSyslog();

private:
    std::map<ealogger::constants::LOG_LEVEL, int>
        loglevel_syslog_lookup; /**< map syslog message severity to our loglevels */

    std::mutex mtx_syslog;

    void write_message(ATTR_UNUSED const std::string &msg);
    void config_changed();
};
}
/**
 * @}
 */

#endif /* SINK_SYSLOG_H */
