# Function for generation of export macros for libraries
#include(GenerateExportHeader)

set(EALOGGER_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/ealogger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/logqueue.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sink.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sink_console.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sink_file.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sink_syslog.cpp
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(UNIX)
    find_path(
        EALOGGER_SYSLOG
        syslog.h
    )
    if(${EALOGGER_SYSLOG})
        message(STATUS "Syslog sink is available")
    endif()
endif()

#add bin dir because of the configure headers we create later
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    "../include"
)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
    ${CMAKE_SOURCE_DIR}/include/ealogger/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

#define library with headers and source files
add_library(ealogger ${EALOGGER_SOURCE} ${EALOGGER_HEADER})
#link with this libraries
target_link_libraries(ealogger Threads::Threads)

# set c++ standard for target
set_property(TARGET ealogger PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ealogger PROPERTY CXX_STANDARD 11)
#generate_export_header(ealogger)

install(TARGETS ealogger DESTINATION lib)
foreach(header ${EALOGGER_HEADER})
    install(FILES ${header} DESTINATION include/ealogger)
endforeach(header)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/config.h DESTINATION include/ealogger)


if(${BUILD_UNIT_TEST})
    message(STATUS "Will build unit tester for ealogger")
    add_subdirectory(test)
endif()
