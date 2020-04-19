
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES} 

${CMAKE_SOURCE_DIR}/passive
)

# List of source files;
set( CXXSOURCES
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
	 
