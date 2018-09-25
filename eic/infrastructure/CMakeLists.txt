
set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 

${CMAKE_SOURCE_DIR}/geobase 
${CMAKE_SOURCE_DIR}/parbase 
${CMAKE_SOURCE_DIR}/base 
${CMAKE_SOURCE_DIR}/base/sim 
${CMAKE_SOURCE_DIR}/base/steer 
${CMAKE_SOURCE_DIR}/base/event  
${CMAKE_SOURCE_DIR}/dbase/dbInterface 
${CMAKE_SOURCE_DIR}/dbase/dbValidation
${CMAKE_SOURCE_DIR}/dbase/dbInput

${CMAKE_SOURCE_DIR}/fairtools
${CMAKE_SOURCE_DIR}/pnddata
${CMAKE_SOURCE_DIR}/PndTools/generalTools

${CMAKE_SOURCE_DIR}/eic/base
${CMAKE_SOURCE_DIR}/eic/infrastructure
)

#include_directories( ${INCLUDE_DIRECTORIES} )

# List of source files;
set( SOURCES BeamPipeGeoParData.cxx)

SET(DEPENDENCIES eicbase)
GENERATE_EICROOT_LIBRARY(infrastructure infrastructureLinkDef.h)
	 
