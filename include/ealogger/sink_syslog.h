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

#include "sink.h"

/**
 * @brief Derived Config Class for the Syslog sink
 */
struct SinkConfigSyslog : public SinkConfig {
public:
    SinkConfigSyslog(std::string msg_pattern, std::string datetime_pattern,
                     bool enabled, con::LOG_LEVEL min_lvl)
        : SinkConfig(std::move(msg_pattern), std::move(datetime_pattern),
                     enabled, min_lvl){};
    virtual ~SinkConfigSyslog(){};
};

class SinkSyslog : public Sink
{
public:
    SinkSyslog(std::shared_ptr<SinkConfigSyslog> config);
    virtual ~SinkSyslog();

private:
    /** map syslog message priority to our loglevels */
    std::map<con::LOG_LEVEL, int> loglevel_syslog_lookup;

    std::mutex mtx_syslog;

    void write_message(const std::string &msg);
    void config_changed();
};

#endif /* SINK_SYSLOG_H */