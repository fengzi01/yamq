cmake_minimum_required(VERSION 2.8)

# Options. Turn on with 'cmake -Dmyvarname=ON'.
option(test "Build all tests." OFF) # Makes boolean 'test' available.

# Make PROJECT_SOURCE_DIR, PROJECT_BINARY_DIR, and PROJECT_NAME available.
set(PROJECT_NAME yamq)
project(${PROJECT_NAME})

# set(CMAKE_CXX_FLAGS "-g -Wall")
set(CMAKE_C_FLAGS "-g -O2 -Wall -fPIC")
set(CMAKE_CXX_FLAGS "-g -O2 -Wall -fPIC")

if ( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
    message( FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt." )
endif()

# force static runtime libraries for msvc builds
if(MSVC)
  set(variables 
    CMAKE_CXX_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_RELEASE
	CMAKE_CXX_FLAGS_RELWITHDEBINFO
	CMAKE_CXX_FLAGS_MINSIZEREL
  )
  foreach(variable ${variables})
	if(${variable} MATCHES "/MD")
	  string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
	endif()
  endforeach()
endif()

# If you want your own include/ directory, set this, and then you can do
# include_directories(${COMMON_INCLUDES}) in other CMakeLists.txt files.
# set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)

################################
# Normal Libraries & Executables
################################

set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/src/)
include_directories(${COMMON_INCLUDES})

#file(GLOB SRC_LIB_FILES ${PROJECT_SOURCE_DIR}/src/lib/*.cpp)
#add_library(project1_lib ${SRC_LIB_FILES})
#add_library(project1_lib project1.cpp)

file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(${PROJECT_NAME} ${SRC_FILES})
# Key idea: SEPARATE OUT your main() function into its own file so it can be its
# own executable. Separating out main() means you can add this library to be
# used elsewhere.

#target_link_libraries(project1 project1_lib)

################################
# Testing
################################
if (test)
  if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
  endif (APPLE)

  # This adds another subdirectory, which has 'project(gtest)'.
  # add_subdirectory(lib/gtest-1.6.0)

  enable_testing()

  # Include the gtest library. gtest_SOURCE_DIR is available due to
  # 'project(gtest)' above.
  # include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

  include_directories(${COMMON_INCLUDES})
  file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/test/*.cpp)

  # 这个里面必须有CMakeLists.txt
  # add_subdirectory(/home/fengyuwei/code/github/googletest gtest)
  add_subdirectory(/Users/fengyuwei/code/googletest gtest)
  include_directories(${GTEST_INCLUDE_DIR})

  ##############
  # Unit Tests
  ##############
  add_executable(TestExample test/test_example.cpp src/example.cpp)
  target_link_libraries(TestExample gtest gtest_main)
  # target_link_libraries(runUnitTests project1_lib)
  add_test(NAME example COMMAND TestExample)
  # You can also omit NAME and COMMAND. The second argument could be some other
  # test executable.
  # add_test(example1 runUnitTests)

  #add_executable(TestA TestA.cpp)
  #target_link_libraries(TestA gtest)
  #add_test(NAME AllTestsInA COMMAND TestA)

  #add_executable(TestB TestB.cpp)
  #target_link_libraries(TestB gtest)
  #add_test(NAME AllTestsInB COMMAND TestB)

  #add_executable(TestC TestC.cpp)
  #target_link_libraries(TestC gtest)
  #add_test(NAME AllTestsInC COMMAND TestC)
endif()
