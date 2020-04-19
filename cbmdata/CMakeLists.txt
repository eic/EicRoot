# Use the same GENERATE_EICROOT_LIBRARY as in my own codes;

set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES} 

${CMAKE_SOURCE_DIR}/cbmbase 
${CMAKE_SOURCE_DIR}/cbmdata 
)

set(CXXSOURCES
CbmDetectorList.cxx

CbmBaseHit.cxx
CbmDigi.cxx
CbmHit.cxx
CbmPixelHit.cxx
CbmStack.cxx
CbmStripHit.cxx
CbmTrack.cxx
CbmTrackMatch.cxx
CbmCluster.cxx

CbmMCEpoch.cxx
CbmMCEventHeader.cxx
CbmMCTrack.cxx

rich/CbmRichRing.cxx	
rich/CbmRichHit.cxx 
rich/CbmRichPoint.cxx

global/CbmGlobalTrack.cxx
global/CbmVertex.cxx
)

Set(DEPENDENCIES Base )
GENERATE_EICROOT_LIBRARY( CbmData DataLinkDef.h)


