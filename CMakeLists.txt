cmake_minimum_required(VERSION 2.8.10)

# Options. Turn on with 'cmake -Dmyvarname=ON'.
option(BUILD_UNIT_TESTS "Whether to build unit tests" OFF)

# Make PROJECT_SOURCE_DIR, PROJECT_BINARY_DIR, and PROJECT_NAME available.
set(PROJECT_NAME "yamq")
project(${PROJECT_NAME})

set(CMAKE_C_FLAGS "-g -O2 -Wall -fPIC")
set(CMAKE_CXX_FLAGS "-g -O2 -Wall -fPIC")

if ( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
    message( FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt." )
endif()

################################
# Normal Libraries & Executables
################################

# include header
set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR})
include_directories(${COMMON_INCLUDES})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/output/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/output/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/output/bin")

file(GLOB_RECURSE LOG_SOURCES "${PROJECT_SOURCE_DIR}/log/*.cpp")
file(GLOB_RECURSE LOG_HEADERS "${PROJECT_SOURCE_DIR}/log/*.h")

file(GLOB_RECURSE RPC_SOURCES "${PROJECT_SOURCE_DIR}/rpc/*.cpp")
file(GLOB_RECURSE DI_SOURCES "${PROJECT_SOURCE_DIR}/di/*.cpp")
file(GLOB_RECURSE SRC_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
set(LOG_LIB "log")
set(RPC_LIB "rpc")
set(DI_LIB "di")

# submodule
#add_executable(${PROJECT_NAME} ${SRC_SOURCES})
add_subdirectory(log)
#add_subdirectory(rpc)
#add_subdirectory(di)
#add_subdirectory(src)
ADD_EXECUTABLE(${PROJECT_NAME} ${SRC_SOURCES}) 
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LOG_LIB})

# static
ADD_EXECUTABLE(${PROJECT_NAME}-static ${SRC_SOURCES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME}-static ${LOG_LIB}-static "-static")

# TEST
if(BUILD_UNIT_TESTS)
    add_subdirectory(test)
endif()

## INSTALL
set(CMAKE_INSTALL_PREFIX /usr/local)
