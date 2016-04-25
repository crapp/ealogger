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

#ifndef LOGMESSAGE_H
#define LOGMESSAGE_H

#include <string>
#include <chrono>
#include <vector>

struct LogMessage {
public:
    /**
     * @brief The LOGTYPE enum stands for the LogMessage type
     */
    enum LOGTYPE {
        DEFAULT = 0, /**< Normal log message */
        STACK        /**< Log message with a vector of stack elements */

    };

    /**
     * @brief A constant iterator typedef that is internally used
     */
    typedef std::vector<std::string>::const_iterator stackMsgIt;

    /**
     * @brief Initializes a log message object
     * @param severity Is the severity of the message, EALogger#logLevels
     * @param message Message as std::string
     * @param logType LogMessage#LOGTYPE
     */
    LogMessage(int severity, std::string message, LOGTYPE logType)
        : severity(severity), message(message), logType(logType)
    {
        this->t = std::chrono::system_clock::now();
    }
    /**
     * @brief Initializes a log message object
     * @param severity Is the severity of the message, EALogger#logLevels
     * @param messageVec A vector<std::string> containing the stack elements
     * @param logType LogMessage#LOGTYPE
     */
    LogMessage(int severity, std::vector<std::string> messageVec,
               LOGTYPE logType)
        : severity(severity), messageVec(messageVec), logType(logType)
    {
        this->t = std::chrono::system_clock::now();
        this->message = "";
    }

    /**
     * @brief Returns the severity of the message
     * @return Severity returned as int, EALogger#logLevels
     */
    int getSeverity() { return this->severity; }
    /**
     * @brief Get the log message
     * @return Log message as std::string
     */
    std::string getMessage() { return this->message; }
    /**
     * @brief Get the log message type
     * @return LogMessage#LOGTYPE
     */
    LOGTYPE getLogType() { return this->logType; }
    /**
     * @brief Returns a constant iterator pointing the begin of the message vector
     * @return #stackMsgIt
     */
    stackMsgIt getStackElementsBegin() { return this->messageVec.cbegin(); }
    /**
     * @brief Returns a constant iterator pointing the end of the message vector
     * @return #stackMsgIt
     */
    stackMsgIt getStackElementsEnd() { return this->messageVec.cend(); }
private:
    /** Time Point when this log message was created*/
    std::chrono::system_clock::time_point t;
    /** Severity of this message */
    int severity;
    /** The log message */
    std::string message;
    /** A vector of stack elements */
    std::vector<std::string> messageVec;
    /** The log message type */
    LOGTYPE logType;
};

#endif  // LOGMESSAGE_H
