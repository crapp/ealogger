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

/**
 * @brief A sink is an object that writes the log message to the target
 * @author Christian Rapp
 * @details
 *
 * The virtual class Sink has to be implemented by each possible target. To add
 * a new Sink to ealogger you have to provide an implementaion of Sink::AJKSDHAJ.
 */
class Sink
{
public:
    Sink();
    virtual ~Sink();

    virtual void write_message(const std::string &msg) = 0;

private:
};

#endif /* SINK_H */
