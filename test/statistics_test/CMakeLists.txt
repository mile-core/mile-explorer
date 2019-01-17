
file (GLOB TESTS_SOURCES ${TESTS_SOURCES}
        *.cpp
        )

set (TEST wallets_top_test_${PROJECT_NAME})

add_executable(${TEST} ${TESTS_SOURCES})

set(MILECSA_LIBS mile-explorerdb milecsa milecsa_jsonrpc )

target_link_libraries (
        ${TEST}
        ${PROJECT_LIB}
        ${MILECSA_LIBS}
        ${OPENSSL_SSL_LIBRARY}
        ${OPENSSL_CRYPTO_LIBRARY}
        ${CMAKE_THREAD_LIBS_INIT}
        ${Boost_LIBRARIES}
        mile-explorerdb
        /home/atticus/CLionProjects/mile-explorer/vendor/librethinkdbxx/build/librethinkdb++.a
        milecsa_dispatchq
)

add_test (NAME wallets_top_test COMMAND ${TEST})
enable_testing ()
