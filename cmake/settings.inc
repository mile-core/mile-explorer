set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     ON)
set(CMAKE_CXX_FLAGS -pthread)

find_package (Boost REQUIRED COMPONENTS system)
find_package(OpenSSL)

include_directories(
        ../common/include
        ../vendor/librethinkdbxx/build/include/
        ../vendor/mile-csa-jsonrpc-client/include
        ../vendor/mile-csa-jsonrpc-client/vendor/mile-csa-api/include
        ../vendor/mile-csa-jsonrpc-client/vendor/mile-csa-api/vendor/nlohmann
        ../vendor/mile-csa-jsonrpc-client/vendor/mile-csa-api/vendor/mile-crypto/include
        ../vendor/mile-csa-dispatchq/include
        ../vendor/logger/spdlog
	../vendor/logger
        ${OPENSSL_INCLUDE_DIR}
        ${Boost_INCLUDE_DIRS}
        ${OPENSSL_INCLUDE_DIR}
)

link_directories(
        ../vendor/librethinkdbxx/build
)
