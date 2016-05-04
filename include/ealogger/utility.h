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

#ifndef UTILITY_H
#define UTILITY_H

/**
 * @file utility.h
 * @brief Header with utility functions for ealogger
 */

#ifndef _WIN32
#include <unistd.h>
#endif
#include <string>
#include <algorithm>
#include <regex>
#include <ctime>
// Check for backtrace function
#ifdef __GNUC__
#include <execinfo.h>
#include <cxxabi.h>
#endif

/**
 * @namespace utility
 * @brief Namespace for ealogger utility functions
 */
namespace utility
{
/**
 * @brief Get local hostname
 * @return
 */
inline std::string get_hostname()
{
    std::string hname = "UNKNOWN";
    char hchar[1024] = "\0";
    if (gethostname(hchar, sizeof(hchar)) == 0) {
        hname = std::string(hchar);
    }
    return hname;
}

/**
 * @brief Get the last element of a path
 * @param absolute_path
 * @return Filename
 */
inline std::string get_file_name(const std::string &absolute_path)
{
    const auto pos = absolute_path.find_last_of("/\\");
    return (absolute_path.substr(pos + 1));
}

/**
 * @brief Print a demangled stacktrace
 * @param size How many elements of the stack you wish to be printed.
 *
 * The method only works with gcc/llvm compiled software.
 */
inline void stack_trace(unsigned int size,
                        std::vector<std::string> &stack_msg_vec)
{
#ifdef __GLIBC__
    void *addrlist[size + 1];

    size_t no_of_stack_addresses =
        backtrace(addrlist, sizeof(addrlist) / sizeof(void *));
    char **temp_symbols = backtrace_symbols(addrlist, no_of_stack_addresses);

    // initialize a vector of string with the char**
    std::vector<std::string> symbollist(temp_symbols,
                                        temp_symbols + no_of_stack_addresses);
    // temp_symbols has to be freed
    free(temp_symbols);

    std::regex rgx("(^.*\\()(.*\\+)(0[xX][0-9a-fA-F]+\\))");

    // TODO: This always prints stack_trace as first symbol. Should we omit
    // this?
    for (const auto &symbol : symbollist) {
        std::smatch match;
        // if there is no match with our regex we have to continue and use the
        // original symbol
        if (!std::regex_search(symbol, match, rgx)) {
            stack_msg_vec.push_back(symbol);
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
            stack_msg_vec.emplace_back(file + " : " + std::string(realname) +
                                       "+" + caller);
        } else {
            stack_msg_vec.emplace_back(file + " : " + mangled_name + "()" + "+" +
                                       caller);
        }
        free(realname);
    }
#endif
}

/**
 * @brief Get a formatted time string based on
 * @param time_format
 * @return
 */
inline std::string format_time_to_string(std::time_t t,
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

/**
 * @brief Get a formatted time string based
 * @param time_format
 * @return
 */
inline std::string format_time_to_string(const std::string &time_format)
{
    // get raw time
    time_t rawtime;
    std::time(&rawtime);
    return utility::format_time_to_string(std::move(rawtime), time_format);
}
}

#endif /* ifndef UTILITY_H */
