
set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 
${CLHEP_INCLUDE_DIR}
${CMAKE_SOURCE_DIR}/genfit
${CMAKE_SOURCE_DIR}/GenfitTools/trackrep/RKTrackRep
)

#include_directories( ${INCLUDE_DIRECTORIES} )

# List of source files;
set( CXXSOURCES RKTrackRep.cxx GFMaterialEffects.cxx GFPointPath.h RKTools.cxx)

SET(DEPENDENCIES genfit)
GENERATE_EICROOT_LIBRARY(genfitRK genfitRKLinkDef.h)
	 
