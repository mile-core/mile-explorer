set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     ON)
set(CMAKE_CXX_FLAGS -pthread)

set(CURRENT_TARGET_NAME mile-indexer)

find_package (Boost REQUIRED COMPONENTS system)
find_package(OpenSSL)

file (GLOB COMMON_SRC ${COMMON_SRC}
        ../common/src/*.cpp
        ../common/src/db-updates/*.cpp
        )

include_directories(
        ../common/include
        ../vendor/librethinkdbxx/build/include/
        ../vendor/mile-csa-jsonrpc-client/include
        ../vendor/mile-csa-dispatchq/include
        ../vendor/spdlog
	../vendor
        ${OPENSSL_INCLUDE_DIR}
        ${Boost_INCLUDE_DIRS}
        ${OPENSSL_INCLUDE_DIR}
)

link_directories(../vendor/librethinkdbxx/build)

add_executable(${CURRENT_TARGET_NAME} main.cpp ${COMMON_SRC})

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
