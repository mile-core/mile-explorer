set(CURRENT_TARGET_NAME mile-explorer-backend)

include_directories(
        ./jsonrpc-server
)

file (GLOB BACKEND_SRC ${BACKEND_SRC}
        ./jsonrpc-server/*.cpp
        )

set(EXT_SRC ${BACKEND_SRC})

include(../cmake/common.inc)

