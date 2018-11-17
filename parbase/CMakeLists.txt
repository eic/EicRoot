
set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR}
${CMAKE_SOURCE_DIR}/fairtools
${CMAKE_SOURCE_DIR}/base
${CMAKE_SOURCE_DIR}/geobase
${CMAKE_SOURCE_DIR}/parbase
${CMAKE_SOURCE_DIR}/parbase/dbIO
${CMAKE_SOURCE_DIR}/dbase/
${CMAKE_SOURCE_DIR}/dbase/dbInterface
${CMAKE_SOURCE_DIR}/dbase/dbValidation
)

#include_directories( ${INCLUDE_DIRECTORIES} )

# List of source files;
set( CXXSOURCES
FairContFact.cxx       
FairDetParAsciiFileIo.cxx    
FairDetParIo.cxx         
FairDetParRootFileIo.cxx     
FairGenericParAsciiFileIo.cxx     
FairGenericParRootFileIo.cxx     
FairParAsciiFileIo.cxx   
FairParGenericSet.cxx   
FairParIo.cxx  
FairParRootFileIo.cxx 
FairParSet.cxx 
FairParamList.cxx  
FairRtdbRun.cxx  
FairRuntimeDb.cxx
###### Fair DB IO #######
dbIO/FairParTSQLIo.cxx
dbIO/FairDetParTSQLIo.cxx
dbIO/FairGenericParTSQLIo.cxx
)

SET(DEPENDENCIES FairTools RIO Core FairDB)
GENERATE_EICROOT_LIBRARY(ParBase ParBaseLinkDef.h)
	 
