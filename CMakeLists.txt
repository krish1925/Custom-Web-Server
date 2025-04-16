cmake_minimum_required(VERSION 3.10.0)
project(vibe-code-only)

# Make sure developers do not run cmake in the main project directory, to keep
# build artifacts from becoming clutter
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory) and run CMake from there.
    You may need to remove CMakeCache.txt." )
endif()

# Turn on debug builds if we are building in a devel shell
if (CMAKE_BUILD_TYPE STREQUAL "" AND "$ENV{DEVEL_SHELL}" STREQUAL "1")
    message(STATUS "Setting debug build type by default in devel shell")
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Output binaries to a sub directory "bin"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable cmake testing
include(CTest)
enable_testing()

# Enable GoogleTest
include(GoogleTest)
add_subdirectory(/usr/src/googletest googletest)

# Enable Boost
# Use static libraries so binaries can be deployed without a full boost install
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.50 REQUIRED COMPONENTS system)
message(STATUS "Boost version: ${Boost_VERSION}")

include_directories(include)

# Server libraries
add_library(server_lib src/session.cc src/server.cc)
add_library(config_parser_lib src/config_parser.cc)
target_link_libraries(server_lib Boost::system)

# Main webserver executable
add_executable(webserver src/server_main.cc)
target_link_libraries(webserver server_lib config_parser_lib Boost::system)

# Adding webserver test - using gtest but not gtest_main
add_executable(webserver_test tests/webserver_test.cc)
target_link_libraries(webserver_test server_lib config_parser_lib Boost::system gtest)
gtest_discover_tests(webserver_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)