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
        DEFAULT = 0, /** A normal log message */
        STACK /** A log message with a vector of stack elements */
    };

    /**
     * @brief A constant iterator typedef that is internally used
     */
    typedef std::vector<std::string>::const_iterator stackMsgIt;

    /**
     * @brief Initializes a log message object
     * @param severity Is the severity of the message
     * @param message Message as std::string
     * @param logType @see LOGTYPE
     */
    LogMessage(int severity, std::string message, LOGTYPE logType) :
        severity(severity), message(message), logType(logType)
    {
        this->t = std::chrono::system_clock::now();
    }
    /**
     * @brief Initializes a log message object
     * @param severity Is the severity of the message
     * @param messageVec A vector<std::string> containing the stack elements
     * @param logType @see LOGTYPE
     */
    LogMessage(int severity, std::vector<std::string> messageVec, LOGTYPE logType) :
        severity(severity), messageVec(messageVec), logType(logType)
    {
        this->t = std::chrono::system_clock::now();
        this->message = "";
    }

    /**
     * @brief Returns the severity of the message
     * @return Severity returned as int
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
     * @return @see LOGTYPE
     */
    LOGTYPE getLogType()
    {
        return this->logType;
    }

    /**
     * @brief Returns a constant iterator pointing the begin of the message vector
     * @return @see stackMsgIt
     */
    stackMsgIt getStackElementsBegin()
    {
        return this->messageVec.cbegin();
    }
    /**
     * @brief Returns a constant iterator pointing the end of the message vector
     * @return @see stackMsgIt
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
    /** The log message type */
    LOGTYPE logType;
    /** A vector of stack elements */
    std::vector<std::string> messageVec;
};

#endif // LOGMESSAGE_H
