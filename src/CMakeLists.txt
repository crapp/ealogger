set(SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/logqueue.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/simplelogger.cpp
)

find_package(Threads REQUIRED)

if(UNIX)
    find_path(
        SYSLOG
        syslog.h
    )
    if(${SYSLOG})
        message(STATUS "Syslog sink is available")
    endif()
endif()

#add bin dir because of the configure headers we create later
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    "../include/simplelogger"
)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
    ${CMAKE_SOURCE_DIR}/include/simplelogger/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

#define library with headers and source files
add_library(simplelogger SHARED ${SOURCE} ${HEADERS})
#link with this libraries
target_link_libraries(simplelogger ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS simplelogger DESTINATION lib)
foreach(header ${HEADERS})
#install(FILES ${PROJECT_SOURCE_DIR}/include/simplelogger/simplelogger.h DESTINATION include/simplelogger)
    install(FILES ${header} DESTINATION include/simplelogger)
endforeach(header)

# This must be done after we set the target
if (APPLE AND CMAKE_GENERATOR MATCHES "Xcode")
    message("Setting c++11 properties for XCode project file")
    #in case we produce an XCode project file set the most important attributes.
    SET_TARGET_PROPERTIES(simplelogger PROPERTIES XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    SET_TARGET_PROPERTIES(simplelogger PROPERTIES XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
endif()
