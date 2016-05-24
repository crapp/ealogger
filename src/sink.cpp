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

#include <ealogger/sink.h>

namespace eal = ealogger;
namespace con = ealogger::constants;

eal::Sink::Sink(std::string msg_template, std::string datetime_pattern,
                bool enabled, con::LOG_LEVEL min_lvl)
    : msg_template(std::move(msg_template)),
      datetime_pattern(std::move(datetime_pattern)),
      enabled(enabled),
      min_level(min_lvl)
{
    this->fill_conv_patterns(true);
    this->loglevel_lookup = {{con::LOG_LEVEL::EAL_DEBUG, "DEBUG"},
                             {con::LOG_LEVEL::EAL_INFO, "INFO"},
                             {con::LOG_LEVEL::EAL_WARNING, "WARNING"},
                             {con::LOG_LEVEL::EAL_ERROR, "ERROR"},
                             {con::LOG_LEVEL::EAL_FATAL, "FATAL"},
                             {con::LOG_LEVEL::EAL_STACK, "Stacktrace"},
                             {con::LOG_LEVEL::EAL_INTERNAL, "INTERNAL"}};
}
eal::Sink::~Sink() {}
void eal::Sink::set_msg_template(std::string msg_template)
{
    std::lock_guard<std::mutex> lock(this->mtx_msg_template);
    this->msg_template = std::move(msg_template);
    this->fill_conv_patterns(false);
}

void eal::Sink::set_datetime_pattern(std::string datetime_pattern)
{
    std::lock_guard<std::mutex> lock(this->mtx_datetime_pattern);
    this->datetime_pattern = std::move(datetime_pattern);
}

void eal::Sink::set_enabled(bool enabled)
{
    std::lock_guard<std::mutex> lock(this->mtx_enabled);
    this->enabled = enabled;
    this->config_changed();
}
bool eal::Sink::get_enabled()
{
    std::lock_guard<std::mutex> lock(this->mtx_enabled);
    return this->enabled;
}

void eal::Sink::set_min_lvl(con::LOG_LEVEL min_lvl)
{
    std::lock_guard<std::mutex> lock(this->mtx_min_lvl);
    this->min_level = min_lvl;
}

void eal::Sink::prepare_log_message(
    const std::shared_ptr<LogMessage> &log_message)
{
    // TODO: Is it a good idea to check whether this Sink is enabled here or
    // should we check in Logger?
    if (!this->get_enabled())
        return;

    con::LOG_LEVEL msg_lvl = log_message->get_severity();

    std::unique_lock<std::mutex> min_level_lock(this->mtx_min_lvl);
    if (msg_lvl < this->min_level &&
        log_message->get_log_type() == LogMessage::LOGTYPE::DEFAULT)
        return;
    min_level_lock.unlock();

#ifndef EALOGGER_PRINT_INTERNAL
    // Print INTERNAL messages only when defined
    if (msg_lvl == con::LOG_LEVEL::EAL_INTERNAL) {
        return;
    }
#endif

    std::string msg = "";
    // TODO: Large if else blocks are not nice. We could move the ConversionPattern
    // for loop to a different location
    if (log_message->get_log_type() == LogMessage::LOGTYPE::STACK) {
        msg += "Stacktrace \n";
        for (LogMessage::msg_vec_it it = log_message->get_msg_vec_begin();
             it != log_message->get_msg_vec_end(); it++) {
            msg += *it + "\n";
        }
    } else {
        std::unique_lock<std::mutex> msg_template_lock(this->mtx_msg_template);
        msg = this->msg_template;
        msg_template_lock.unlock();
        std::lock_guard<std::mutex> vec_conv_patterns_lock(
            this->mtx_conv_pattern);
        for (const auto &cp : this->vec_conv_patterns) {
            switch (cp.get_pattern_type()) {
            case ConversionPattern::PATTERN_TYPE::DT: {
                std::unique_lock<std::mutex> datetime_pattern_lock(
                    this->mtx_datetime_pattern);
                cp.replace_conversion_pattern(
                    msg,
                    eal::utility::format_time_to_string(
                        log_message->get_timepoint(), this->datetime_pattern));
            } break;
            case ConversionPattern::PATTERN_TYPE::FILE:
                cp.replace_conversion_pattern(
                    msg,
                    eal::utility::get_file_name(log_message->get_call_file()));
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
                cp.replace_conversion_pattern(msg, eal::utility::get_hostname());
                break;
            case ConversionPattern::PATTERN_TYPE::MSG:
                cp.replace_conversion_pattern(msg, log_message->get_message());
                break;
            case ConversionPattern::PATTERN_TYPE::LVL:
                cp.replace_conversion_pattern(
                    msg, this->loglevel_lookup.at(log_message->get_severity()));
                break;
            default:
                break;
            }
        }
    }

    // here the sinks have to write the message
    this->write_message(msg);
}

void eal::Sink::fill_conv_patterns(bool lock)
{
    std::lock_guard<std::mutex> vec_conv_patterns_lock(this->mtx_conv_pattern);
    if (!this->vec_conv_patterns.empty()) {
        this->vec_conv_patterns.clear();
        this->vec_conv_patterns.shrink_to_fit();
    }

    std::string msgp = "";
    if (lock) {
        std::unique_lock<std::mutex> msg_template_lock(this->mtx_msg_template);
        msgp = this->msg_template;
        msg_template_lock.unlock();
    } else {
        msgp = this->msg_template;
    }

    if (msgp.find("%d") != std::string::npos) {
        this->vec_conv_patterns.emplace_back(
            ConversionPattern("%d", ConversionPattern::PATTERN_TYPE::DT));
    }
    if (msgp.find("%f") != std::string::npos) {
        this->vec_conv_patterns.emplace_back(
            ConversionPattern("%f", ConversionPattern::PATTERN_TYPE::FILE));
    }
    if (msgp.find("%F") != std::string::npos) {
        this->vec_conv_patterns.emplace_back(ConversionPattern(
            "%F", ConversionPattern::PATTERN_TYPE::FILE_ABSOLUTE));
    }
    if (msgp.find("%l") != std::string::npos) {
        this->vec_conv_patterns.emplace_back(
            ConversionPattern("%l", ConversionPattern::PATTERN_TYPE::LINE));
    }
    if (msgp.find("%u") != std::string::npos) {
        this->vec_conv_patterns.emplace_back(
            ConversionPattern("%u", ConversionPattern::PATTERN_TYPE::FUNC));
    }
    if (msgp.find("%h") != std::string::npos) {
        this->vec_conv_patterns.emplace_back(
            ConversionPattern("%h", ConversionPattern::PATTERN_TYPE::HOST));
    }
    if (msgp.find("%t") != std::string::npos) {
        this->vec_conv_patterns.emplace_back(
            ConversionPattern("%t", ConversionPattern::PATTERN_TYPE::THREADID));
    }
    if (msgp.find("%m") != std::string::npos) {
        this->vec_conv_patterns.emplace_back(
            ConversionPattern("%m", ConversionPattern::PATTERN_TYPE::MSG));
    }
    if (msgp.find("%s") != std::string::npos) {
        this->vec_conv_patterns.emplace_back(
            ConversionPattern("%s", ConversionPattern::PATTERN_TYPE::LVL));
    }
}
