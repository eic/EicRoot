
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}

${CMAKE_SOURCE_DIR}/GenfitTools/trackrep
${CMAKE_SOURCE_DIR}/GenfitTools/trackrep/GeaneTrackRep
${CMAKE_SOURCE_DIR}/GenfitTools/trackrep/RKTrackRep
${CMAKE_SOURCE_DIR}/GenfitTools/trackrep/LSLtrackRep
${CMAKE_SOURCE_DIR}/genfit
${CMAKE_SOURCE_DIR}/trackbase
${CMAKE_SOURCE_DIR}/pnddata
${CMAKE_SOURCE_DIR}/pnddata/TrackData
${CMAKE_SOURCE_DIR}/geane

${GEANT3_INCLUDE_DIR}

${CMAKE_SOURCE_DIR}/GenfitTools/adapters
)

# List of source files;
set( CXXSOURCES
PndGenfitAdapters.cxx
PndGenfitField.cxx
)

SET(DEPENDENCIES PndData genfitRK trackrep)
GENERATE_EICROOT_LIBRARY(genfitAdapters genfitAdaptersLinkDef.h)
