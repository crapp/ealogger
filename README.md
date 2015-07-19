#Simplelogger#

This library provides a simple to use yet powerful logging functionality for c++ applications.
You can use it to log to stdout or to a logfile. It was designed to be fast (uses Threads) and flexible.
The library registers a signal handler for SIGUSR1 and is compatible with logrotate (only available on linux currently). It was tested on Linux, Windows and OS X. 

**Table of Contents**  *generated with [DocToc](http://doctoc.herokuapp.com/)*

- [Simplelogger](#simplelogger)
	- [Compilation](#compilation)
	- [Usage](#usage)
		- [Minimum Code example](#minimum-code-example)
		- [Colorized Logfiles with multitail](#colorized-logfiles-with-multitail)
	- [Source Code Documentation](#source-code-documentation)
	- [ToDo](#todo)
	- [FAQ](#faq)
	- [License](#license)


##Compilation##

the Library uses the [CMake](http://cmake.org/) build system. This way you 
can easily build it on different platforms. No special dependencies are
required apart from CMake >= 2.8 and a **c++11** compatible compiler (e.g. gcc >=4.7; 
Visual Studio 2013 (2010 and 2012 are not supported)).

Example for building a shared library without debug symbols. We are using an
out of source build here.
```shell
git clone https://github.com/crapp/simplelogger.git simplelogger
cd simplelogger
mkdir build
cd build
cmake ../ -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
make
sudo make install
```

This will install libsimplelogger to /usr/local/include on Unix systems.
You may also use different generators for other platforms. This is useful
on MacOs or Windows. CMake allows you to generate Visual Studio or XCode
project files in order to build the library.

##Usage##

Usage of this Library is quite simple. We provide a simple tester that you can build with the library.
Use the cmake command line argument **-DBUILD_TEST=ON**.
    
###Minimum Code example###
Here is a small example. First we instantiate a new Logger object, than we print some messages.

```c++
std::unique_ptr<SimpleLogger> log = std::unique_ptr<SimpleLogger>(
    new SimpleLogger(SimpleLogger::logLevels::INFO,
                     true,
                     true,
                     false,
                     false,
                     "%H:%M:%S",
                     "logToMe.log"));
log->writeLog(SimpleLogger::logLevels::DEBUG,
             "Do you see me?");
log->writeLog(SimpleLogger::logLevels::WARNING,
             "A warning message");
log->writeLog(SimpleLogger::logLevels::ERROR,
             "An error message");
log->writeLog(SimpleLogger::logLevels::FATAL,
             "A fatal message");
log->printStackTrace(10);
```
This will output:

    [17:55:28] WARNING: A warning message
    [17:55:28] ERROR: An error message
    [17:55:28] FATAL: A fatal message
    [17:55:28] Stacktrace:
        .../libsimplelogger.so(_ZN12SimpleLogger15printStackTraceEj+0xb1) [0x7fb21db14ce3]
	    .../logtest(main+0x6f6) [0x402cb3]
	    /usr/lib/libc.so.6(__libc_start_main+0xf5) [0x7fb21cd3bb05]
	    .../logtest() [0x4024f9]

As you can see the DEBUG level message is not printed. This is because of the minimum severity
we set when we created the object. At the end we are printing a Stacktrace. c++ methods are not demangeld currently.

###Colorized Logfiles with multitail###
We all know large logfiles are difficult to read. So some sort of color highlighting might be 
useful. If you are using a unix system you may try [multitail](http://www.vanheusden.com/multitail/)

Here is a screenshot how this might look like
![simplelogger multitail](http://crapp.github.io/simplelogger/screenshots/SimpleLoggerMultitail.jpeg "Simplelogger multitail")

The color scheme for multitail I used to generate the above colors in the screenshot looks like this

    colorscheme:simpleLogger
    cs_re:,magenta,bold:^.+FATAL:.+
    cs_re:,red,bold:^.+ERROR:.+
    cs_re:,blue:^.+WARNING:.+
    cs_re:,green:^.+INFO:.+

Put it in ~/.multitailrc and start mutlitail
```shell
multitail -cS simpleLogger mylogfile.log
```
##Source Code Documentation##

Is available as doxygen generated html documentation. The doxygen project file is located in the  [doc](https://github.com/crapp/simplelogger/tree/master/doc) folder. You may use it to generate the documentation.

##ToDo##

* Print stacktrace method does not demangle c++ method names. So the stacktrace is not so easy to read. I found a solution on the net and will implement it soon.
* Printing stacktrace does only work with gcc/llvm. There is a possible solution for Windows though.
* Support for other logrotation services like newsyslog or LogRotateWin

##FAQ##

**Yet another logger lib, why?**
I wanted to have a flexible and easy to use logger lib for my c++ projects. Especially one that makes use of the new c++11 features.
**Why does it not have feature X?**
Feal free to fork the project and do a pull request!

##License##

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
