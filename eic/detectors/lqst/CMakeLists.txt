
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}

${CMAKE_SOURCE_DIR}/pnddata
${CMAKE_SOURCE_DIR}/PndTools/generalTools

${CMAKE_SOURCE_DIR}/eic/base
${CMAKE_SOURCE_DIR}/eic/detectors/lqst
)

# List of source files;
set( CXXSOURCES EicLowQ2Tagger.cxx LqstGeoParData.cxx )

SET(DEPENDENCIES eicbase)
GENERATE_EICROOT_LIBRARY(lqst lqstLinkDef.h)
