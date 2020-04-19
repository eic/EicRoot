
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}

${CMAKE_SOURCE_DIR}/pnddata
${CMAKE_SOURCE_DIR}/PndTools/generalTools

${CMAKE_SOURCE_DIR}/tpc/base
${CMAKE_SOURCE_DIR}/tpc/tools

${CMAKE_SOURCE_DIR}/eic/base
${CMAKE_SOURCE_DIR}/eic/tracking
${CMAKE_SOURCE_DIR}/eic/detectors/tpc
)

# List of source files;
set( CXXSOURCES
EicTpc.cxx 
#EicToFopiMoCaConverter.cxx 
#FopiToEicDigiConverter.cxx 
EicTpcDigiHitProducer.cxx

TpcGeoParData.cxx
)

SET(DEPENDENCIES eicbase tracking)
GENERATE_EICROOT_LIBRARY(tpc tpcLinkDef.h)
