cmake_minimum_required(VERSION 3.0)
project(ealogger)

# Version Number
set(ealogger_VERSION_MAJOR 0)
set(ealogger_VERSION_MINOR 7)
set(ealogger_VERSION_PATCH 0)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

option(BUILD_TEST "Build tester for ealogger" OFF)
option(PRINT_INTERNAL_MESSAGES "Print messages with INTERNAL priority. Only usefull for ealogger developers." OFF)
option(BUILD_SHARED_LIBS "Build shared library" ON)

# Initialize CXXFLAGS for Linux, OS X and MinGW on Windows
if (NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
    set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g")
    set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG} -Wextra")
    set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
endif(NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")

# Compiler-specific C++11 activation.
if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
    if (NOT (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7))
        message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.7 or greater.")
    endif ()
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC" AND ${MSVC_VERSION} LESS 1800)
    message(
      FATAL_ERROR
      "This Project does not support compiling on Windows with "
      "MSVC ${CMAKE_CXX_COMPILER_ID}. Please use MSVC 12 or higher.")
else ()
    message(FATAL_ERROR "Your C++ compiler does not support C++11.")
endif ()

add_subdirectory(include/ealogger)
add_subdirectory(src)
add_subdirectory(test)
