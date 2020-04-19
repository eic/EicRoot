
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}

${GEANT3_INCLUDE_DIR}

${CMAKE_SOURCE_DIR}/mcstack
${CMAKE_SOURCE_DIR}/passive
${CMAKE_SOURCE_DIR}/pnddata
${CMAKE_SOURCE_DIR}/field
${CMAKE_SOURCE_DIR}/trackbase 
${CMAKE_SOURCE_DIR}/pnddata/TrackData
${CMAKE_SOURCE_DIR}/pnddata/PidData
${CMAKE_SOURCE_DIR}/rho
${CMAKE_SOURCE_DIR}/pid/PidData
${CMAKE_SOURCE_DIR}/PndTools/generalTools
${CMAKE_SOURCE_DIR}/eventdisplay

# FIXME: do these assignments conditional later;
${EICSMEAR}/include
#@${PROMC}/include
#@${PROMC}/src
#@${PROTOBUF}/include/google/protobuf/io
#@${EICMC}/include

${CMAKE_SOURCE_DIR}/eic/field
${CMAKE_SOURCE_DIR}/eic/event
${CMAKE_SOURCE_DIR}/eic/base
${CMAKE_SOURCE_DIR}/eic/calorimetry
)

#include_directories( ${INCLUDE_DIRECTORIES} )

# List of source files;
set( CXXSOURCES
EicLibrary.cxx

EicDetName.cxx
EicNamePatternHub.cxx 
EicContFact.cxx

EicGeo.cxx
EicGeoPar.cxx
EicGeoMap.cxx
EicGeoParData.cxx

EicRunSim.cxx
EicMCApplication.cxx
EicRunAna.cxx
EicRunDigi.cxx

EicBlackHole.cxx
EicEnergyMonitor.cxx
EicFieldMapDetector.cxx
EicDetector.cxx
EicFieldGradDetector.cxx

EicMoCaPoint.cxx
#EicFakeMoCaPoint.cxx
#EicFakeMoCaPointGenerator.cxx
#EicFakeMoCaPointDatabase.cxx
#EicFakeMoCaPointDbEntry.cxx
#EicFakeMoCaPointDbHit.cxx

EicDigiHitProducer.cxx
EicDigiParData.cxx

EicFileNameExpansion.cxx

EicEventManager.cxx
EicBoxGenerator.cxx	 
EicAsciiBoxGenerator.cxx
)

SET(EXTRA_HEADERS EicUnits.h)

if (DEFINED EICSMEAR) 
SET(DEPENDENCIES Base Field EventDisplay PndData Passive Eve event)
else (DEFINED EICSMEAR)
SET(DEPENDENCIES Base Field EventDisplay PndData Passive Eve)
endif (DEFINED EICSMEAR)

GENERATE_EICROOT_LIBRARY(eicbase eicbaseLinkDef.h)

