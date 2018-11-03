set(CURRENT_TARGET_NAME mile-explorer-backend)

include_directories(
        ./
)

file (GLOB BACKEND_SRC ${BACKEND_SRC}
        ./jsonrpc/*.cpp
        ./jsonrpc/http/*.cpp
        ./jsonrpc/https/*.cpp
        ./api/*.cpp
        )

set(EXT_SRC ${BACKEND_SRC})

include(../cmake/common.inc)

