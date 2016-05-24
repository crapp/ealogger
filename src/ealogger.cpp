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

#include <ealogger/ealogger.h>

/**
 * @file ealogger.cpp
 */

namespace eal = ealogger;
namespace con = ealogger::constants;

eal::Logger::Logger(bool async) : async(async)
{
    this->logger_mutex_map.emplace(
        con::LOGGER_SINK::CONSOLES,
        std::unique_ptr<std::mutex>(new std::mutex()));
    this->logger_mutex_map.emplace(
        con::LOGGER_SINK::SYSLOG, std::unique_ptr<std::mutex>(new std::mutex()));
    this->logger_mutex_map.emplace(
        con::LOGGER_SINK::FILE_SIMPLE,
        std::unique_ptr<std::mutex>(new std::mutex()));
// TODO: Make registration of signal handler configurable
#ifdef __linux__
    if (signal(SIGUSR1, eal::Logger::logrotate) == SIG_ERR)
        throw std::runtime_error("Could not create signal handler for SIGUSR1");
#endif

    if (this->async) {
        logger_thread_stop = false;
        logger_thread = std::thread(&eal::Logger::thread_entry_point, this);
    }
}

eal::Logger::~Logger()
{
    if (this->async) {
        // wait for queue to be emptied. after 1 second we will exit the background logger thread
        int i = 0;
        // TODO: Make this wait for queue to be empty optional
        while (!this->log_msg_queue.empty()) {
            if (i == 100)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            i++;
        }
        this->set_logger_thread_stop(true);
        // we need to write one more log message to wakeup the background logger
        // thread it will pop the last message from the queue.
        this->write_log("Logger EXIT", con::LOG_LEVEL::INTERNAL, "", 0, "");
        try {
            logger_thread.join();
        } catch (const std::system_error &ex) {
            std::cerr << "Could not join with logger background thread: "
                      << ex.what() << std::endl;
        }
    }
}

void eal::Logger::write_log(std::string msg, con::LOG_LEVEL lvl,
                            std::string file, int lnumber, std::string func)
{
    std::shared_ptr<LogMessage> m;
    LogMessage::LOGTYPE type = LogMessage::LOGTYPE::DEFAULT;
    if (lvl == con::LOG_LEVEL::STACK) {
        type = LogMessage::LOGTYPE::STACK;
        std::vector<std::string> stack_vec;
        // TODO: Stack size is hard coded
        eal::utility::stack_trace(10, stack_vec);
        m = std::make_shared<LogMessage>(lvl, std::move(stack_vec), type,
                                         std::move(file), lnumber,
                                         std::move(func));
    } else {
        m = std::make_shared<LogMessage>(lvl, std::move(msg), type,
                                         std::move(file), lnumber,
                                         std::move(func));
    }
    if (this->async) {
        this->log_msg_queue.push(std::move(m));
    } else {
        this->internal_log_routine(std::move(m));
    }
}

void eal::Logger::write_log(std::string msg, con::LOG_LEVEL lvl)
{
    this->write_log(std::move(msg), lvl, "", 0, "");
}

void eal::Logger::init_syslog_sink(bool enabled, con::LOG_LEVEL min_lvl,
                                   std::string msg_template,
                                   std::string datetime_pattern)
{
    try {
        std::lock_guard<std::mutex> lock(
            *(this->logger_mutex_map[con::LOGGER_SINK::SYSLOG].get()));
        this->logger_sink_map[con::LOGGER_SINK::SYSLOG] =
            std::make_shared<SinkSyslog>(std::move(msg_template),
                                         std::move(datetime_pattern), enabled,
                                         min_lvl);
    } catch (const std::exception &ex) {
    }
}
void eal::Logger::init_console_sink(bool enabled, con::LOG_LEVEL min_lvl,
                                    std::string msg_template,
                                    std::string datetime_pattern)
{
    try {
        std::lock_guard<std::mutex> lock(
            *(this->logger_mutex_map[con::LOGGER_SINK::CONSOLES].get()));
        this->logger_sink_map[con::LOGGER_SINK::CONSOLES] =
            std::make_shared<SinkConsole>(std::move(msg_template),
                                          std::move(datetime_pattern), enabled,
                                          min_lvl);
    } catch (const std::exception &ex) {
    }
}
void eal::Logger::init_file_sink(bool enabled, con::LOG_LEVEL min_lvl,
                                 std::string msg_template,
                                 std::string datetime_pattern,
                                 std::string logfile,
                                 bool flush_buffer)
{
    try {
        std::lock_guard<std::mutex> lock(
            *(this->logger_mutex_map[con::LOGGER_SINK::FILE_SIMPLE].get()));
        this->logger_sink_map[con::LOGGER_SINK::FILE_SIMPLE] =
            std::make_shared<SinkFile>(std::move(msg_template),
                                       std::move(datetime_pattern), enabled,
                                       min_lvl, std::move(logfile),
                                       flush_buffer);
    } catch (const std::exception &ex) {
    }
}

