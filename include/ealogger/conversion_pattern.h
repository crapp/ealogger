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

#ifndef CONVERSION_PATTERN_H
#define CONVERSION_PATTERN_H

#include <string>
#include <iostream>

#include "ealogger/global.h"

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
 * %s  :  Log level / severity
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
        MSG,           /**< Log Message */
        LVL            /**< Log level/severity */
    };

    ConversionPattern(std::string conv_pattern, const PATTERN_TYPE ptype)
        : conv_pattern(std::move(conv_pattern)), ptype(ptype){};

    template <typename T>
    /**
     * @brief Replace conversion pattern with new_value
     *
     * @param msg_template Message pattern
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
inline void ConversionPattern::replace_conversion_pattern(
    std::string &msg, std::string new_value) const
{
    // set pos to 0. we use this pos to narrow down our search area
    // std::cout << "conv pattern: " << this->conv_pattern
    //<< " new_value: " << new_value << std::endl;
    size_t pos_in_string = 0;
    while ((pos_in_string = msg.find(this->conv_pattern, pos_in_string)) !=
           std::string::npos) {
        msg.replace(pos_in_string, this->conv_pattern.length(), new_value);
        pos_in_string += new_value.length();
    }
}
#endif /* ifndef CONVERSION_PATTERN_H */
