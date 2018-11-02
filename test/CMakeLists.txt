include(${CMAKE_SOURCE_DIR}/cmake/settings.inc)

include_directories (
        ./
)

add_subdirectory(rpc_test)
enable_testing ()
