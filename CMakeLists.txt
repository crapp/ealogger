cmake_minimum_required(VERSION 2.8)
project(ealogger)

# Version Number
set(${PROJECT_NAME}_VERSION_MAJOR 0)
set(${PROJECT_NAME}_VERSION_MINOR 6)
set(${PROJECT_NAME}_VERSION_PATCH 0)

option(BUILD_TEST "Build tester for ealogger" OFF)
option(PRINT_INTERNAL_MESSAGES "Print messages with INTERNAL priority. Only usefull for ealogger developers." OFF)

if(UNIX)
    message("Detected unix system")
    # Initialize CXXFLAGS.
    set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
    set(CMAKE_CXX_FLAGS_DEBUG          "-Wextra -O0 -g")
    set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")


    # Compiler-specific C++11 activation.
    if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
        execute_process(
            COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
        if (NOT (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7))
            message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.7 or greater.")
        endif ()
    elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    else ()
        message(FATAL_ERROR "Your C++ compiler does not support C++11.")
    endif ()
endif(UNIX)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    message("Detected Windows")
endif()


add_subdirectory(include/ealogger)
add_subdirectory(src)
add_subdirectory(test)
