#include "logqueue.h"

LogQueue::LogQueue()
{
}

void LogQueue::push(std::shared_ptr<LogMessage> m)
{
    std::lock_guard<std::mutex> lock(this->mtx);
    this->logQueueInternal.push(m);
    //notify logger thread to wake up and pop latest message
    this->condLogQueueInternal.notify_one();
}

std::shared_ptr<LogMessage> LogQueue::pop()
{
    std::unique_lock<std::mutex> lock(this->mtx);
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
