include(${CMAKE_SOURCE_DIR}/cmake/settings.inc)

file (GLOB COMMON_SRC ${COMMON_SRC}
        ../common/src/*.cpp
        ../common/src/db-updates/*.cpp
        )


add_executable(${CURRENT_TARGET_NAME} main.cpp ${COMMON_SRC} ${EXT_SRC})

target_link_libraries(
        ${CURRENT_TARGET_NAME}
        ${CMAKE_THREAD_LIBS_INIT}
        ${Boost_LIBRARIES}
        ${OPENSSL_SSL_LIBRARY}
        ${OPENSSL_CRYPTO_LIBRARY}
        milecsa_jsonrpc
        milecsa
        milecsa_dispatchq
        yaml-cpp
        librethinkdb++.a
)
