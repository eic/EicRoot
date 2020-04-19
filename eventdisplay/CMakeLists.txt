
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}

${GEANT3_INCLUDE_DIR}

${CMAKE_SOURCE_DIR}/trackbase 
${CMAKE_SOURCE_DIR}/geane
${CMAKE_SOURCE_DIR}/eventdisplay
)

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
	 
