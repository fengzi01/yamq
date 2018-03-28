enable_testing()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/output/test")

# Download gtest
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(DownloadGtest)
include_directories(${GTEST_INCLUDE_DIR})

# unittest
## log
file(GLOB LOG_UNITTESTS "log*_unittest.cpp")
foreach(LOG_UT ${LOG_UNITTESTS})
    get_filename_component(LOG_UT_WE ${LOG_UT} NAME_WE)
    MESSAGE(STATUS "ADD_TEST:" ${LOG_UT_WE})
    add_executable(${LOG_UT_WE} ${LOG_UT} $<TARGET_OBJECTS:log_objects>)
    ## gtest
    target_link_libraries(${LOG_UT_WE} gtest gtest_main)
    add_test(NAME ${LOG_UT_WE} COMMAND ${LOG_UT_WE})
endforeach()

file(GLOB LOG_TESTS "log*_test.cpp")
foreach(LOG_TEST ${LOG_TESTS}) 
    get_filename_component(LOG_TEST_WE ${LOG_TEST} NAME_WE)
    add_executable(${LOG_TEST_WE} ${LOG_TEST} $<TARGET_OBJECTS:log_objects>)
endforeach()

## rpc
file(GLOB UNITTESTS "rpc*_unittest.cpp")
foreach(UT ${UNITTESTS})
    get_filename_component(UT_NAME ${UT} NAME_WE)
    MESSAGE(STATUS "ADD_TEST:" ${UT_NAME})
    add_executable(${UT_NAME} ${UT} $<TARGET_OBJECTS:log_objects> $<TARGET_OBJECTS:rpc_objects>)
    target_link_libraries(${UT_NAME} gtest gtest_main)
    add_test(NAME ${UT_NAME} COMMAND ${UT_NAME})
endforeach()

file(GLOB TESTS "rpc*_test.cpp")
foreach(TEST ${TESTS}) 
    get_filename_component(TEST_NAME ${TEST} NAME_WE)
    add_executable(${TEST_NAME} ${TEST} $<TARGET_OBJECTS:log_objects> $<TARGET_OBJECTS:rpc_objects>)
endforeach()

