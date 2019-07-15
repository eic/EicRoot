
set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 
${GEANT3_INCLUDE_DIR}
${BASE_INCLUDE_DIRECTORIES} 
${CMAKE_SOURCE_DIR}/pndbase/PndStdUtils 
${CMAKE_SOURCE_DIR}/field 
${CMAKE_SOURCE_DIR}/trackbase 
${CMAKE_SOURCE_DIR}/passive
${CMAKE_SOURCE_DIR}/PndTools/generalTools
${CMAKE_SOURCE_DIR}/PndTools/AnalysisTools
${CMAKE_SOURCE_DIR}/emc/EmcMC
${CMAKE_SOURCE_DIR}/gem
${CMAKE_SOURCE_DIR}/genfit
${CMAKE_SOURCE_DIR}/pnddata
${CMAKE_SOURCE_DIR}/pnddata/TrackData
${CMAKE_SOURCE_DIR}/pnddata/PidData
${CMAKE_SOURCE_DIR}/geane
${CMAKE_SOURCE_DIR}/trackrep
${CMAKE_SOURCE_DIR}/rho
${CMAKE_SOURCE_DIR}/rho/RhoBase
${CMAKE_SOURCE_DIR}/pid
${CMAKE_SOURCE_DIR}/pid/PidData
${CMAKE_SOURCE_DIR}/pid/PidCorr
${CMAKE_SOURCE_DIR}/pid/PidClassifier
${CMAKE_SOURCE_DIR}/GenfitTools/recotasks

${CMAKE_SOURCE_DIR}/eic/tracking
)

#include_directories( ${INCLUDE_DIRECTORIES} )

# List of source files;
set( CXXSOURCES
PidCorr/PndPidCorrelator.cxx
PidCorr/PndPidTrackInfo.cxx
)

SET(DEPENDENCIES Base GeoBase ParBase RecoTasks generalTools Geane PndData TrkBase)
GENERATE_EICROOT_LIBRARY(Pid PidLinkDef.h)
