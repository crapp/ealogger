# ealogger

[![Build Status](https://travis-ci.org/crapp/ealogger.svg?branch=master)](https://travis-ci.org/crapp/ealogger)
[![ealogger License](https://img.shields.io/badge/license-Apache--2.0-blue.svg)](#license)

The **e**asy **a**synchronous logger provides a simple to use yet powerful logging
functionality for C++ applications.
You can use it to log to **console**, to a **file** or **system log (syslog)**.
It was designed to be fast (uses Threads) and flexible. The library registers a
signal handler for SIGUSR1 and is compatible with logrotate (only available on
linux currently). It was tested on Linux, Windows and OS X.

**Table of Contents**  *generated with [DocToc](http://doctoc.herokuapp.com/)*

- [ealogger](#ealogger)
	- [Compilation](#compilation)
	- [Usage](#usage)
		- [Minimum Code example](#minimum-code-example)
		- [Colorized Logfiles with multitail](#colorized-logfiles-with-multitail)
	- [Source Code Documentation](#source-code-documentation)
	- [ToDo](#todo)
	- [FAQ](#faq)
	- [License](#license)

## Setting up ealogger

ealogger source code is hosted on [github.com](https://github.com/crapp/ealogger).
You may either compile the source code or use binary packages.

### Dependencies

Make sure your development environment meets these requirements

* cmake >= 3.4
* gcc >= 4.9
* clang >= 3.5
* MSVC >= 14 (Visual Studio 2015)

### Installation

The Library uses the [CMake](http://cmake.org/) build system. This way you
can easily build it on different platforms.

#### CMake Options

There are some cmake options you can customize for your requirements

* BUILD_TEST (default off) : Setting this to **ON** will compile a tester
  application
* BUILD_SHARED_LIBS (default on): Whether or not to compile as shared library

#### Linux / OS X

Example for building a shared library without debug symbols. We are using an
out of source build here.

```shell
# clone the sources from github
git clone https://github.com/crapp/ealogger.git ealogger
# change directory and create build directory
cd ealogger
mkdir build
cd build
# run cmake from within the build directory
cmake ../ -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
make
```

#### Windows

This shows how to create a Visual Studio Solution with cmake

```shell
# create build directory
mkdir build
cd build
# Please change these options so they suit your build evironment.
cmake -G"Visual Studio 14 2015 Win64" ../
```
Open the solution file with Visual Studio and compile the library.

You may also use [different generators](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html) for other platforms.

## Usage

Usage of the logger is easy. For a start have a look at the tester application.

### Minimum Code example

Here is a small example. First we instantiate a new Logger object, than we print
some messages.

```c++
std::unique_ptr<EALogger> log = std::unique_ptr<EALogger>(
    new EALogger(EALogger::log_level::INFO,
                     true,
                     true,
                     false,
                     "%H:%M:%S",
                     "logToMe.log"));
log->debug("Do you see me?");
log->info("An info message")
log->warn("A warning message");
log->error("An error message");
log->fatal("A fatal message");
```
This will output:
```shell
[17:55:28] INFO: An info message
[17:55:28] WARNING: A warning message
[17:55:28] ERROR: An error message
[17:55:28] FATAL: A fatal message
```

As you can see the DEBUG level message is not printed. This is because of the
minimum severity we set when we created the object.

### Colorized Logfiles using multitail

Logfiles are sometimes difficult to read. So some sort of color
highlighting might be useful. If you are using a Unix system you may try
[multitail](http://www.vanheusden.com/multitail/)

Here is a screenshot how this might look like
![ealogger multitail](http://crapp.github.io/ealogger/screenshots/ealogger_multitail.jpeg "EALogger multitail")

The color scheme for multitail I used to generate the colors in the
screenshot looks like this

    colorscheme:ealogger
    cs_re:,magenta,bold:^.+FATAL:.+
    cs_re:,red,bold:^.+ERROR:.+
    cs_re:,blue:^.+WARNING:.+
    cs_re:,green:^.+INFO:.+

Put it in ~/.multitailrc and start mutlitail
```shell
multitail -cS ealogger mylogfile.log
```
## Source Code Documentation

Is available as [doxygen](http://www.stack.nl/~dimitri/doxygen/) generated html
documentation hosteb by [github.io](https://crapp.github.io/ealogger)

The doxygen project file is located in the
[doc](https://github.com/crapp/ealogger/tree/master/doc) folder and can be used
to generate a version of the documentation on your system.

## Development

ealogger uses [travis ci](https://travis-ci.org/crapp/ealogger) for continuous
integration. Currently we use this service to automatically compile the source
code with each pull request on different platforms and compilers.

### Branches

The github repository of ealogger has several different branches

* master: Main development branch. Everything in here is guaranteed to compile
and is tested (at least a little bit :)). This is the place for new features
and bugfixes. Pull requests welcome.
* dev: Test branch. Code is guaranteed to compile on the developers build
environment. Nothing more nothing less.
* release-x.x: Branch for a release. Only bugfixes are allowed here.
Pull requests welcome.
* gh-pages: Special branch for the static API HTML documentation that will be
hosted by github.io. Content is generated with doxygen.

### Coding standards

The source code is formatted with clang-format using the following configuration

```
Language                            : Cpp,
BasedOnStyle                        : LLVM,
AccessModifierOffset                : -4,
AllowShortIfStatementsOnASingleLine : false,
AlwaysBreakTemplateDeclarations     : true,
ColumnLimit                         : 81,
IndentCaseLabels                    : false,
Standard                            : Cpp11,
IndentWidth                         : 4,
TabWidth                            : 4,
BreakBeforeBraces                   : Linux,
CommentPragmas                      : '(^ IWYU pragma : )|(^.*\[.*\]\(.*\).*$)|(^.*@brief|@param|@return|@throw.*$)|(/\*\*<.*\*/)'
```

### Versioning

I decided to use [semantic versioning](http://semver.org/) and stick to their rules.

> Given a version number MAJOR.MINOR.PATCH, increment the:
>
> 1. MAJOR version when you make incompatible API changes,
>
> 2. MINOR version when you add functionality in a backwards-compatible manner, and
>
> 3. PATCH version when you make backwards-compatible bug fixes.

We are currently at this stage

> Major version zero (0.y.z) is for initial development. Anything may change at any time. The public API should not be considered stable.

## Bugs, feature requests, ideas

Please use the [github bugtracker](https://github.com/crapp/ealogger/issues)
to submit bugs or feature requests

## FAQ

**Yet another logger lib, why?**

I wanted to have a flexible and easy to use logger lib for my c++ projects.
Especially one that makes use of the new c++11 features and uses concurrency.

**Why does it not have feature X?**

Feel free to fork the project and make a pull request!

## License

```
Copyright (C) 2013 - 2016 Christian Rapp

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```
