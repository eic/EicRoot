
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}

${CMAKE_SOURCE_DIR}/pnddata
${CMAKE_SOURCE_DIR}/PndTools/generalTools

${CMAKE_SOURCE_DIR}/eic/base
${CMAKE_SOURCE_DIR}/eic/tracking
${CMAKE_SOURCE_DIR}/eic/detectors/gem
)

# List of source files;
set( CXXSOURCES EicGem.cxx GemGeoParData.cxx )

SET(DEPENDENCIES eicbase)
GENERATE_EICROOT_LIBRARY(gem gemLinkDef.h)
