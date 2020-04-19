
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}

${CMAKE_SOURCE_DIR}/pnddata
${CMAKE_SOURCE_DIR}/PndTools/generalTools

${CMAKE_SOURCE_DIR}/eic/base
${CMAKE_SOURCE_DIR}/eic/tracking
${CMAKE_SOURCE_DIR}/eic/detectors/maps
)

# List of source files;
set( CXXSOURCES EicMaps.cxx MapsGeoParData.cxx VstGeoParData.cxx FstGeoParData.cxx MapsMimosaAssembly.cxx)

SET(DEPENDENCIES eicbase)
GENERATE_EICROOT_LIBRARY(maps mapsLinkDef.h)
