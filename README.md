#Simple Logger#

This library provides a very simple logging funtionality for c++ applications. 
You can use it to log to stdout and to a logfile. The library registers a signal 
handler for SIGUSR1 and is compatible with logrotate.


##Compilation##

the Library uses the [CMake](http://cmake.org/) build system. This way you 
can easily build it on different platforms. No special dependencies are
required apart from CMake >= 2.8 and a **c++11** compatible compiler.

Example for building a shared library without debug symbols. We are using an
out of source build here.
```shell
git clone https://github.com/crapp/simplelogger.git simplelogger
cd simplelogger
mkdir build
cd build
cmake ../ -G "Unix Makefiles" -DCMAKE_RELEASE_TYPE=Release
make
sudo make install
```

This will install libsimplelogger to /usr/local/include on Unix systems.
You may also use different generators for other platforms. This is useful
on MacOs or Windows. CMake allows you to generate Visual Studio or XCode
project files in order to build the library.

##Usage##

Usage of this Library is quite simple. Please refer to the html documentation of
the library located in the [doc](https://github.com/crapp/simplelogger/tree/master/doc) folder.

##License##

    Copyright (C) 2013, 2014 Christian Rapp

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
