#include "logqueue.h"

LogQueue::LogQueue()
{
}

void LogQueue::push(std::shared_ptr<LogMessage> m)
{
    // acquire the lock on the mutex and push a message object in the queue
    std::lock_guard<std::mutex> lock(this->mtx);
    this->logQueueInternal.push(m);
    // notify logger thread to wake up and pop latest message
    this->condLogQueueInternal.notify_one();
}

std::shared_ptr<LogMessage> LogQueue::pop()
{
    std::unique_lock<std::mutex> lock(this->mtx);

    // wait unlocks the acquired lock on this->mtx and puts the thread to sleep
    // until it gets notfied by notify_one(). The wakeup will only occur if the
    // used queue is not empty. After the wakeup the lock is reacquired. Now
    this->condLogQueueInternal.wait(lock, [this] () {
        return !this->logQueueInternal.empty();
    });

    std::shared_ptr<LogMessage> LogMessage = this->logQueueInternal.front();
    this->logQueueInternal.pop();
    return LogMessage;
}

bool LogQueue::empty()
{
    std::lock_guard<std::mutex> lock(this->mtx);
    return this->logQueueInternal.empty();
}
