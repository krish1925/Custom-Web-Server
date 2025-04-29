cmake_minimum_required(VERSION 3.10.0)
project(vibe-code-only)

# Prevent in‑source builds
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In‑source builds not allowed.\n"
                      "Please create a separate build directory and run CMake from there.\n"
                      "You may need to remove CMakeCache.txt.")
endif()

# Default to Debug when in a development shell
if (NOT CMAKE_BUILD_TYPE AND "$ENV{DEVEL_SHELL}" STREQUAL "1")
  message(STATUS "Setting default build type to Debug in devel shell")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Output all runtime binaries into ${CMAKE_BINARY_DIR}/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable testing
include(CTest)
enable_testing()

# GoogleTest
include(GoogleTest)
add_subdirectory(/usr/src/googletest googletest)
enable_testing()

# Boost (static libs)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.50 REQUIRED COMPONENTS system log log_setup regex)
message(STATUS "Boost version: ${Boost_VERSION}")

include_directories(include)

add_library(logging STATIC src/logging.cc)
target_link_libraries(logging PUBLIC
    Boost::log_setup
    Boost::log
    Boost::regex
)

# Core libraries
add_library(config_parser_lib src/config_parser.cc)

add_library(config_manager_lib src/config_manager.cc)
target_link_libraries(config_manager_lib
    PUBLIC  handler_lib
    PRIVATE config_parser_lib
)

add_library(handler_lib src/echo_handler.cc src/static_file_handler.cc)
target_include_directories(handler_lib PUBLIC include)

add_library(server_lib STATIC src/server.cc src/session.cc src/response.cc) # concrete handler
target_link_libraries(server_lib
    PRIVATE config_manager_lib
    PRIVATE Boost::system
    PRIVATE logging
)

# Main executable
add_executable(webserver src/_main.cc)
target_link_libraries(webserver
  PRIVATE
    server_lib
    config_parser_lib
    config_manager_lib
    Boost::system
)

# Unit Tests
add_executable(webserver_test tests/webserver_test.cc)
target_link_libraries(webserver_test
  PRIVATE
    server_lib
    config_parser_lib
    config_manager_lib
    Boost::system
    gtest
    gtest_main
    logging
)
gtest_discover_tests(webserver_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

add_executable(config_manager_test tests/config_manager_test.cc)
target_link_libraries(config_manager_test
  PRIVATE
    config_parser_lib
    config_manager_lib
    gtest
    gtest_main
    logging
)
gtest_discover_tests(config_manager_test)

add_executable(session_test tests/session_test.cc)
target_link_libraries(session_test
 PRIVATE
   server_lib
   Boost::system
   gtest
   gtest_main
)
gtest_discover_tests(session_test)

add_executable(config_parser_test
  tests/config_parser_test.cc
)
target_link_libraries(config_parser_test
  PRIVATE
    config_parser_lib
    gtest
    gtest_main
    logging
)
gtest_discover_tests(config_parser_test
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

add_executable(server_test tests/server_test.cc)
target_link_libraries(server_test
 PRIVATE
   server_lib
   Boost::system
   gtest
   gtest_main
)
gtest_discover_tests(server_test)

# Integration tests
add_executable(integration_test tests/integration/integration_tests.cc)
target_link_libraries(integration_test
  PRIVATE
    server_lib
    config_parser_lib
    config_manager_lib
    Boost::system
    gtest
    gtest_main
    logging
)
gtest_discover_tests(integration_test
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration
)

# Coverage report configuration
include(cmake/CodeCoverageReportConfig.cmake)
generate_coverage_report(
  TARGETS server_lib config_parser_lib config_manager_lib
  TESTS webserver_test config_manager_test session_test config_parser_test server_test integration_test
)


