set(CURRENT_TARGET_NAME mile-explorerdb)

include(../cmake/settings.inc)

set(INDEXER_VERSION  ${PROJECT_VERSION_MAJOR}${PROJECT_VERSION_MINOR}${PROJECT_VERSION_PATCH})
set(EXPLORER_VERSION ${PROJECT_VERSION_MAJOR}${PROJECT_VERSION_MINOR}${PROJECT_VERSION_PATCH})

configure_file(include/version.hpp.in ${CMAKE_BINARY_DIR}/generated/version.hpp )
include_directories( ${CMAKE_BINARY_DIR}/generated/ ) # Make sure it can be included...

file (GLOB COMMON_SRC ${COMMON_SRC}
        ../common/src/*.cpp
        ../common/src/db/*.cpp
        ../common/src/db/migrations/*.cpp
        )

add_library(${CURRENT_TARGET_NAME} ${COMMON_SRC})
