
set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 
${CMAKE_SOURCE_DIR}/fairtools
${CMAKE_SOURCE_DIR}/geobase
)

#include_directories( ${INCLUDE_DIRECTORIES} )

# List of source files;
set( CXXSOURCES
FairGeoAssembly.cxx
FairGeoCone.cxx	
FairGeoCons.cxx 
FairGeoEltu.cxx 
FairGeoBuilder.cxx  
FairGeoInterface.cxx  
FairGeoIo.cxx  
FairGeoLoader.cxx  
FairGeoMatrix.cxx 
FairGeoMedia.cxx	 
FairGeoMedium.cxx 
FairGeoNode.cxx 
FairGeoOldAsciiIo.cxx 
FairGeoPcon.cxx  
FairGeoPgon.cxx 
FairGeoRootBuilder.cxx  
FairGeoRotation.cxx 
FairGeoSet.cxx  
FairGeoShapes.cxx 
FairGeoSphe.cxx  
FairGeoTransform.cxx 
FairGeoTrap.cxx	
FairGeoTrd1.cxx 
FairGeoTube.cxx  
FairGeoTubs.cxx  
FairGeoTorus.cxx 
FairGeoVector.cxx  
FairGeoVolume.cxx 
FairGeoCompositeVolume.cxx 
FairGeoAsciiIo.cxx	
FairGeoBrik.cxx 
FairGeoBasicShape.cxx 
)

SET(DEPENDENCIES Geom Graf3d Core)
GENERATE_EICROOT_LIBRARY(GeoBase GeoBaseLinkDef.h)
	 
