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

#include "logqueue.h"

LogQueue::LogQueue() {}
void LogQueue::push(std::shared_ptr<LogMessage> m)
{
    // acquire the lock on the mutex and push a message object in the queue
    std::lock_guard<std::mutex> lock(this->mtx);
    this->msg_queue.push(m);
    // notify logger thread to wake up and pop latest message
    this->cond_var_queue.notify_one();
}

std::shared_ptr<LogMessage> LogQueue::pop()
{
    std::unique_lock<std::mutex> lock(this->mtx);

    // wait unlocks the acquired lock on this->mtx and puts the thread to sleep
    // until it gets notfied by notify_one(). The wakeup will only occur if the
    // used queue is not empty. After the wakeup the lock is reacquired. Now
    this->cond_var_queue.wait(lock,
                              [this]() { return !this->msg_queue.empty(); });

    std::shared_ptr<LogMessage> lmessage = this->msg_queue.front();
    this->msg_queue.pop();
    return lmessage;
}

bool LogQueue::empty()
{
    std::lock_guard<std::mutex> lock(this->mtx);
    return this->msg_queue.empty();
}
