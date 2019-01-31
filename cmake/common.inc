include(${CMAKE_SOURCE_DIR}/cmake/settings.inc)

add_executable(${CURRENT_TARGET_NAME} main.cpp ${EXT_SRC})

target_link_libraries(
        ${CURRENT_TARGET_NAME}
        ${CMAKE_THREAD_LIBS_INIT}
        mile-explorerdb
        milecsa_jsonrpc
        milecsa
        milecsa_dispatchq
        yaml-cpp
        librethinkdb++.a
        ${Boost_LIBRARIES}
        ${OPENSSL_SSL_LIBRARY}
        ${OPENSSL_CRYPTO_LIBRARY}
)
