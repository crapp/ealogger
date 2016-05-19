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

#include <chrono>
#include <memory>
#include <string>

#include "ealogger/ealogger.h"

int main(void)
{
    namespace eal = ealogger;
    namespace con = ealogger::constants;

    // init an ealogger object and a file sink
    std::unique_ptr<eal::EALogger> log =
        std::unique_ptr<eal::EALogger>(new eal::EALogger(true));
    log->init_file_sink();

    // take the time
    std::chrono::system_clock::time_point t = std::chrono::system_clock::now();
    // log 1000000 messages
    for (int i = 0; i < 1000000; i++) {
        log->info("Hello Afrika - Tell me how you're doin'! ");
    }
    // end time. this is to calculate how long it took ealogger to creat LogMessage
    // objects and push them on a queue
    std::chrono::system_clock::time_point tstop =
        std::chrono::system_clock::now();

    // wait until all messages are written to the logfile
    while (!log->queue_empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // get a timepoint so you know how long it actually took to write the
    // messages
    // to the file
    std::chrono::system_clock::time_point tstop_empty =
        std::chrono::system_clock::now();

    // print results.
    std::cout << "Time in milliseconds to put messages on a queue: "
              << std::chrono::duration_cast<std::chrono::microseconds>(
                     tstop -
                     t).count() /
                     1000
              << "ms" << std::endl;
    std::cout << "Time untill all messages were written to the logfile: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     tstop_empty - t)
                     .count()
              << "ms" << std::endl;
    return 0;
}
