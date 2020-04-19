
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}

${GEANT3_INCLUDE_DIR}

${CMAKE_SOURCE_DIR}/trackbase 
${CMAKE_SOURCE_DIR}/geane
)

# List of source files;
set( CXXSOURCES
FairGeane.cxx       
FairGeanePro.cxx
)

SET(DEPENDENCIES Base TrkBase Geom VMC EG Physics Core)
GENERATE_EICROOT_LIBRARY(Geane GeaneLinkDef.h)
