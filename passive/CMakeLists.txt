
set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 
${BASE_INCLUDE_DIRECTORIES} 
${CMAKE_SOURCE_DIR}/passive
)

#include_directories( ${INCLUDE_DIRECTORIES} )

# List of source files;
set( SOURCES
PndCave.cxx            
PndGeoCave.cxx        
PndGeoMagnet.cxx      
PndGeoPassivePar.cxx  
PndGeoPipe.cxx        
PndGeoTarget.cxx      
PndMagnet.cxx         
PndPassiveContFact.cxx
PndPipe.cxx           
PndTarget.cxx 
)

SET(DEPENDENCIES Base GeoBase ParBase)
GENERATE_EICROOT_LIBRARY(Passive PassiveLinkDef.h)
	 
