find_package (Threads)

file (GLOB TESTS_SOURCES ${TESTS_SOURCES}
        *.cpp
        )

set (TEST genesis_test_${PROJECT_NAME})

add_executable(${TEST} ${TESTS_SOURCES})

set(MILECSA_LIBS milecsa_jsonrpc)

target_link_libraries (
        ${TEST}
        ${PROJECT_LIB}
        ${MILECSA_LIBS}
        ${OPENSSL_SSL_LIBRARY}
        ${OPENSSL_CRYPTO_LIBRARY}
        ${CMAKE_THREAD_LIBS_INIT}
        ${Boost_LIBRARIES}
        mile-explorerdb)

add_test (NAME genesis_test COMMAND ${TEST})
enable_testing ()
