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
        /** Normal log message */
        DEFAULT = 0,
        /** Log message with a vector of stack elements */
        STACK
    };

    /**
     * @brief A constant iterator typedef that is internally used
     */
    typedef std::vector<std::string>::const_iterator stackMsgIt;

    /**
     * @brief Initializes a log message object
     * @param severity Is the severity of the message, SimpleLogger#logLevels
     * @param message Message as std::string
     * @param logType LogMessage#LOGTYPE
     */
    LogMessage(int severity, std::string message, LOGTYPE logType) :
        severity(severity), message(message), logType(logType)
    {
        this->t = std::chrono::system_clock::now();
    }
    /**
     * @brief Initializes a log message object
     * @param severity Is the severity of the message, SimpleLogger#logLevels
     * @param messageVec A vector<std::string> containing the stack elements
     * @param logType LogMessage#LOGTYPE
     */
    LogMessage(int severity, std::vector<std::string> messageVec, LOGTYPE logType) :
        severity(severity), messageVec(messageVec), logType(logType)
    {
        this->t = std::chrono::system_clock::now();
        this->message = "";
    }

    /**
     * @brief Returns the severity of the message
     * @return Severity returned as int, SimpleLogger#logLevels
     */
    int getSeverity()
    {
        return this->severity;
    }
    /**
     * @brief Get the log message
     * @return Log message as std::string
     */
    std::string getMessage()
    {
        return this->message;
    }

    /**
     * @brief Get the log message type
     * @return LogMessage#LOGTYPE
     */
    LOGTYPE getLogType()
    {
        return this->logType;
    }

    /**
     * @brief Returns a constant iterator pointing the begin of the message vector
     * @return #stackMsgIt
     */
    stackMsgIt getStackElementsBegin()
    {
        return this->messageVec.cbegin();
    }
    /**
     * @brief Returns a constant iterator pointing the end of the message vector
     * @return #stackMsgIt
     */
    stackMsgIt getStackElementsEnd()
    {
        return this->messageVec.cend();
    }


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

#endif // LOGMESSAGE_H
