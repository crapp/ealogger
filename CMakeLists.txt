cmake_minimum_required(VERSION 3.4)
project(ealogger)

# Version Number
set(ealogger_VERSION_MAJOR 0)
set(ealogger_VERSION_MINOR 7)
set(ealogger_VERSION_PATCH 0)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

option(BUILD_EXAMPLES "Build example and benchmark applications for ealogger" OFF)
option(BUILD_UNIT_TEST "Build a unit test application based on Catch" OFF)
option(PRINT_INTERNAL_MESSAGES "Print messages with INTERNAL priority. Only usefull for ealogger developers." OFF)
option(BUILD_SHARED_LIBS "Build shared library" ON)

include(CheckCXXCompilerFlag)
include(CheckCXXSymbolExists)

# Initialize CXXFLAGS for Linux, OS X and MinGW on Windows
if (NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
    set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g")
    set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG} -Wextra")
    set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
    CHECK_CXX_COMPILER_FLAG("-rdynamic" COMPILER_SUPPORTS_RDYNAMIC)
    if (COMPILER_SUPPORTS_RDYNAMIC)
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -rdynamic")
    endif(COMPILER_SUPPORTS_RDYNAMIC)
endif(NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")

# TODO: I should use cmake makros and try_compile to make sure compiler supports
# what we need

set(EALOGGER_HAVE_DECL_GETTIME 1)

# Compiler-specific C++11 activation.
if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
        message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.9 or greater.")
    endif ()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5)
        message(STATUS "Your version of gcc does not implement std::get_time. Will search for std::strptime")
        CHECK_CXX_SYMBOL_EXISTS(strptime "ctime" EALOGGER_HAVE_DECL_STRPTIME)
        set(EALOGGER_HAVE_DECL_GETTIME 0)
    endif ()
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.3)
        message(FATAL_ERROR "${PROJECT_NAME} requires clang 3.3 or greater.")
    endif()
    if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    if (${MSVC_VERSION} LESS 1800)
        message(
          FATAL_ERROR
          "This Project does not support compiling on Windows with "
          "MSVC ${CMAKE_CXX_COMPILER_ID}. Please use MSVC 12 or higher.")
    endif ()
else ()
    message(FATAL_ERROR "Your C++ compiler does not support C++11.")
endif ()

# check if compiler can parse time
set(EALOGGER_CAN_PARSE_TIME 0)
if (EALOGGER_HAVE_DECL_STRPTIME OR EALOGGER_HAVE_DECL_GETTIME)
    set(EALOGGER_CAN_PARSE_TIME 1)
endif()

add_subdirectory(include/ealogger)
add_subdirectory(src)
add_subdirectory(examples)
