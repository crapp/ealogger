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

EALogger::EALogger(EALogger::log_level min_level, bool log_to_console,
                   bool log_to_file, bool log_to_syslog, bool async,
                   bool print_tid, std::string dt_format, std::string logfile)
    : min_level(min_level),
      log_to_console(log_to_console),
      log_to_file(log_to_file),
      log_to_syslog(log_to_syslog),
      async(async),
      print_tid(print_tid),
      dt_format(dt_format),
      logfile_path(logfile)
{
    // TODO: Make registration of signal handler configurable
    EALogger::signal_SIGUSR1 = false;
#ifdef __linux__
    if (signal(SIGUSR1, EALogger::logrotate) == SIG_ERR)
        throw std::runtime_error("Could not create signal handler for SIGUSR1");
#endif

    this->loglevel_lookup = {{EALogger::log_level::DEBUG, " DEBUG: "},
                             {EALogger::log_level::INFO, " INFO: "},
                             {EALogger::log_level::WARNING, " WARNING: "},
                             {EALogger::log_level::ERROR, " ERROR: "},
                             {EALogger::log_level::FATAL, " FATAL: "},
                             {EALogger::log_level::INTERNAL, " INTERNAL: "}};
#ifdef SYSLOG
    this->loglevel_syslog_lookup = {
        {EALogger::log_level::DEBUG, LOG_DEBUG},
        {EALogger::log_level::INFO, LOG_INFO},
        {EALogger::log_level::WARNING, LOG_WARNING},
        {EALogger::log_level::ERROR, LOG_ERR},
        {EALogger::log_level::FATAL, LOG_CRIT},
        {EALogger::log_level::INTERNAL,
         LOG_DEBUG}};  // mapping internal to syslog debug
#else
    this->loglevel_syslog_lookup = {};
#endif

    if (this->get_log_to_file()) {
        this->logfile_stream.open(this->logfile_path,
                                  std::ios::out | std::ios::app);
        if (!this->logfile_stream)
            throw std::runtime_error("Can not open logfile: " +
                                     this->logfile_path);
        // set exception mask for the file stream
        this->logfile_stream.exceptions(std::ifstream::badbit |
                                        std::ifstream::failbit);
    }

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
        this->write_log(EALogger::log_level::INTERNAL, "Logger exit");
        try {
            logger_thread.join();
        } catch (const std::system_error &ex) {
            std::cerr << "Could not join with logger background thread: "
                      << ex.what() << std::endl;
        }
    }
    if (this->logfile_stream && this->logfile_stream.is_open()) {
        this->logfile_stream.flush();
        this->logfile_stream.close();
    }
}

void EALogger::write_log(EALogger::log_level lvl, std::string msg)
{
    std::shared_ptr<LogMessage> m;
    if (this->async) {
        m = std::make_shared<LogMessage>(lvl, std::move(msg),
                                         LogMessage::LOGTYPE::DEFAULT);
        this->log_msg_queue.push(m);
    } else {
        m = std::make_shared<LogMessage>(lvl, std::move(msg),
                                         LogMessage::LOGTYPE::DEFAULT);
        this->internal_log_routine(m);
    }
}

void EALogger::debug(std::string msg)
{
    this->write_log(EALogger::log_level::DEBUG, std::move(msg));
}

void EALogger::info(std::string msg)
{
    this->write_log(EALogger::log_level::INFO, std::move(msg));
}

void EALogger::warn(std::string msg)
{
    this->write_log(EALogger::log_level::WARNING, std::move(msg));
}

void EALogger::error(std::string msg)
{
    this->write_log(EALogger::log_level::ERROR, std::move(msg));
}

void EALogger::fatal(std::string msg)
{
    this->write_log(EALogger::log_level::FATAL, std::move(msg));
}

