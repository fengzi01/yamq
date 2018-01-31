if (BUILD_UNIT_TESTS)
  enable_testing()
  # include_directories(${COMMON_INCLUDES})

  # gtest
  #add_subdirectory(/home/fengyuwei/code/github/googletest gtest)
  add_subdirectory(/Users/fengyuwei/code/googletest gtest)
  include_directories(${GTEST_INCLUDE_DIR})

  # log unittest
  file(GLOB LOG_UNITTESTS "log_*_unittest.cpp")
  foreach(LOG_UT ${LOG_UNITTESTS})
      get_filename_component(LOG_UT_WE ${LOG_UT} NAME_WE)
      add_executable(${LOG_UT_WE} ${LOG_UT}
          $<TARGET_OBJECTS:LOG_LIB>
          )
      target_link_libraries(${LOG_UT_WE}
          gtest gtest_main
          ${DYNAMIC_LIB})
      add_test(NAME LOG_UT_WE COMMAND LOG_UT_WE)
  endforeach()


  ##############
  # Unit Tests
  ##############
  # add_executable(TestExample test/test_example.cpp src/example.cpp)
  #target_link_libraries(TestExample gtest gtest_main)
  # add_test(NAME example COMMAND TestExample)
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
