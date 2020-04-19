
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}

${CMAKE_SOURCE_DIR}/pnddata
${CMAKE_SOURCE_DIR}/field
${CMAKE_SOURCE_DIR}/PndTools/generalTools

${CMAKE_SOURCE_DIR}/eic/base
${CMAKE_SOURCE_DIR}/eic/calorimetry
)

# List of source files;
set( CXXSOURCES 
BarrelGeoParData.cxx
EndcapGeoParData.cxx

FiberParData.cxx 

CalorimeterGeoParData.cxx

EicCalorimeterDigiHit.cxx
EicCalorimeterDigiHitProducer.cxx

EicCalorimeterReconstruction.cxx
)

SET(DEPENDENCIES eicbase)
GENERATE_EICROOT_LIBRARY(calorimetry calorimetryLinkDef.h)
