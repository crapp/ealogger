#ifndef LOGQUEUE_H
#define LOGQUEUE_H

#include <memory>
#include <mutex>
/*
 * We use a std::queue as basis for this threadsafe queue
 */
#include <queue>
/*
 * We need a conditional variable to signal a waiting thread
 */
#include <condition_variable>

#include "logmessage.h"

/**
 * @brief The LogQueue class represents a threadsafe queue we use to store log messages
 * @author Christian Rapp (crapp)
 *
 * A big problem of most logger implementations is performance. If you are logging
 * many messages in a short time your logger may slow down the whole application.
 * Simpleogger can write to a iostream and a fstream at the same time. These
 * operations take time. To minimize the overhead created by writing to streams
 * we are using a threadsafe queue. This queue is filled by the logger and a separate
 * thread object and works with the FIFO principle.
 */
class LogQueue
{
public:
    LogQueue();

    /**
     * @brief Push LogMessage in the Queue.
     * @param m LogMessage object as shared pointer
     */
    void push(std::shared_ptr<LogMessage> m);
    /**
     * @brief Get the next LogMessage object in the Queue und remove it
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
    std::queue<std::shared_ptr<LogMessage> > logQueueInternal;
    /** conditianl variable we use to signal the background thread to wake up and
     * pop a new LogMessage object and route it to the internal message method
     */
    std::condition_variable condLogQueueInternal;
};

#endif // LOGQUEUE_H
