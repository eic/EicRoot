
set(INCLUDE_DIRECTORIES
 ${ROOT_INCLUDE_DIR} 
  ${PROJECT_SOURCE_DIR}/fairtools
)

#include_directories( ${INCLUDE_DIRECTORIES} )

# List of source files;
set( SOURCES
   FairLogger.cxx
)

GENERATE_EICROOT_LIBRARY(FairTools FairToolsLinkDef.h)
	 
