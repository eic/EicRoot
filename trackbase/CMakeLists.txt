
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}

${CMAKE_SOURCE_DIR}/field 
${CMAKE_SOURCE_DIR}/passive
${CMAKE_SOURCE_DIR}/genfit
${CMAKE_SOURCE_DIR}/trackbase
)

# List of source files;
set( CXXSOURCES
FairTrackPar.cxx
FairTrackParP.cxx
FairTrackParH.cxx 
FairGeaneUtil.cxx
)

SET(DEPENDENCIES Base Physics Matrix Core)
GENERATE_EICROOT_LIBRARY(TrkBase TrackBaseLinkDef.h)
