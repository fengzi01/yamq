enable_testing()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/output/test")

# gtest
#add_subdirectory(/home/fengyuwei/code/github/googletest gtest)
#add_subdirectory(/Users/fengyuwei/code/googletest gtest)
#include_directories(${GTEST_INCLUDE_DIR})

##################################
# Download and install GoogleTest
include(ExternalProject)
ExternalProject_Add(googletest
    URL https://github.com/google/googletest/archive/release-1.7.0.zip
    URL_HASH SHA256=b58cb7547a28b2c718d1e38aee18a3659c9e3ff52440297e965f5edffe34b6d0
    PREFIX ${CMAKE_BINARY_DIR}/googletest
    INSTALL_COMMAND ""
    )
ExternalProject_Get_Property(googletest source_dir binary_dir)
set(GTEST_INCLUDE_DIRS ${source_dir}/include)
set(GTEST_LINK_DIRS ${binary_dir})
add_library(gtest STATIC IMPORTED)
add_library(gtest_main STATIC IMPORTED)
add_library(gmock STATIC IMPORTED)
add_dependencies(gtest googletest)
add_dependencies(gtest_main googletest)
add_dependencies(gmock googletest)

include_directories(
    ${GTEST_INCLUDE_DIRS}
    )
link_directories(${GTEST_LINK_DIRS})

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
    libgtest.a libgtest_main.a
    )
add_dependencies(just_for_test googletest)

# unittest
file(GLOB LOG_UNITTESTS "log_*_unittest.cpp")
foreach(LOG_UT ${LOG_UNITTESTS})
    get_filename_component(LOG_UT_WE ${LOG_UT} NAME_WE)
    MESSAGE(STATUS "ADD_TEST:" ${LOG_UT_WE})
    add_executable(${LOG_UT_WE} ${LOG_UT}
        $<TARGET_OBJECTS:LOG_LIB>
        )

    ## gtest
    add_dependencies(${LOG_UT_WE} just_for_test)
    target_link_libraries(${LOG_UT_WE}
        libgtest.a libgtest_main.a
        ${DYNAMIC_LIB})

    add_test(NAME ${LOG_UT_WE} COMMAND ${LOG_UT_WE})
endforeach()

