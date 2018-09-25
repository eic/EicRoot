
set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 
${CMAKE_SOURCE_DIR}/genfit
${CMAKE_SOURCE_DIR}/GenfitTools/trackrep/LSLtrackRep
)

#include_directories( ${INCLUDE_DIRECTORIES} )

# List of source files;
set( SOURCES LSLTrackRep.cxx VirtSpacePointRecoHit.cxx LSLEQM.cxx Nystrom.cxx)

SET(DEPENDENCIES genfit)
GENERATE_EICROOT_LIBRARY(genfitLSL genfitLSLLinkDef.h)
	 
