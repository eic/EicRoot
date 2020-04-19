
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}

${CMAKE_SOURCE_DIR}/GenfitTools/trackrep
${CMAKE_SOURCE_DIR}/GenfitTools/trackrep/GeaneTrackRep
${CMAKE_SOURCE_DIR}/GenfitTools/trackrep/LSLtrackRep
${CMAKE_SOURCE_DIR}/GenfitTools/trackrep/RKTrackRep
${CMAKE_SOURCE_DIR}/genfit
${CMAKE_SOURCE_DIR}/trackbase
${CMAKE_SOURCE_DIR}/geane

${GEANT3_INCLUDE_DIR}
)

# List of source files;
set( CXXSOURCES
GeaneTrackRep/GeaneTrackRep.cxx
)

SET(DEPENDENCIES Geane TrkBase)
GENERATE_EICROOT_LIBRARY(trackrep trackrepLinkDef.h)
	 
