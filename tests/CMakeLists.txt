enable_testing()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/output/test")

# Download gtest
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(DownloadGtest)
include_directories(${GTEST_INCLUDE_DIR})

# just for test
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/just_for_test.cpp
    "#include \"gtest/gtest.h\"\n"
    )
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/just_for_test.cpp
    "TEST(test,test){EXPECT_TRUE(true);}"
    )
add_executable(just_for_test
    ${CMAKE_CURRENT_BINARY_DIR}/just_for_test.cpp
    )
target_link_libraries(just_for_test
    gtest gtest_main
    )

# unittest
## LOG
file(GLOB LOG_UNITTESTS "log/*_unittest.cpp")
foreach(LOG_UT ${LOG_UNITTESTS})
    get_filename_component(LOG_UT_WE ${LOG_UT} NAME_WE)
    MESSAGE(STATUS "ADD_TEST:" ${LOG_UT_WE})
    add_executable(${LOG_UT_WE} ${LOG_UT}
        $<TARGET_OBJECTS:LOG_LIB>
        )

    ## gtest
    target_link_libraries(${LOG_UT_WE}
        gtest gtest_main)

    add_test(NAME ${LOG_UT_WE} COMMAND ${LOG_UT_WE})
endforeach()

