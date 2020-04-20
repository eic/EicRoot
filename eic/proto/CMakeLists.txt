
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}

${CMAKE_SOURCE_DIR}/eic/base
${CMAKE_SOURCE_DIR}/eic/proto
)

# List of source files;
set( CXXSOURCES EicProtoGenerator.cxx )

SET(DEPENDENCIES Base )
GENERATE_EICROOT_LIBRARY(proto eicprotoLinkDef.h)

