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

/**
 *@file sink.h
 */

#include <algorithm>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include <iostream>

#include <ealogger/conversion_pattern.h>
#include <ealogger/global.h>
#include <ealogger/logmessage.h>
#include <ealogger/utility.h>

namespace ealogger
{
/**
 * @defgroup SINK_GROUP Sinks
 * @brief Documentation for all sinks ealogger is able to use
 * @{
 */

/**
 * @brief A sink is an object that writes the log message to a specific target
 * @author Christian Rapp
 * @details
 *
 * The virtual class Sink has to be implemented by each possible target. To add
 * a new Sink to ealogger you have to provide an implementation for Sink::write_message
 */
class Sink
{
public:
    /**
     * @brief Sink constructor
     * @param msg_template Message template for this sink
     * @param datetime_pattern datetime conversion pattern
     * @param enabled Whether or not this sink is enabled
     * @param min_lvl The minimum log severity
     *
     * A Log message can use conversion patterns in the message template \p msg_template.
     * Every Sink uses its own message template.
     *
     * Internally std::strftime is used from header ctime to create formatted time
     * strings.
     * So for parameter Sink#datetime_pattern you have to use conversion patterns
     * that are recognised by [strftime](http://en.cppreference.com/w/cpp/chrono/c/strftime)
     * For example "%H:%M:%S" returns a 24-hour based time string like 20:12:02
     *
     * @note
     * Sink objects are not exposed to the user directly. You have to use the
     * Logger class to change a sinks settings. Some options for a Sink
     * are only exposed via Logger::init_sink_* methods.
     *
     * @sa
     * ConversionPattern
     */
    Sink(std::string msg_template, std::string datetime_pattern, bool enabled,
         ealogger::constants::LOG_LEVEL min_lvl);
    virtual ~Sink();

    /**
     * @brief Set the message template with conversion patterns
     *
     * @param msg_template
     *
     * @details
     * You can define a message template for every Sink. Every conversion specifier
     * will be substituted with the corresponding information.
     *
     * @sa
     * ConversionPattern
     */
    void set_msg_template(std::string msg_template);
    /**
     * @brief Set the date time conversion pattern
     *
     * @param datetime_pattern
     * @details
     * You can use all conversion patterns that are used by [strftime](http://en.cppreference.com/w/cpp/chrono/c/strftime)
     * The position of the date time information can be specified with ealogger
     * ConversionPattern.
     *
     */
    void set_datetime_pattern(std::string datetime_pattern);
    /**
     * @brief Enable or disable the sink
     *
     * @param enabled
     */
    void set_enabled(bool enabled);
    /**
     * @brief Check if this sink is enabled
     *
     * @return
     */
    bool get_enabled();
    /**
     * @brief Set minimum severity for this sink
     *
     * @param min_lvl
     */
    void set_min_lvl(ealogger::constants::LOG_LEVEL min_lvl);

    /**
     * @brief Prepare a log message before it is written to the targets
     *
     * @param log_message
     *
     * @details
     * Preparing means conversion patterns will be substituted with the
     * corresponding information
     */
    void prepare_log_message(const std::shared_ptr<LogMessage> &log_message);

protected:
    // TODO: I think some of these protected members could be moved to private
    std::string
        msg_template; /**< Message template string consisting of conversion patterns */
    std::string datetime_pattern; /**< Date / time conversion pattern */
    bool enabled;                 /**< Is this Sink enabled  */
    ealogger::constants::LOG_LEVEL
        min_level; /**< Minimum log message severity for this sink */

    // Mutexes for all important members
    std::mutex mtx_msg_template;     /**< Sink#msg_template mutex  */
    std::mutex mtx_datetime_pattern; /**< Sink#datetime_pattern mutex */
    std::mutex mtx_enabled;          /**< Sink#enabled mutex */
    std::mutex mtx_min_lvl;          /**< Sink#min_levek mutex */
    std::mutex mtx_conv_pattern;     /**< Sink#vec_conv_patterns mutex */

    std::vector<ConversionPattern>
        vec_conv_patterns; /**< Vector of conversion patterns a Sink uses*/

    std::map<ealogger::constants::LOG_LEVEL, std::string>
        loglevel_lookup; /**< Lookup table for loglevel Strings */

    /**
     * @brief Fill Sink#vec_conv_patterns with ConverionPattern depending on
     * Sink#msg_template
     */
    void fill_conv_patterns(bool lock);
    /**
     * @brief Writes a LogMessage object to the logger sink
     *
     * @param msg LogMessage object
     * @details
     * This interface method has to be implemented by every logger sink. The sink
     * is required to format the log message according to Sink::msg_template and
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
/**
 * @}
 */
}

#endif /* SINK_H */
