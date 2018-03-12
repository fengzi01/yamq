enable_testing()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/output/test")

# Download gtest
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(DownloadGtest)
include_directories(${GTEST_INCLUDE_DIR})

# unittest
## LOG
file(GLOB LOG_UNITTESTS "log*_unittest.cpp")
foreach(LOG_UT ${LOG_UNITTESTS})
    get_filename_component(LOG_UT_WE ${LOG_UT} NAME_WE)
    #MESSAGE(STATUS "ADD_TEST:" ${LOG_UT_WE})
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

file(GLOB RPC_TESTS "rpc*_test.cpp")
foreach(RPC_TEST ${RPC_TESTS}) 
    get_filename_component(RPC_TEST_WE ${RPC_TEST} NAME_WE)
    add_executable(${RPC_TEST_WE} ${RPC_TEST} $<TARGET_OBJECTS:log_objects> $<TARGET_OBJECTS:rpc_objects>)
endforeach()

