/*  This is a simple yet powerful logger library for c++
    Copyright (C) 2013 - 2015 Christian Rapp

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
 * Simplelogger can write to a iostream, a fstream and a syslog sink at the same time. These
 * operations take time. To minimize the overhead created by writing to a stream
 * we are using a threadsafe queue. This queue is filled by the logger and a separate
 * thread object and works with the FIFO principle. Please note this is _not_ a lock free
 * solution so far.
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
    std::queue<std::shared_ptr<LogMessage>> logQueueInternal;
    /** conditional variable we use to signal the background thread to wake up and
     * pop a new LogMessage object and route it to the internal message method
     */
    std::condition_variable condLogQueueInternal;
};

#endif  // LOGQUEUE_H
