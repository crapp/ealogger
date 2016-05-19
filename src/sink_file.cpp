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

#include "ealogger/sink_file.h"

namespace eal = ealogger;
namespace con = ealogger::constants;

eal::SinkFile::SinkFile(std::string msg_template, std::string datetime_pattern,
                        bool enabled, con::LOG_LEVEL min_lvl,
                        std::string log_file)
    : eal::Sink(std::move(msg_template), std::move(datetime_pattern), enabled,
                min_lvl),
      log_file(log_file)
{
    if (this->get_enabled()) {
        this->open_file();
    }
}

eal::SinkFile::~SinkFile() { this->close_file(); }
void eal::SinkFile::set_log_file(std::string log_file)
{
    std::lock_guard<std::mutex> lock(this->mtx_log_file);
    this->log_file = std::move(log_file);
    // TODO: If this is the same filename?
    this->close_file();
    this->open_file();
}

void eal::SinkFile::write_message(const std::string &msg)
{
    std::lock_guard<std::mutex> lock(this->mtx_file_stream);
    try {
        if (this->file_stream.is_open()) {
            // TODO: The file stream is not flushed so when the application
            // crashes
            // it could be some information is lost.
            this->file_stream << msg << "\n";
        }
    } catch (const std::exception &ex) {
        // TODO: And now?
    }
}

void eal::SinkFile::config_changed()
{
    // we can access enabled directly here because this is called from
    // set_enabled and the coresponding mutex is already locked
    if (this->enabled && this->file_stream.is_open()) {
        this->close_file();
        return;
    }
    if (this->enabled && !this->file_stream.is_open()) {
        this->open_file();
        return;
    }
}

void eal::SinkFile::open_file()
{
    std::lock_guard<std::mutex> lock(this->mtx_file_stream);
    try {
        if (!this->file_stream.is_open()) {
            std::unique_lock<std::mutex> lock_log_file(this->mtx_log_file);
            this->file_stream.open(this->log_file,
                                   std::ios::out | std::ios::app);
            if (!this->file_stream)
                throw std::runtime_error("Can not open logfile: " +
                                         this->log_file);
            lock_log_file.unlock();

            // set exception mask for the file stream
            this->file_stream.exceptions(std::ifstream::badbit |
                                         std::ifstream::failbit);
        }
    } catch (const std::exception &ex) {
    }
}
void eal::SinkFile::close_file()
{
    std::lock_guard<std::mutex> lock(this->mtx_file_stream);
    if (this->file_stream.is_open()) {
        this->file_stream.flush();
        this->file_stream.close();
    }
}
