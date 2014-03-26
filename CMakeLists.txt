cmake_minimum_required(VERSION 2.8)
project(SimpleLogger)

# Version Number
set(${PROJECT_NAME}_VERSION_MAJOR 0)
set(${PROJECT_NAME}_VERSION_MINOR 5)
set(${PROJECT_NAME}_VERSION_PATCH 0)

option(APPLE_USE_CLANG OFF)
option(APPLE_USE_XCODE ON)
option(BUILD_TEST "Build tester for simplelogger" OFF)

#CMAKE_CXX_FLAGS will be used for any build type

if(APPLE)
    message("Detected mac os")
    if(NOT APPLE_USE_XCODE)
        if(APPLE_USE_CLANG)
            message("Setting compiler flags for clang")
            #Only use this if cmake found the wrong compiler
            set(CMAKE_CXX_COMPILER "/usr/bin/c++")
            set(CMAKE_C_COMPILER "/usr/bin/cc")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -mmacosx-version-min=10.8")
        else()
            message("Setting compiler flags for gcc")
            #Only use this if cmake found the wrong compiler
            #c++ compiler
            set (CMAKE_CXX_COMPILER "/opt/local/bin/g++-mp-4.8")
            #c compiler
            set (CMAKE_C_COMPILER "/opt/local/bin/gcc-mp-4.8")
            #we use c++11
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
        endif(APPLE_USE_CLANG)
    endif(APPLE_USE_XCODE)
    #extra compiler messages
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wextra -g -Wall")
    set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
else()
    if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        message("Detected Linux")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
        #extra compiler messages
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wextra -g -Wall")
        set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
        set(CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
    endif()
    if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        message("Detected Windows")
    endif()
endif(APPLE)

add_subdirectory(include/simplelogger)
add_subdirectory(src)
add_subdirectory(test)
