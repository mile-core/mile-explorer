include(${CMAKE_SOURCE_DIR}/cmake/settings.inc)

include_directories (
        ./
)

add_subdirectory(rpc_test)
add_subdirectory(block_time)
add_subdirectory(condition_map)
enable_testing ()
