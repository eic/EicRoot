
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}

${CMAKE_SOURCE_DIR}/pnddata
${CMAKE_SOURCE_DIR}/PndTools/generalTools

${CMAKE_SOURCE_DIR}/eic/base
${CMAKE_SOURCE_DIR}/eic/calorimetry
${CMAKE_SOURCE_DIR}/eic/detectors/calorimeter
)

# List of source files;
set( CXXSOURCES EicCalorimeter.cxx FemcGeoParData.cxx CemcGeoParData.cxx BemcGeoParData.cxx HcalGeoParData.cxx)

SET(DEPENDENCIES eicbase calorimetry)
GENERATE_EICROOT_LIBRARY(calorimeter calorimeterLinkDef.h)

