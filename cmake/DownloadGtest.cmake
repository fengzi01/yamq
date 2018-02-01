# Download and unpack googletest at configure time
set(googletest-root-dir ${CMAKE_BINARY_DIR}/3rdparty/googletest)
set(googletest-download-dir ${googletest-root-dir}/googletest-download)
configure_file(${PROJECT_SOURCE_DIR}/cmake/DownloadGtest.CMakeLists.txt.in ${googletest-download-dir}/CMakeLists.txt)
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
	WORKING_DIRECTORY ${googletest-download-dir} )
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
	WORKING_DIRECTORY ${googletest-download-dir} )

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This adds
# the following targets: gtest, gtest_main, gmock
# and gmock_main
add_subdirectory("${googletest-root-dir}/googletest-src"
	"${googletest-root-dir}/googletest-build")

# The gtest/gmock targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if(CMAKE_VERSION VERSION_LESS 2.8.11)
	include_directories("${gtest_SOURCE_DIR}/include"
		"${gmock_SOURCE_DIR}/include")
endif()

# Now simply link your own targets against gtest, gmock,
# etc. as appropriate
