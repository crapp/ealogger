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

#ifndef GLOBAL_H
#define GLOBAL_H

namespace ealogger_constants
{
/**
 * @brief Sinks ealogger supports
 */
enum LOGGER_SINK { CONSOLES = 0, SYSLOGS, FILES };
enum CONVERSION_PATTERN {};

/**
 * @brief An enumaration representing the supported loglevels.
 *
 * This enum is used to define the severity of a log message and to set the
 * minimum loglevel.
 */
enum LOG_LEVEL {
    DEBUG = 0, /**< Debug message */
    INFO,      /**< Info message */
    WARNING,   /**< Warning message */
    ERROR,     /**< Error message */
    FATAL,     /**< Fatal Message */
    STACK,     /**< Stack log message */
    INTERNAL   /**< Internal Message, do not use this loglevel yourself */
};
}

#endif /* ifndef GLOBAL_H */