void EALogger::stack_trace(unsigned int size)
{
#ifdef __GLIBC__
    void *addrlist[size + 1];

    size_t no_of_stack_addresses =
        backtrace(addrlist, sizeof(addrlist) / sizeof(void *));
    char **temp_symbols = backtrace_symbols(addrlist, no_of_stack_addresses);

    // initialize a vector of string with the char**
    std::vector<std::string> symbollist(temp_symbols,
                                        temp_symbols + no_of_stack_addresses);
    std::vector<std::string> stackvec;
    // temp_symbols has to be freed
    free(temp_symbols);

    //std::regex rgx("\\(.*\\+|\\+0[xX][0-9a-fA-F]+\\)");
    std::regex rgx("(^.*\\()(.*\\+)(0[xX][0-9a-fA-F]+\\))");

    // TODO: This always prints stack_trace as first symbol. Should we omit
    // this?
    for (const auto &symbol : symbollist) {
        std::cout << "Symbol: " << symbol << std::endl;
        std::smatch match;
        // if there is no match with our regex we have to continue and use the
        // original symbol
        if (!std::regex_search(symbol, match, rgx)) {
            std::cout << "no regex match" << std::endl;
            stackvec.push_back(symbol);
            continue;
        }
        // get the regex matches and create the 3 strings we need
        std::string file = match[1].str();
        file = file.substr(0, file.size() - 1);
        std::string mangled_name = match[2].str();
        mangled_name = mangled_name.substr(0, mangled_name.size() - 1);
        std::string caller = match[3].str();
        caller = caller.substr(0, caller.size() - 1);

        // demangle status
        int status = 0;
        // must be freed
        char *realname =
            abi::__cxa_demangle(mangled_name.c_str(), 0, 0, &status);
        if (status == 0) {
            stackvec.emplace_back(file + " : " + std::string(realname) + "+" +
                                  caller);
        } else {
            stackvec.emplace_back(file + " : " + mangled_name + "()" + "+" +
                                  caller);
        }
        free(realname);
    }
    std::shared_ptr<LogMessage> m = std::make_shared<LogMessage>(
        EALogger::log_level::DEBUG, stackvec, LogMessage::LOGTYPE::STACK);
    if (this->async) {
        this->log_msg_queue.push(m);
    } else {
        this->internal_log_routine(m);
    }
#endif
}

void EALogger::set_dt_format(std::string fmt)
{
    std::lock_guard<std::mutex> guard(this->mtx_dt_format);
    this->dt_format = fmt;
}

std::string EALogger::get_dt_format()
{
    std::lock_guard<std::mutex> guard(this->mtx_dt_format);
    return this->dt_format;
}

void EALogger::set_log_to_console(bool b)
{
    std::lock_guard<std::mutex> guard(this->mtx_log_console);
    this->log_to_console = b;
}

bool EALogger::get_log_to_console()
{
    std::lock_guard<std::mutex> guard(this->mtx_log_console);
    return this->log_to_console;
}

void EALogger::set_log_to_file(bool b)
{
    std::lock_guard<std::mutex> guard(this->mtx_log_file);
    this->log_to_file = b;
}

bool EALogger::get_log_to_file()
{
    std::lock_guard<std::mutex> guard(this->mtx_log_file);
    return this->log_to_file;
}

void EALogger::set_log_to_syslog(bool b)
{
    std::lock_guard<std::mutex> guard(this->mtx_log_syslog);
    this->log_to_syslog = b;
}

bool EALogger::get_log_to_syslog()
{
    std::lock_guard<std::mutex> guard(this->mtx_log_syslog);
    return this->log_to_syslog;
}

void EALogger::set_print_tid(bool b)
{
    std::lock_guard<std::mutex> guard(this->mtx_print_tid);
    this->print_tid = b;
}

bool EALogger::get_print_tid()
{
    std::lock_guard<std::mutex> guard(this->mtx_print_tid);
    return this->print_tid;
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
        this->internal_log_routine(m);
    }
}

