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

#include <ctime>
#include <iomanip>
#include <sstream>

#include "catch.hpp"
#include "config.h"
#include "ealogger/utility.h"

namespace ut = ealogger::utility;

TEST_CASE("Get filename from path", "[utility]")
{
    SECTION("File name without path")
    {
        std::string path = "this_is_the_file.notype";
        REQUIRE(ut::get_file_name(path) == path);
    }

    SECTION("File name with path")
    {
        std::string path = "/somewhere/on/the//disk/should/be.log";
        REQUIRE(ut::get_file_name(path) == "be.log");
        std::string path_win = "C:\\oh\\look\\a\\windows\\path\\log.log";
        REQUIRE(ut::get_file_name(path_win) == "log.log");
    }

    SECTION("A complicated path with lots of culprits")
    {
        std::string path =
            "umlautsüäö/white space/other_(things)_!?_/log file name.log";
        REQUIRE(ut::get_file_name(path) == "log file name.log");
    }
}

#ifdef EALOGGER_CAN_PARSE_TIME
TEST_CASE("Formatted time string", "[utility]")
{
    std::tm t = {};
    std::stringstream ss;
    SECTION("Test some past date and see if we get back what we want")
    {
        ss << "2011-01-01";
#ifdef EALOGGER_HAVE_DECL_GETTIME
        ss >> std::get_time(&t, "%Y-%m-%d");
#endif
#ifdef EALOGGER_HAVE_DECL_STRPTIME
        // TODO: Should check return value of strptime
        strptime(ss.str().c_str(), "%Y-%m-%d", &t);
#endif
        std::string formatted_time =
            ut::format_time_to_string(std::mktime(&t), "%Y-%m-%d");
        REQUIRE(formatted_time == "2011-01-01");
    }
}
#endif
