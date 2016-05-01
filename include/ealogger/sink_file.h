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

#ifndef SINK_FILE_H
#define SINK_FILE_H

#include <fstream>

#include "sink.h"

struct SinkConfigFile : public SinkConfig {
public:
    SinkConfigFile(std::string msg_pattern, std::string datetime_pattern,
                   bool enabled, con::LOG_LEVEL min_lvl, std::string log_file)
        : SinkConfig(std::move(msg_pattern), std::move(datetime_pattern),
                     enabled, min_lvl),
          log_file(std::move(log_file))
    {
    }
    void set_log_file(std::string log_file)
    {
        std::lock_guard<std::mutex> lock(this->mtx_log_file);
        this->log_file = std::move(log_file);
    }
    std::string get_log_file()
    {
        std::lock_guard<std::mutex> lock(this->mtx_log_file);
        return this->log_file;
    };

private:
    std::mutex mtx_log_file;
    std::string log_file;
};

class SinkFile : public Sink
{
public:
    SinkFile(std::shared_ptr<SinkConfigFile> config);
    virtual ~SinkFile();

private:
    std::mutex mtx_file_stream;
    std::ofstream file_stream;
    std::string current_filename;

    void write_message(const std::string &msg);
    void config_changed();
    void open_file();
    void close_file();
};

#endif /* SINK_FILE_H */
