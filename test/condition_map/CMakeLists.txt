
file (GLOB TESTS_SOURCES ${TESTS_SOURCES}
        *.cpp
        )

set (TEST condition_map_${PROJECT_NAME})

add_executable(${TEST} ${TESTS_SOURCES})

set(MILECSA_LIBS milecsa milecsa_jsonrpc)

target_link_libraries (
        ${TEST}
        ${PROJECT_LIB}
        ${MILECSA_LIBS}
        ${OPENSSL_SSL_LIBRARY}
        ${OPENSSL_CRYPTO_LIBRARY}
        ${CMAKE_THREAD_LIBS_INIT}
        ${Boost_LIBRARIES}
        mile-explorerdb)

add_test (NAME condition_map COMMAND ${TEST})
enable_testing ()
