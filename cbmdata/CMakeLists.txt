# Create a library called "libCbmData" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 

${BASE_INCLUDE_DIRECTORIES} 
${CMAKE_SOURCE_DIR}/cbmbase 
${CMAKE_SOURCE_DIR}/cbmdata 
)

include_directories( ${INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
)
 
link_directories( ${LINK_DIRECTORIES})

set(SRCS
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

set(LINKDEF  DataLinkDef.h)
Set(LIBRARY_NAME CbmData)
Set(DEPENDENCIES
    Base
)

GENERATE_LIBRARY()


