cmake_minimum_required(VERSION 3.12)

project(votechess)

###
# Find packages
###
find_package(spdlog CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)
find_package(Boost REQUIRED)
###

###
# Base directory relative includes for everyone
###
include_directories(src/main)
###

###
# Global Flags
###
add_compile_options(-Wall -Wextra -Wpedantic)
###

###
# Flags
###
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()
###

###
# Targets
###
# votechess
add_executable(
    votechess
    src/main/votechess/main.cpp
    src/main/votechess/irc_client.cpp
    src/main/votechess/vote_service.cpp
    src/main/votechess/uci_service_wrapper.cpp
    src/main/votechess/rng_service.cpp
    src/main/votechess/config_service.cpp
)
target_link_libraries(votechess PRIVATE spdlog::spdlog)
target_link_libraries(votechess INTERFACE Boost::Boost)
###


###
# Tests
###
#enable_testing()
#add_test(votechess_test_build "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target votechess_test)
#add_test(votechess_test_run votechess_test)
#set_tests_properties(votechess_test_run PROPERTIES DEPENDS votechess_test_build)
###

###
# votechess_test
#add_executable(
#    votechess_test
#    src/test/test.cpp
#    src/test/votechess/example.cpp
#)
#target_link_libraries(votechess_test PRIVATE Catch2::Catch2)
###