void EALogger::internal_log_routine(std::shared_ptr<LogMessage> m)
{
    // lock mutex because iostreams or fstreams are not threadsafe
    if (!this->async)
        std::lock_guard<std::mutex> lock(this->mtx_log);
    if (EALogger::signal_SIGUSR1) {
        EALogger::signal_SIGUSR1 = false;
        this->logfile_stream.flush();
        this->logfile_stream.close();
        this->logfile_stream.open(this->logfile_path,
                                  std::ios::out | std::ios::app);
        if (!this->logfile_stream)
            throw std::runtime_error("Can not open logfile: " +
                                     this->logfile_path);
        // set exception mask for the file stream
        this->logfile_stream.exceptions(std::ifstream::badbit |
                                        std::ifstream::failbit);
    }
    EALogger::log_level msg_lvl =
        static_cast<EALogger::log_level>(m->get_severity());
    if (msg_lvl >= this->min_level ||
        m->get_log_type() == LogMessage::LOGTYPE::STACK) {
        try {
            std::string t_id = "";
            if (this->print_tid) {
                std::stringstream ss;
                ss << std::this_thread::get_id();
                t_id = " " + ss.str();
            }

            if (m->get_log_type() == LogMessage::LOGTYPE::STACK) {
                std::stringstream stack_message;
                stack_message
                    << "[" << this->format_time_to_string(this->get_dt_format())
                    << "]" << t_id << " Stacktrace: " << std::endl;
                if (this->get_log_to_console()) {
                    std::cout << stack_message.str();
                }
                if (this->get_log_to_file()) {
                    this->logfile_stream << stack_message.str();
                }
#ifdef SYSLOG
                if (this->get_log_to_syslog()) {
                    syslog(this->loglevel_syslog_lookup.at(msg_lvl),
                           "Stacktrace:");
                }
#endif
                // TODO: Old style vector iteration. c++11 range based for loop
                // would be nice
                for (std::vector<std::string>::const_iterator it =
                         m->get_msg_vec_begin();
                     it != m->get_msg_vec_end(); it++) {
                    if (this->get_log_to_console())
                        std::cout << "\t" << *it << std::endl;
                    if (this->get_log_to_file())
                        this->logfile_stream << "\t" << *it << std::endl;
#ifdef SYSLOG
                    if (this->get_log_to_syslog()) {
                        syslog(this->loglevel_syslog_lookup.at(msg_lvl), "\t %s",
                               it->c_str());
                    }
#endif
                }
            } else {
#ifndef PRINT_INTERNAL_MESSAGES
                // Print INTERNAL messages only when defined
                if (msg_lvl == EALogger::log_level::INTERNAL) {
                    return;
                }
#endif
                std::string log_level_string = this->loglevel_lookup.at(msg_lvl);
                std::stringstream log_stringstream;
                log_stringstream
                    << "[" << this->format_time_to_string(this->get_dt_format())
                    << "]" << t_id << log_level_string << m->get_message()
                    << std::endl;
                if (this->get_log_to_console())
                    std::cout << log_stringstream.str();
                if (this->get_log_to_file())
                    this->logfile_stream << log_stringstream.str();
#ifdef SYSLOG
                if (this->get_log_to_syslog())
                    syslog(this->loglevel_syslog_lookup.at(msg_lvl), "%s",
                           m->get_message().c_str());
#endif
            }

        } catch (const std::ios_base::failure &fail) {
            std::cerr << "Can not write to Logfile stream" << std::endl;
            // std::cerr << "Error: " << fail << std::endl;
        }
    }
}

std::string EALogger::format_time_to_string(const std::string &time_format)
{
    // get raw time
    time_t rawtime;
    std::time(&rawtime);
    return this->format_time_to_string(std::move(rawtime), time_format);
}

std::string EALogger::format_time_to_string(std::time_t t,
                                            const std::string &time_format)
{
    // time struct
    struct tm *timeinfo;
    // buffer where we store the formatted time string
    char buffer[80];

    // FIXME: This is not threadsafe. Use localtime_r instead
    timeinfo = std::localtime(&t);
    //    localtime_r

    std::strftime(buffer, 80, time_format.c_str(), timeinfo);
    return (std::string(buffer));
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

bool EALogger::signal_SIGUSR1;
