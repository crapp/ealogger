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

/** @file sink_file.h */

/**
 * @addtogroup SINK_GROUP
 * @{
 */

#include <fstream>

#include "ealogger/sink.h"

/**
 * @brief Sink to write to a log file
 */
class SinkFile : public Sink
{
public:
    /**
     * @brief SinkFile constructor with additional parameters
     *
     * @param msg_template String with conversion specifiers
     * @param datetime_pattern Conversion specifiers for date time
     * @param enabled Whether or not this sink is enabled
     * @param min_lvl Minimum severity
     * @param log_file Log file
     * @details
     * Make sure you have write permissions for the log file and the corresponding
     * directories exist.
     */
    SinkFile(std::string msg_template, std::string datetime_pattern,
             bool enabled, ealogger_constants::LOG_LEVEL min_lvl,
             std::string log_file);
    virtual ~SinkFile();

    /**
     * @brief Set log file
     *
     * @param log_file
     * @warning
     * This is method is not exposed by the API currently. You may have the
     * possibility to work with Sink objects directly in the future. If you need
     * to change the logfile use EALogger::init_file_sink to reinit the SinkFile
     */
    void set_log_file(std::string log_file);

private:
    std::mutex mtx_file_stream;
    std::mutex mtx_log_file;

    std::ofstream file_stream;
    std::string log_file;

    void write_message(const std::string &msg);
    /**
     * @brief Called when Sink::set_enabled was called
     * @details
     * Opens or closes logfile according to Sink#enabled
     */
    void config_changed();
    /**
     * @brief Open logfile
     */
    void open_file();
    /**
     * @brief Close logfile
     */
    void close_file();
};

/** @} */

#endif /* SINK_FILE_H */