// void eal::Logger::init_file_sink_rotating(bool enabled, con::LOG_LEVEL
// min_lvl,
// std::string msg_template,
// std::string datetime_pattern,
// std::string logfile)
//{
//}

void eal::Logger::set_msg_template(con::LOGGER_SINK sink,
                                   std::string msg_template)
{
    try {
        std::lock_guard<std::mutex> lock(*(this->logger_mutex_map[sink].get()));
        this->logger_sink_map.at(sink)->set_msg_template(
            std::move(msg_template));
    } catch (const std::out_of_range &ex) {
        // TODO: What do we do here if the sink does not exist?
    }
}
void eal::Logger::set_datetime_pattern(con::LOGGER_SINK sink,
                                       std::string datetime_pattern)
{
    try {
        std::lock_guard<std::mutex> lock(*(this->logger_mutex_map[sink].get()));
        this->logger_sink_map.at(sink)->set_datetime_pattern(
            std::move(datetime_pattern));
    } catch (const std::out_of_range &ex) {
        // TODO: What do we do here if the sink does not exist?
    }
}
void eal::Logger::set_enabled(con::LOGGER_SINK sink, bool enabled)
{
    try {
        std::lock_guard<std::mutex> lock(*(this->logger_mutex_map[sink].get()));
        this->logger_sink_map.at(sink)->set_enabled(enabled);
    } catch (const std::out_of_range &ex) {
        // TODO: What do we do here if the sink does not exist?
    }
}
void eal::Logger::set_min_lvl(con::LOGGER_SINK sink, con::LOG_LEVEL min_level)
{
    try {
        std::lock_guard<std::mutex> lock(*(this->logger_mutex_map[sink].get()));
        this->logger_sink_map.at(sink)->set_min_lvl(min_level);
    } catch (const std::out_of_range &ex) {
        // TODO: What do we do here if the sink does not exist?
    }
}

void eal::Logger::discard_sink(con::LOGGER_SINK sink)
{
    try {
        std::lock_guard<std::mutex> lock(*(this->logger_mutex_map[sink].get()));
        std::size_t removed = this->logger_sink_map.erase(sink);
        if (removed > 0) {
            // should we use this?
        }
    } catch (const std::exception &ex) {
        // TODO: What do we do here if the sink does not exist?
    }
}

bool eal::Logger::is_initialized(con::LOGGER_SINK sink)
{
    bool ret = false;
    try {
        std::lock_guard<std::mutex> lock(*(this->logger_mutex_map[sink].get()));
        if (this->logger_sink_map.find(sink) != this->logger_sink_map.end()) {
            ret = true;
        }
    } catch(const std::exception &ex) {

    }
    return ret;
}

bool eal::Logger::queue_empty() { return this->log_msg_queue.empty(); }
void eal::Logger::logrotate(int signo)
{
#ifdef __linux__
    if (signo == SIGUSR1) {
        eal::Logger::signal_SIGUSR1 = true;
    }
#endif
}

void eal::Logger::thread_entry_point()
{
    while (!this->get_logger_thread_stop()) {
        std::shared_ptr<LogMessage> m = this->log_msg_queue.pop();
        this->internal_log_routine(std::move(m));
    }
}

void eal::Logger::internal_log_routine(std::shared_ptr<LogMessage> m)
{
    for (const auto &sink : logger_sink_map) {
        std::lock_guard<std::mutex> lock(
            *(this->logger_mutex_map[sink.first].get()));
        sink.second->prepare_log_message(m);
    }
}

bool eal::Logger::get_logger_thread_stop()
{
    std::lock_guard<std::mutex> guard(this->mtx_logger_stop);
    return this->logger_thread_stop;
}

void eal::Logger::set_logger_thread_stop(bool stop)
{
    std::lock_guard<std::mutex> guard(this->mtx_logger_stop);
    this->logger_thread_stop = stop;
}

bool eal::Logger::signal_SIGUSR1 = false;
