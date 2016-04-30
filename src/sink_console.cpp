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

#include "sink_console.h"

SinkConsole::SinkConsole(std::string msg_pattern, std::string dateime_pattern,
                         bool enabled)
    : Sink(std::move(msg_pattern), std::move(dateime_pattern), enabled)
{
}
SinkConsole::~SinkConsole() {}
void SinkConsole::write_message(const std::string &msg)
{
    std::cout << msg << std::endl;
}
