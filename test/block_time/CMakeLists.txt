find_package (Threads)

file (GLOB TESTS_SOURCES ${TESTS_SOURCES}
        *.cpp
        )

set (TEST block_time_${PROJECT_NAME})

add_executable(${TEST} ${TESTS_SOURCES})

set(MILECSA_LIBS milecsa milecsa_jsonrpc milecsa_dispatchq)

target_link_libraries (
        ${TEST}
        ${PROJECT_LIB}
        ${MILECSA_LIBS}
        ${OPENSSL_SSL_LIBRARY}
        ${OPENSSL_CRYPTO_LIBRARY}
        ${CMAKE_THREAD_LIBS_INIT}
        ${Boost_LIBRARIES}
        mile-explorerdb)

add_test (NAME block_time COMMAND ${TEST})
enable_testing ()
