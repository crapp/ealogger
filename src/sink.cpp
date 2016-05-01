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

Sink::Sink(std::shared_ptr<SinkConfig> config) : config(config)
{
    this->fill_conv_patterns();
    this->loglevel_lookup = {{con::LOG_LEVEL::DEBUG, "DEBUG"},
                             {con::LOG_LEVEL::INFO, "INFO"},
                             {con::LOG_LEVEL::WARNING, "WARNING"},
                             {con::LOG_LEVEL::ERROR, "ERROR"},
                             {con::LOG_LEVEL::FATAL, "FATAL"},
                             {con::LOG_LEVEL::STACK, "Stacktrace"},
                             {con::LOG_LEVEL::INTERNAL, "INTERNAL"}};
}
Sink::~Sink() {}
void Sink::set_config(std::shared_ptr<SinkConfig> config)
{
    std::lock_guard<std::mutex> lock(this->mtx_config);
    this->config = std::move(config);
    // TODO: This is not nice, there is no indication msg_pattern has changed but
    // we need to run fill_conv_patterns here.
    this->fill_conv_patterns();
    // "notfiy" derived classes about the changed config
    this->config_changed();
}

std::shared_ptr<SinkConfig> Sink::get_config()
{
    std::lock_guard<std::mutex> lock(this->mtx_config);
    return this->config;
}

void Sink::prepare_log_message(const std::shared_ptr<LogMessage> &log_message)
{
    // I am using a unique lock here that I can unlock myself but also works with
    // RAII
    std::unique_lock<std::mutex> lock(this->mtx_config);
    // TODO: Is it a good idea to check whether this Sink is enabled here or
    // should we check in EALogger?
    if (!this->config->get_enabled())
        return;

    con::LOG_LEVEL msg_lvl =
        static_cast<con::LOG_LEVEL>(log_message->get_severity());
    if (msg_lvl < this->config->get_min_lvl() &&
        log_message->get_log_type() == LogMessage::LOGTYPE::DEFAULT)
        return;

#ifndef PRINT_INTERNAL_MESSAGES
    // Print INTERNAL messages only when defined
    if (msg_lvl == con::LOG_LEVEL::INTERNAL) {
        return;
    }
#endif
    std::string msg = "";
    // TODO: Large if else blocks are not nice. We could move the ConversionPattern
    // for loop to a different function.
    if (log_message->get_log_type() == LogMessage::LOGTYPE::STACK) {
        msg += "Stacktrace \n";
        for (LogMessage::msg_vec_it it = log_message->get_msg_vec_begin();
             it != log_message->get_msg_vec_end(); it++) {
            msg += *it + "\n";
        }
    } else {
        msg = this->config->get_msg_pattern();
        for (const auto &cp : this->vec_conv_patterns) {
            switch (cp.get_pattern_type()) {
            case ConversionPattern::PATTERN_TYPE::DT:
                cp.replace_conversion_pattern(
                    msg, utility::format_time_to_string(
                             log_message->get_timepoint(),
                             this->config->get_datetime_pattern()));
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
            case ConversionPattern::PATTERN_TYPE::LVL:
                cp.replace_conversion_pattern(
                    msg, this->loglevel_lookup.at(static_cast<con::LOG_LEVEL>(
                             log_message->get_severity())));
                break;
            default:
                break;
            }
        }
    }

    lock.unlock();
    // here the sinks have to write the message
    this->write_message(msg);
}

void Sink::fill_conv_patterns()
{
    std::lock_guard<std::mutex> lock(this->mtx_config);
    if (!this->vec_conv_patterns.empty()) {
        this->vec_conv_patterns.clear();
        this->vec_conv_patterns.shrink_to_fit();
    }
    if (this->config->get_msg_pattern().find("%d") != std::string::npos) {
        this->vec_conv_patterns.emplace_back(
            ConversionPattern("%d", ConversionPattern::PATTERN_TYPE::DT));
    }
    if (this->config->get_msg_pattern().find("%f") != std::string::npos) {
        this->vec_conv_patterns.emplace_back(
            ConversionPattern("%f", ConversionPattern::PATTERN_TYPE::FILE));
    }
    if (this->config->get_msg_pattern().find("%F") != std::string::npos) {
        this->vec_conv_patterns.emplace_back(ConversionPattern(
            "%F", ConversionPattern::PATTERN_TYPE::FILE_ABSOLUTE));
    }
    if (this->config->get_msg_pattern().find("%l") != std::string::npos) {
        this->vec_conv_patterns.emplace_back(
            ConversionPattern("%l", ConversionPattern::PATTERN_TYPE::LINE));
    }
    if (this->config->get_msg_pattern().find("%fu") != std::string::npos) {
        this->vec_conv_patterns.emplace_back(
            ConversionPattern("%fu", ConversionPattern::PATTERN_TYPE::FUNC));
    }
    if (this->config->get_msg_pattern().find("%h") != std::string::npos) {
        this->vec_conv_patterns.emplace_back(
            ConversionPattern("%h", ConversionPattern::PATTERN_TYPE::HOST));
    }
    if (this->config->get_msg_pattern().find("%t") != std::string::npos) {
        this->vec_conv_patterns.emplace_back(
            ConversionPattern("%t", ConversionPattern::PATTERN_TYPE::THREADID));
    }
    if (this->config->get_msg_pattern().find("%m") != std::string::npos) {
        this->vec_conv_patterns.emplace_back(
            ConversionPattern("%m", ConversionPattern::PATTERN_TYPE::MSG));
    }
    if (this->config->get_msg_pattern().find("%s") != std::string::npos) {
        this->vec_conv_patterns.emplace_back(
            ConversionPattern("%s", ConversionPattern::PATTERN_TYPE::LVL));
    }
}
