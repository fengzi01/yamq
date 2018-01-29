if (BUILD_UNIT_TESTS)
  enable_testing()
  # include_directories(${COMMON_INCLUDES})
  file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/test/*.cpp)

  add_subdirectory(/home/fengyuwei/code/github/googletest gtest)
  #add_subdirectory(/Users/fengyuwei/code/googletest gtest)
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
