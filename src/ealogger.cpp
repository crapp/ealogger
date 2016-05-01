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

#include "ealogger.h"

EALogger::EALogger(bool async) : async(async)
{
    this->logger_sink_map.emplace(std::make_pair(
        con::LOGGER_SINK::CONSOLES,
        std::make_shared<SinkConsole>(std::make_shared<SinkConfigConsole>(
            "%d %s: %m", "%F %T", true, con::LOG_LEVEL::DEBUG))));
    this->logger_sink_map.emplace(std::make_pair(
        con::LOGGER_SINK::FILES,
        std::make_shared<SinkFile>(std::make_shared<SinkConfigFile>(
            "%d %s [%f:%l] %m", "%F %T", false, con::LOG_LEVEL::DEBUG,
            "ealogger_logfile.log"))));
    this->logger_sink_map.emplace(std::make_pair(
        con::LOGGER_SINK::SYSLOGS,
        std::make_shared<SinkSyslog>(std::make_shared<SinkConfigSyslog>(
            "%s: %m", "%F %T", true, con::LOG_LEVEL::DEBUG))));
// TODO: Make registration of signal handler configurable
#ifdef __linux__
    if (signal(SIGUSR1, EALogger::logrotate) == SIG_ERR)
        throw std::runtime_error("Could not create signal handler for SIGUSR1");
#endif

    if (this->async) {
        logger_thread_stop = false;
        logger_thread = std::thread(&EALogger::thread_entry_point, this);
    }
}

EALogger::~EALogger()
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

void EALogger::write_log(std::string msg, con::LOG_LEVEL lvl, std::string file,
                         int lnumber, std::string func)
{
    std::shared_ptr<LogMessage> m;
    LogMessage::LOGTYPE type = LogMessage::LOGTYPE::DEFAULT;
    if (lvl == con::LOG_LEVEL::STACK) {
        type = LogMessage::LOGTYPE::STACK;
        std::vector<std::string> stack_vec;
        // TODO: Stack size is hard coded
        utility::stack_trace(10, stack_vec);
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

std::shared_ptr<SinkConfig> EALogger::get_sink_config(con::LOGGER_SINK sink)
{
    return this->logger_sink_map.at(sink)->get_config();
}

void EALogger::set_sink_config(con::LOGGER_SINK sink,
                               std::shared_ptr<SinkConfig> config)
{
    this->logger_sink_map[sink]->set_config(std::move(config));
}

void EALogger::logrotate(int signo)
{
#ifdef __linux__
    if (signo == SIGUSR1) {
        EALogger::signal_SIGUSR1 = true;
    }
#endif
}

void EALogger::thread_entry_point()
{
    while (!this->get_logger_thread_stop()) {
        std::shared_ptr<LogMessage> m = this->log_msg_queue.pop();
        this->internal_log_routine(std::move(m));
    }
}

void EALogger::internal_log_routine(std::shared_ptr<LogMessage> m)
{
    // lock mutex because iostreams or fstreams are not threadsafe
    // if (!this->async)
    // std::lock_guard<std::mutex> lock(this->mtx_log);
    // if (EALogger::signal_SIGUSR1) {
    // EALogger::signal_SIGUSR1 = false;
    //this->logfile_stream.flush();
    //this->logfile_stream.close();
    //this->logfile_stream.open(this->logfile_path,
    // std::ios::out | std::ios::app);
    // if (!this->logfile_stream)
    // throw std::runtime_error("Can not open logfile: " +
    // this->logfile_path);
    //// set exception mask for the file stream
    //this->logfile_stream.exceptions(std::ifstream::badbit |
    // std::ifstream::failbit);
    //}
    for (const auto &sink : logger_sink_map) {
        sink.second->prepare_log_message(m);
    }
}

bool EALogger::get_logger_thread_stop()
{
    std::lock_guard<std::mutex> guard(this->mtx_logger_stop);
    return this->logger_thread_stop;
}

void EALogger::set_logger_thread_stop(bool stop)
{
    std::lock_guard<std::mutex> guard(this->mtx_logger_stop);
    this->logger_thread_stop = stop;
}

bool EALogger::signal_SIGUSR1 = false;
