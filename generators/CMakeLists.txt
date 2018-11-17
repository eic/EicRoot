
set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR}
${PLUTO_INCLUDE_DIR} 
${CMAKE_SOURCE_DIR}/fairtools
${CMAKE_SOURCE_DIR}/geobase 
${CMAKE_SOURCE_DIR}/field 
${CMAKE_SOURCE_DIR}/base 
${CMAKE_SOURCE_DIR}/generators
)

#include_directories( ${INCLUDE_DIRECTORIES} )

# List of source files;
set( CXXSOURCES
FairAsciiGenerator.cxx    
FairBoxGenerator.cxx      
FairIonGenerator.cxx      
FairParticleGenerator.cxx 
FairShieldGenerator.cxx
# Enable back once figure out how not to use -ffriend-injection flag under Mac OS;
# NB: also do not forget to uncomment in linkdef file;   
#@@@FairPlutoGenerator.cxx    
#@@@FairPlutoReactionGenerator.cxx    
FairUrqmdGenerator.cxx    
FairEvtGenGenerator.cxx
)

SET(DEPENDENCIES Base EG MathCore Physics RIO Core)
GENERATE_EICROOT_LIBRARY(Gen GenLinkDef.h)
	 
