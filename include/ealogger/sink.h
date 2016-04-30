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

#include "utility.h"
#include "logmessage.h"

/**
 * @brief A conversion pattern for message strings
 *
 * @details
 * ealogger supports several conversion patterns that let you substitude a pattern
 * within a string with a appropriate value. This way the style of the log messages
 * can be set for each log sink. Currently we support these conversion patterns
 *
 * %d  :  Date Time as provided
 * %f  :  Name of the file from where a log message was issued
 * %F  :  Absolut path of a file as provided by __FILE__ from where a log mesage was issued
 * %l  :  Line number of the file from where a log message was issued (__LINE__)
 * %fu :  Name of the function from where a log message was issued (__func__)
 * %h  :  Hostname
 * %t  :  Thread ID
 * %m  :  Log message
 *
 */
struct ConversionPattern {
public:
    enum PATTERN_TYPE {
        DT = 0,        /**< Datetime Pattern */
        FILE,          /**< Caller Filename */
        FILE_ABSOLUTE, /**< Caller absolute filename */
        LINE,          /**< Line number */
        FUNC,          /**< Caller Function */
        HOST,          /**< Hostname */
        THREADID,      /**< Thread ID */
        MSG            /**< Log Message */
    };

    ConversionPattern(std::string conv_pattern, const PATTERN_TYPE ptype)
        : conv_pattern(std::move(conv_pattern)), ptype(ptype){};

    template <typename T>
    /**
     * @brief Replace conversion pattern with new_value
     *
     * @param msg_pattern Message pattern
     * @param new_value Value to replace the conversion pattern with
     */
    void replace_conversion_pattern(std::string &msg, T new_value) const
    {
        this->replace_conversion_pattern(msg, std::to_string(new_value));
    }

    ConversionPattern::PATTERN_TYPE get_pattern_type() const
    {
        return this->ptype;
    }

private:
    std::string conv_pattern;
    const PATTERN_TYPE ptype;
    /**
     * @brief Some conversion patterns need additional data
     *
     * @param cp Conversion Pattern
     *
     * @return Value that will be used to replace the conversion pattern
     */
    void check_conversion_pattern(const std::string &cp, std::string &new_value)
    {
        if (cp == "%f") {
            new_value = utility::get_file_name(new_value);
        }
        if (cp == "%h") {
            new_value = utility::get_hostname();
        }
    }
};
template <>
/**
 * @brief Replace conversion pattern with new_value
 *
 * @param msg The message pattern
 * @param new_value New value
 * @details
 *
 * This method is a specialization of ConversionPattern::replace_conversion_pattern(std::string, T)
 * Here the actual replacement of all instances of the conversion pattern take
 * place.
 */
void ConversionPattern::replace_conversion_pattern(std::string &msg,
                                                   std::string new_value) const
{
    // set pos to 0. we use this pos to narrow down our search area
    size_t pos_in_string = 0;
    while ((pos_in_string = msg.find(this->conv_pattern, pos_in_string)) !=
           std::string::npos) {
        msg.replace(pos_in_string, this->conv_pattern.length(), new_value);
        pos_in_string += new_value.length();
    }
}

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
     *
     * @param msg_pattern The message pattern/style for a sink
     * @param datetime_pattern The datetime pattern
     * @param enabled Whether or not the sink is enabled
     */
    Sink(std::string msg_pattern, std::string datetime_pattern, bool enabled);
    virtual ~Sink();

    void prepare_log_message(const std::shared_ptr<LogMessage> &log_message);
    void set_msg_pattern(std::string msg_pattern);
    std::string get_msg_pattern();
    void set_datetime_pattern(std::string datetime_pattern);
    std::string get_datetime_pattern();
    void set_enabled(bool enabled);
    bool get_enabled();

protected:
    std::mutex mtx_msg_pattern;
    std::mutex mtx_datetime_pattern;
    std::mutex mtx_enabled;

    std::string msg_pattern;
    std::string datetime_pattern;
    bool enabled;

    std::vector<ConversionPattern> vec_conv_patterns;

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
};

#endif /* SINK_H */
