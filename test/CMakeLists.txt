set(TESTSOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

set(TESTHEADER)

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

if(BUILD_TEST)
    message(STATUS "Will build the tester application for SimpleLogger")
    #add_executable(logtest EXCLUDE_FROM_ALL ${TESTSOURCE})
    add_executable(logtest ${TESTSOURCE} ${TESTHEADER})
    target_link_libraries(logtest simplelogger)
    #this can be done after we set our target
    #FIXME check ${CMAKE_GENERATOR} for used generator and remove APPLE_USE_XCODE
    if (APPLE AND APPLE_USE_XCODE)
        message("Setting c++11 properties for XCode project file")
        #in case we produce an XCode project file set the most important attributes.
        SET_TARGET_PROPERTIES(logtest PROPERTIES XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
        SET_TARGET_PROPERTIES(logtest PROPERTIES XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
    endif()
endif(BUILD_TEST)
