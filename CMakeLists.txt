cmake_minimum_required(VERSION 3.8)
project(mile-explorer)

set(CMAKE_CXX_STANDARD 17)

set (CMAKE_CXX_STANDARD 17)
set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_MULTITHREADED OFF)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

file(GLOB LIBRARY_FILE_LIST ${CMAKE_SOURCE_DIR}/vendor/mile-csa-jsonrpc-client/*.*)
list(LENGTH LIBRARY_FILE_LIST NUMBER_OF_FILES)
if (NUMBER_OF_FILES EQUAL 0)
    message(STATUS "'mile-csa-jsonrpc-client' library not found")
    execute_process(COMMAND git submodule update --init --recursive vendor/mile-csa-jsonrpc-client
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
else ()
    message(STATUS "'mile-csa-jsonrpc-client' library found")
endif()

file(GLOB LIBRARY_FILE_LIST ${CMAKE_SOURCE_DIR}/vendor/mile-csa-dispatchq/*.*)
list(LENGTH LIBRARY_FILE_LIST NUMBER_OF_FILES)
if (NUMBER_OF_FILES EQUAL 0)
    message(STATUS "'mile-csa-dispatchq' library not found")
    execute_process(COMMAND git submodule update --init --recursive vendor/mile-csa-dispatchq
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
else ()
    message(STATUS "'mile-csa-dispatchq' library found")
endif()

add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/mile-csa-jsonrpc-client ${CMAKE_BINARY_DIR}/mile-csa-jsonrpc-client)
add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/mile-csa-dispatchq ${CMAKE_BINARY_DIR}/mile-csa-dispatchq)

set(yaml_path vendor/yaml-cpp)
find_package(${YAML_INCLUDE_DIR} REQUIRED PATHS ${yaml_path} )
include_directories(SYSTEM vendor/yaml-cpp/include)

add_subdirectory(vendor)
add_subdirectory(indexer)