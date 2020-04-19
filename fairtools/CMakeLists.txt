
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}
)

# List of source files;
set( CXXSOURCES
   FairLogger.cxx
)

GENERATE_EICROOT_LIBRARY(FairTools FairToolsLinkDef.h)
	 
