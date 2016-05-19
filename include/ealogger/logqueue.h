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

#ifndef LOGQUEUE_H
#define LOGQUEUE_H

#include <memory>
#include <mutex>
/*
 * We use a std::queue as basis for this threadsafe queue
 */
#include <queue>
/*
 * We need a conditional variable to notify a waiting thread
 */
#include <condition_variable>

#include <ealogger/logmessage.h>

namespace ealogger
{
/**
 * @brief The LogQueue class represents a threadsafe queue ealogger uses to store
 * log messages
 * @author Christian Rapp (crapp)
 *
 * A big problem of most logger implementations is performance. If you are logging
 * many messages in a short time your logger may slow down the whole application.
 * ealogger can write to an iostream, a fstream or a syslog sink at the same time. These
 * operations take time. To minimize the overhead created by writing to a stream
 * we are using a threadsafe queue. This queue is filled by the logger and a separate
 * thread object and works with the FIFO principle and pops the messages from the
 * queue and hands them over to the sinks.
 *
 * Please note this is _not_ a lock free solution so far.
 */
class LogQueue
{
public:
    /**
     * @brief LogQueue constructor
     */
    LogQueue();

    /**
     * @brief Push LogMessage in the Queue.
     * @param m LogMessage object as shared pointer
     */
    void push(std::shared_ptr<LogMessage> m);
    /**
     * @brief Get the next LogMessage object in the Queue and remove it
     * @return Shared pointer LogMessage object
     */
    std::shared_ptr<LogMessage> pop();
    /**
     * @brief Check if the Queue is empty
     * @return True if it is empty, otherwise false
     */
    bool empty();

private:
    /** The Mutex that makes the Queue threadsafe */
    std::mutex mtx;
    std::queue<std::shared_ptr<LogMessage>> msg_queue;
    /**
     * conditional variable we use to signal the background thread to wake up and
     * pop a new LogMessage object and route it to the internal message method
     */
    std::condition_variable cond_var_queue;
};
}

#endif  // LOGQUEUE_H
