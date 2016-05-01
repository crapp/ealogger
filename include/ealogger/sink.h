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

#ifndef SINK_H
#define SINK_H

#include <string>
#include <algorithm>
#include <memory>
#include <mutex>
#include <iostream>

#include "utility.h"
#include "global.h"
#include "conversion_pattern.h"
#include "logmessage.h"

namespace con = ealogger_constants;

/**
 * @brief Struct with the base configuration for a logger sink
 */
struct SinkConfig {
public:
    /**
     * @brief Constructor for the base config
     *
     * @param msg_pattern The message pattern for this sink
     * @param datetime_pattern The datetime pattern
     * @param enabled Whether or not this sink is enabled
     * @param min_lvl The minimum log severity
     *
     * A Log message can have conversion patterns. Every Sink uses its own message
     * pattern. Have a look at ConversionPattern for more details.
     *
     * Internally std::strftime is used from header ctime to create formatted time
     * strings.
     * So for parameter Sink#datetime_pattern you have to use format specifiers
     * that are recognised by strftime.
     * See [en.cppreference.com](http://en.cppreference.com/w/cpp/chrono/c/strftime)
     * for details.
     * For example "%H:%M:%S" returns a 24-hour based time string like 20:12:01
     */
    SinkConfig(std::string msg_pattern, std::string datetime_pattern,
               bool enabled, con::LOG_LEVEL min_lvl)
        : msg_pattern(std::move(msg_pattern)),
          datetime_pattern(std::move(datetime_pattern)),
          enabled(enabled),
          min_level(min_lvl){};
    virtual ~SinkConfig(){};

    void set_msg_pattern(std::string msg_pattern)
    {
        std::lock_guard<std::mutex> lock(this->mtx_msg_pattern);
        this->msg_pattern = std::move(msg_pattern);
        // TODO: How could I notify the Sink object about the new msg_pattern?
        // this->fill_conv_patterns();
    }
    std::string get_msg_pattern()
    {
        std::lock_guard<std::mutex> lock(this->mtx_msg_pattern);
        return this->msg_pattern;
    }

    void set_datetime_pattern(std::string datetime_pattern)
    {
        std::lock_guard<std::mutex> lock(this->mtx_datetime_pattern);
        this->datetime_pattern = std::move(datetime_pattern);
    }
    std::string get_datetime_pattern()
    {
        std::lock_guard<std::mutex> lock(this->mtx_datetime_pattern);
        return this->datetime_pattern;
    }

    void set_enabled(bool enabled)
    {
        std::lock_guard<std::mutex> lock(this->mtx_enabled);
        this->enabled = enabled;
    }
    bool get_enabled()
    {
        std::lock_guard<std::mutex> lock(this->mtx_enabled);
        return this->enabled;
    }

    void set_min_lvl(con::LOG_LEVEL min_lvl)
    {
        std::lock_guard<std::mutex> lock(this->mtx_min_lvl);
        this->min_level = min_lvl;
    }
    con::LOG_LEVEL get_min_lvl()
    {
        std::lock_guard<std::mutex> lock(this->mtx_min_lvl);
        return this->min_level;
    }

protected:
    std::mutex mtx_msg_pattern;
    std::mutex mtx_datetime_pattern;
    std::mutex mtx_enabled;
    std::mutex mtx_min_lvl;

    std::string msg_pattern;
    std::string datetime_pattern;
    bool enabled;
    con::LOG_LEVEL min_level;
};

/**
 * @brief A sink is an object that writes the log message to the target
 * @author Christian Rapp
 * @details
 *
 * The virtual class Sink has to be implemented by each possible target. To add
 * a new Sink to ealogger you have to provide an implementaion of Sink::write_message
 */
class Sink
{
public:
    /**
     * @brief Sink constructor
     * @param config Configuration object
     */
    Sink(std::shared_ptr<SinkConfig> config);
    virtual ~Sink();

    // FIXME: The user can provide here a config object of the wrong derived
    // class
    // dynamic_pointer_cast would fail in this case. We must make this virtual and
    // implement the method in each sink. The means we need some kind of boilerplate
    // for this method. Because we have to lock a mutex and renew the ConversionPattern
    // vector.
    void set_config(std::shared_ptr<SinkConfig> config);
    std::shared_ptr<SinkConfig> get_config();

    void prepare_log_message(const std::shared_ptr<LogMessage> &log_message);

protected:
    std::shared_ptr<SinkConfig> config;
    std::vector<ConversionPattern> vec_conv_patterns;

    /** lookup table for loglevel Strings */
    std::map<con::LOG_LEVEL, std::string> loglevel_lookup;
    std::mutex mtx_config;

    /**
     * @brief Fill Sink#vec_conv_patterns with ConverionPattern depending on
     * Sink#msg_pattern
     */
    void fill_conv_patterns();
    /**
     * @brief Writes a LogMessage object to the logger sink
     *
     * @param msg LogMessage object
     * @details
     * This interface method has to be implemented by every logger sink. The sink
     * is required to format the log message according to Sink::msg_pattern and
     * Sink::datetime_pattern. After the message is ready it will be written to the
     * specified sink.
     */
    virtual void write_message(const std::string &msg) = 0;

    /**
     * @brief This method will be called when the SinkConfig option changes
     * @details
     * Derived Sink classes have to implement this and take necessary steps
     * regarding a changed configuration file
     */
    virtual void config_changed() = 0;
};

#endif /* SINK_H */
