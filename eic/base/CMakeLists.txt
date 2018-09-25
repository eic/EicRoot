
set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 

${GEANT3_INCLUDE_DIR}

${CMAKE_SOURCE_DIR}/geobase 
${CMAKE_SOURCE_DIR}/parbase 
${CMAKE_SOURCE_DIR}/base 

${CMAKE_SOURCE_DIR}/base/sim 
${CMAKE_SOURCE_DIR}/base/steer 
${CMAKE_SOURCE_DIR}/base/event 
${CMAKE_SOURCE_DIR}/dbase/dbInterface 
${CMAKE_SOURCE_DIR}/dbase/dbValidation
${CMAKE_SOURCE_DIR}/dbase/dbInput

${CMAKE_SOURCE_DIR}/generators 

${CMAKE_SOURCE_DIR}/mcstack
${CMAKE_SOURCE_DIR}/passive
${CMAKE_SOURCE_DIR}/pnddata
${CMAKE_SOURCE_DIR}/field
${CMAKE_SOURCE_DIR}/trackbase 
${CMAKE_SOURCE_DIR}/pnddata/TrackData
${CMAKE_SOURCE_DIR}/pnddata/PidData
${CMAKE_SOURCE_DIR}/rho
${CMAKE_SOURCE_DIR}/pid/PidData
${CMAKE_SOURCE_DIR}/fairtools
${CMAKE_SOURCE_DIR}/PndTools/generalTools
${CMAKE_SOURCE_DIR}/eventdisplay

# FIXME: do these assignments conditional later;
${EICSMEAR}/include
${PROMC}/include
${PROMC}/src
${PROTOBUF}/include/google/protobuf/io
${EICMC}/include

${CMAKE_SOURCE_DIR}/eic/field
${CMAKE_SOURCE_DIR}/eic/base
${CMAKE_SOURCE_DIR}/eic/calorimetry
)

#include_directories( ${INCLUDE_DIRECTORIES} )

# List of source files;
set( SOURCES
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
#EicFlatDisEventGenerator.cxx 
EicBoxGenerator.cxx	 
EicAsciiBoxGenerator.cxx
)

SET(EXTRA_HEADERS EicUnits.h)

SET(DEPENDENCIES Base Field EventDisplay PndData Passive Eve)
GENERATE_EICROOT_LIBRARY(eicbase eicbaseLinkDef.h)
