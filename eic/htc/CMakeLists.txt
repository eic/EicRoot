
set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 

#${GEANT3_INCLUDE_DIR}

${CMAKE_SOURCE_DIR}/base 
${CMAKE_SOURCE_DIR}/geobase
${CMAKE_SOURCE_DIR}/parbase
${CMAKE_SOURCE_DIR}/fairtools

${CMAKE_SOURCE_DIR}/field

${CMAKE_SOURCE_DIR}/eic/ayk
${CMAKE_SOURCE_DIR}/eic/base
${CMAKE_SOURCE_DIR}/eic/field
${CMAKE_SOURCE_DIR}/eic/tracking

${CMAKE_SOURCE_DIR}/eic/htc

${CMAKE_SOURCE_DIR}/trackbase
${CMAKE_SOURCE_DIR}/pnddata
${CMAKE_SOURCE_DIR}/pnddata/TrackData
${CMAKE_SOURCE_DIR}/PndTools/generalTools
)

#include_directories( ${INCLUDE_DIRECTORIES} )

# List of source files;
set( SOURCES
EicHtcTask.cxx
SensitiveVolume.cxx

KalmanNode.cxx
TrKalmanFilter.cxx

geant.cc
KalmanFilter.cc
MediaLayer.cc
MediaSlice.cc
MediaBank.cc
MediaSliceArray.cc
misc.cc
runge-kutta.cc
RungeKuttaRequest.cc
TrKalmanNode.cc
TrKalmanNodeLocation.cxx
)

SET(DEPENDENCIES eicbase ayk field tracking)
GENERATE_EICROOT_LIBRARY(htc eichtcLinkDef.h)
	 
