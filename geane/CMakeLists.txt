
set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 
${GEANT3_INCLUDE_DIR}
${CMAKE_SOURCE_DIR}/fairtools
${CMAKE_SOURCE_DIR}/geobase 
${CMAKE_SOURCE_DIR}/parbase 
${CMAKE_SOURCE_DIR}/base 
${CMAKE_SOURCE_DIR}/trackbase 
${CMAKE_SOURCE_DIR}/geane
)

#include_directories( ${INCLUDE_DIRECTORIES} )

# List of source files;
set( SOURCES
FairGeane.cxx       
FairGeanePro.cxx
)

SET(DEPENDENCIES Base TrkBase Geom VMC EG Physics Core)
GENERATE_EICROOT_LIBRARY(Geane GeaneLinkDef.h)
