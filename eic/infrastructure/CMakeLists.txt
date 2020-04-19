
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}

${CMAKE_SOURCE_DIR}/pnddata
${CMAKE_SOURCE_DIR}/PndTools/generalTools

${CMAKE_SOURCE_DIR}/eic/base
${CMAKE_SOURCE_DIR}/eic/infrastructure
)

# List of source files;
set( CXXSOURCES BeamPipeGeoParData.cxx)

SET(DEPENDENCIES eicbase)
GENERATE_EICROOT_LIBRARY(infrastructure infrastructureLinkDef.h)
	 
