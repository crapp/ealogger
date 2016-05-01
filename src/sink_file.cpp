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

#include "sink_file.h"

SinkFile::SinkFile(std::shared_ptr<SinkConfigFile> config)
    : Sink(std::move(config))
{
    this->current_filename =
        std::dynamic_pointer_cast<SinkConfigFile>(this->config)->get_log_file();
    if (this->config->get_enabled()) {
        this->open_file();
    }
}

SinkFile::~SinkFile() { this->close_file(); }
void SinkFile::write_message(const std::string &msg)
{
    std::lock_guard<std::mutex> lock(this->mtx_file_stream);
    if (this->file_stream.is_open()) {
        this->file_stream << msg;
    }
}

void SinkFile::config_changed()
{
    if (!this->config->get_enabled() && this->file_stream.is_open()) {
        this->close_file();
        return;
    }
    if (this->config->get_enabled() && !this->file_stream.is_open()) {
        this->open_file();
        return;
    }
    if (std::dynamic_pointer_cast<SinkConfigFile>(this->config)
            ->get_log_file() != this->current_filename) {
        this->close_file();
        this->open_file();
    }
}

void SinkFile::open_file()
{
    std::lock_guard<std::mutex> lock(this->mtx_file_stream);
    if (!this->file_stream.is_open()) {
        this->file_stream.open(this->current_filename,
                               std::ios::out | std::ios::app);
        if (!this->file_stream)
            throw std::runtime_error("Can not open logfile: " +
                                     this->current_filename);

        // set exception mask for the file stream
        this->file_stream.exceptions(std::ifstream::badbit |
                                     std::ifstream::failbit);
    }
}

void SinkFile::close_file()
{
    std::lock_guard<std::mutex> lock(this->mtx_file_stream);
    if (this->file_stream.is_open()) {
        this->file_stream.flush();
        this->file_stream.close();
    }
}
