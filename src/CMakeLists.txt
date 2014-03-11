#comment this if you want to compile with gcc
set(APPLE_USE_CLANG "TRUE")
set(APPLE_USE_XCODE "TRUE")

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

set(SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/simplelogger.cpp
)

#set(HEADERS
#    ${CMAKE_CURRENT_SOURCE_DIR}/helpers.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/simplelogger.h
#)

#add bin dir because of the configure headers we create later
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    "../include/simplelogger"
)

# configure a header file to pass some of the CMake settings
# to the source code
#configure_file (
#    ${CMAKE_CURRENT_SOURCE_DIR}/simpleloggerversion.h.in
#    ${CMAKE_CURRENT_BINARY_DIR}/simpleloggerversion.h
#)

#define library with headers and source files
add_library(simplelogger SHARED ${SOURCE} ${HEADERS})
#link with this libraries
#target_link_libraries(simplelogger)

install(TARGETS simplelogger DESTINATION lib)
foreach(header ${HEADERS})
    message(${header})
    install(FILES ${header} DESTINATION include/simplelogger)
endforeach(header) 
#install(FILES ${HEADERS} DESTINATION include/simplelogger)

#this can be done after we set our target
#FIXME check ${CMAKE_GENERATOR} for used generator and remove APPLE_USE_XCODE
if (APPLE AND APPLE_USE_XCODE)
    message("Setting c++11 properties for XCode project file")
    #in case we produce an XCode project file set the most important attributes.
    SET_TARGET_PROPERTIES(simplelogger PROPERTIES XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    SET_TARGET_PROPERTIES(simplelogger PROPERTIES XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
endif()
