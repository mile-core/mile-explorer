set(CURRENT_TARGET_NAME mile-indexer)

file (GLOB INDXR_SRC ${BACKEND_SRC}
        ./statistics/*.cpp
        )

set(EXT_SRC ${INDXR_SRC})

include(../cmake/common.inc)

