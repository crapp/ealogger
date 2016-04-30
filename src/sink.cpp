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

#include "sink.h"

Sink::Sink(std::string msg_pattern, std::string datetime_pattern, bool enabled)
    : msg_pattern(std::move(msg_pattern)),
      datetime_pattern(std::move(datetime_pattern)),
      enabled(enabled)
{
    this->fill_conv_patterns();
}
Sink::~Sink() {}
void Sink::prepare_log_message(const std::shared_ptr<LogMessage> &log_message)
{
    std::lock_guard<std::mutex>(this->mtx_msg_pattern);
    std::lock_guard<std::mutex>(this->mtx_datetime_pattern);
    std::string msg = this->msg_pattern;
    for (const auto &cp : this->vec_conv_patterns) {
        switch (cp.get_pattern_type()) {
        case ConversionPattern::PATTERN_TYPE::DT:
            cp.replace_conversion_pattern(msg, "");
            break;
        case ConversionPattern::PATTERN_TYPE::FILE:
            cp.replace_conversion_pattern(
                msg, utility::get_file_name(log_message->get_call_file()));
            break;
        case ConversionPattern::PATTERN_TYPE::FILE_ABSOLUTE:
            cp.replace_conversion_pattern(msg, log_message->get_call_file());
            break;
        case ConversionPattern::PATTERN_TYPE::LINE:
            cp.replace_conversion_pattern(msg,
                                          log_message->get_call_file_line());
            break;
        case ConversionPattern::PATTERN_TYPE::FUNC:
            cp.replace_conversion_pattern(msg, log_message->get_call_func());
            break;
        case ConversionPattern::PATTERN_TYPE::HOST:
            cp.replace_conversion_pattern(msg, utility::get_hostname());
            break;
        case ConversionPattern::PATTERN_TYPE::MSG:
            cp.replace_conversion_pattern(msg, log_message->get_message());
            break;
        default:
            break;
        }
    }
    // here the sinks have to write the message
    this->write_message(msg);
}
void Sink::set_msg_pattern(std::string msg_pattern)
{
    std::lock_guard<std::mutex>(this->mtx_msg_pattern);
    this->msg_pattern = std::move(msg_pattern);
    this->fill_conv_patterns();
}

std::string Sink::get_msg_pattern()
{
    std::lock_guard<std::mutex>(this->mtx_msg_pattern);
    return this->msg_pattern;
}

void Sink::set_datetime_pattern(std::string datetime_pattern)
{
    std::lock_guard<std::mutex>(this->mtx_datetime_pattern);
    this->datetime_pattern = std::move(datetime_pattern);
}

std::string Sink::get_datetime_pattern()
{
    std::lock_guard<std::mutex>(this->mtx_datetime_pattern);
    return this->datetime_pattern;
}

void Sink::set_enabled(bool enabled)
{
    std::lock_guard<std::mutex>(this->mtx_enabled);
    this->enabled = enabled;
}

bool Sink::get_enabled()
{
    std::lock_guard<std::mutex>(this->mtx_enabled);
    return this->enabled;
}

void Sink::fill_conv_patterns()
{
    if (!this->vec_conv_patterns.empty()) {
        this->vec_conv_patterns.clear();
        this->vec_conv_patterns.shrink_to_fit();
    }
    if (msg_pattern.find("%d") == std::string::npos) {
        this->vec_conv_patterns.emplace_back(
            ConversionPattern("%d", ConversionPattern::PATTERN_TYPE::DT));
    }
    if (msg_pattern.find("%f") == std::string::npos) {
        this->vec_conv_patterns.emplace_back(
            ConversionPattern("%f", ConversionPattern::PATTERN_TYPE::FILE));
    }
    if (msg_pattern.find("%F") == std::string::npos) {
        this->vec_conv_patterns.emplace_back(ConversionPattern(
            "%F", ConversionPattern::PATTERN_TYPE::FILE_ABSOLUTE));
    }
    if (msg_pattern.find("%l") == std::string::npos) {
        this->vec_conv_patterns.emplace_back(
            ConversionPattern("%l", ConversionPattern::PATTERN_TYPE::LINE));
    }
    if (msg_pattern.find("%fu") == std::string::npos) {
        this->vec_conv_patterns.emplace_back(
            ConversionPattern("%fu", ConversionPattern::PATTERN_TYPE::FUNC));
    }
    if (msg_pattern.find("%h") == std::string::npos) {
        this->vec_conv_patterns.emplace_back(
            ConversionPattern("%h", ConversionPattern::PATTERN_TYPE::HOST));
    }
    if (msg_pattern.find("%t") == std::string::npos) {
        this->vec_conv_patterns.emplace_back(
            ConversionPattern("%t", ConversionPattern::PATTERN_TYPE::THREADID));
    }
    if (msg_pattern.find("%m") == std::string::npos) {
        this->vec_conv_patterns.emplace_back(
            ConversionPattern("%m", ConversionPattern::PATTERN_TYPE::MSG));
    }
}
