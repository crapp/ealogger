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

#include <memory>
#include <string>
#include <chrono>

#include "ealogger/ealogger.h"

int main(void)
{
    namespace con = ealogger_constants;

    std::unique_ptr<EALogger> log =
        std::unique_ptr<EALogger>(new EALogger(true));
    log->init_file_sink();

    std::chrono::system_clock::time_point t = std::chrono::system_clock::now();
    for (int i = 0; i < 1000000; i++) {
        log->info("Hello Africa, tell me how you are doing " +
                  std::to_string(i));
    }
    std::chrono::system_clock::time_point tstop =
        std::chrono::system_clock::now();

    while (!log->queue_empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "Time messages on queue: "
              << std::chrono::duration_cast<std::chrono::microseconds>(tstop - t)
                         .count() /
                     1000
              << "ms" << std::endl;
    std::cout << "Time till end: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::system_clock::now() - t)
                     .count()
              << "ms" << std::endl;
    return 0;
}
