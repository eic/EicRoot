
set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 
${BASE_INCLUDE_DIRECTORIES} 
${CMAKE_SOURCE_DIR}/pnddata 
${CMAKE_SOURCE_DIR}/pnddata/TrackData
${CMAKE_SOURCE_DIR}/trackbase
)

#include_directories( ${INCLUDE_DIRECTORIES} )

# List of source files;
set( SOURCES
PndStack.cxx 
PndMCTrack.cxx

TrackData/PndTrackCandHit.cxx
TrackData/PndTrackCand.cxx
TrackData/PndTrack.cxx
TrackData/PndTrackID.cxx
TrackData/PndVertex.cxx

PidData/PndPidCandidate.cxx
PidData/PndPidProbability.cxx

FairRecoCandidate.cxx
)

SET(DEPENDENCIES Base GeoBase TrkBase)
GENERATE_EICROOT_LIBRARY(PndData PndDataLinkDef.h)
