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
#include <chrono>
#include <sstream>
#include <string>

#include "ealogger.h"
#include "config.h"

int main(int argc, const char* argv[])
{
	std::unique_ptr<EALogger> log = std::unique_ptr<EALogger>(
		new EALogger(EALogger::logLevels::INFO, true, true, false, false, true,
					 "%H:%M:%S", "logToMe.log"));
	std::chrono::steady_clock::time_point t = std::chrono::steady_clock::now();

	std::stringstream version;
	version << VERSION_MAJOR << "." << VERSION_MINOR;
	if (std::string(VERSION_PATCH) != "0") {
		version << "." << VERSION_PATCH;
	}

	log->writeLog(EALogger::logLevels::INFO,
				  "Logtester of ealogger " + version.str() + " is starting");
	log->writeLog(EALogger::logLevels::INFO,
				  "Next message has lower severity than defined minimum");

	log->writeLog(EALogger::logLevels::DEBUG, "Do you see me?");
	log->writeLog(EALogger::logLevels::WARNING, "A warning message");
	log->writeLog(EALogger::logLevels::ERROR, "An error message");
	log->writeLog(EALogger::logLevels::FATAL, "A fatal message");

	log->setLogToSTDOUT(false);
	log->writeLog(EALogger::logLevels::INFO, "Logging to stdout deactivated");
	log->setLogToFile(false);
	log->writeLog(EALogger::logLevels::INFO, "Logging to file deactivated");
	log->setLogToSTDOUT(true);
	log->writeLog(EALogger::logLevels::INFO, "Logging to stdout reactivated");
	log->setLogToFile(true);
	log->writeLog(EALogger::logLevels::INFO, "Logging to file reactivated");

	log->writeLog(EALogger::logLevels::INFO,
				  "Changing date time format specifiers to %A %r");
	log->setDateTimeFormat("%A %r");
	log->writeLog(EALogger::logLevels::INFO, "Format specifiers changed");
	log->printStackTrace(10);

	int msNormal = std::chrono::duration_cast<std::chrono::microseconds>(
					   std::chrono::steady_clock::now() - t)
					   .count();
	log->writeLog(
		EALogger::logLevels::INFO,
		"Logger needed " + std::to_string(msNormal) + "µs to do the logging");

	std::this_thread::sleep_for(std::chrono::milliseconds(3000));

	// now we try the same in multithreading mode
	log = std::unique_ptr<EALogger>(new EALogger(EALogger::logLevels::INFO, true,
												 true, false, true, true,
												 "%H:%M:%S", "logToMe.log"));
	t = std::chrono::steady_clock::now();

	log->writeLog(EALogger::logLevels::INFO, "Logtester is starting");
	log->writeLog(EALogger::logLevels::INFO,
				  "Next message has lower severity than defined minimum");

	log->writeLog(EALogger::logLevels::DEBUG, "Do you see me?");
	log->writeLog(EALogger::logLevels::WARNING, "A warning message");
	log->writeLog(EALogger::logLevels::ERROR, "An error message");
	log->writeLog(EALogger::logLevels::FATAL, "A fatal message");

	log->setLogToSTDOUT(false);
	log->writeLog(EALogger::logLevels::INFO, "Logging to stdout deactivated");
	log->setLogToFile(false);
	log->writeLog(EALogger::logLevels::INFO, "Logging to file deactivated");
	log->setLogToSTDOUT(true);
	log->writeLog(EALogger::logLevels::INFO, "Logging to stdout reactivated");
	log->setLogToFile(true);
	log->writeLog(EALogger::logLevels::INFO, "Logging to file reactivated");

	log->writeLog(EALogger::logLevels::INFO,
				  "Changing date time format specifiers to %A %r");
	log->setDateTimeFormat("%A %r");
	log->writeLog(EALogger::logLevels::INFO, "Format specifiers changed");
	log->printStackTrace(10);

	int msMulti = std::chrono::duration_cast<std::chrono::microseconds>(
					  std::chrono::steady_clock::now() - t)
					  .count();
	log->writeLog(EALogger::logLevels::INFO,
				  "Logger needed " + std::to_string(msMulti) +
					  "µs to do the logging in multithreading mode");

	log->writeLog(EALogger::logLevels::INFO,
				  "The Tester will now write to the System Log. This only works "
				  "on Linux/BSD.");
	std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	log->setLogToSyslog(true);
	log->setLogToFile(false);
	log->setLogToSTDOUT(false);

	log->writeLog(EALogger::logLevels::INFO, "Hello World Syslog");
	log->writeLog(EALogger::logLevels::DEBUG, "This is a debug message");
	log->writeLog(EALogger::logLevels::INFO, "This is an info message");
	log->writeLog(EALogger::logLevels::WARNING, "This is a warning message");
	log->writeLog(EALogger::logLevels::ERROR, "This is an error message");
	log->writeLog(EALogger::logLevels::FATAL, "This is a fatal message");
	log->printStackTrace(10);

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	log->setLogToFile(true);
	log->setLogToSTDOUT(true);
	log->setLogToSyslog(false);

	log->writeLog(EALogger::logLevels::INFO,
				  "ealogger tester finished, bye bye");
	return 0;
}
