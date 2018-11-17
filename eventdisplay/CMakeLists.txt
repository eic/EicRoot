
set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 
${GEANT3_INCLUDE_DIR}
${CMAKE_SOURCE_DIR}/fairtools
${CMAKE_SOURCE_DIR}/geobase 
${CMAKE_SOURCE_DIR}/parbase 
${CMAKE_SOURCE_DIR}/base 
${CMAKE_SOURCE_DIR}/trackbase 
${CMAKE_SOURCE_DIR}/geane
${CMAKE_SOURCE_DIR}/eventdisplay
)

#include_directories( ${INCLUDE_DIRECTORIES} )

# List of source files;
set( CXXSOURCES
FairEventManager.cxx  
FairMCTracks.cxx
FairEventManagerEditor.cxx
FairMCTracksEditor.cxx
FairBoxSet.cxx
FairBoxSetDraw.cxx
FairBoxSetEditor.cxx
FairPointSetDraw.cxx
FairMCPointDraw.cxx
FairHitDraw.cxx
FairHitPointSetDraw.cxx
FairTimeStampPointDraw.cxx
)

SET(DEPENDENCIES Base Eve Geom EG  Physics Ged Gui Graf3d Tree Core)
GENERATE_EICROOT_LIBRARY(EventDisplay EventDisplayLinkDef.h)
	 
