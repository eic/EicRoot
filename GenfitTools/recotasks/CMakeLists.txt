
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}

${CMAKE_SOURCE_DIR}/GenfitTools/trackrep
${CMAKE_SOURCE_DIR}/GenfitTools/trackrep/GeaneTrackRep
${CMAKE_SOURCE_DIR}/GenfitTools/trackrep/RKTrackRep
${CMAKE_SOURCE_DIR}/genfit
${CMAKE_SOURCE_DIR}/trackbase
${CMAKE_SOURCE_DIR}/stt
${CMAKE_SOURCE_DIR}/fts
${CMAKE_SOURCE_DIR}/pnddata
${CMAKE_SOURCE_DIR}/pnddata/TrackData
${CMAKE_SOURCE_DIR}/PndTools/generalTools
${CMAKE_SOURCE_DIR}/geane
${CMAKE_SOURCE_DIR}/GenfitTools/adapters
${CMAKE_SOURCE_DIR}/GenfitTools/recohits
${CMAKE_SOURCE_DIR}/GenfitTools/recotasks

${GEANT3_INCLUDE_DIR}

# Need EicMoCaPoint.h (which has no extra dependencies);
${CMAKE_SOURCE_DIR}/eic/base
)

# List of source files;
set( CXXSOURCES
PndRecoKalmanFit.cxx
PndRecoDafFit.cxx
PndRecoKalmanTask.cxx
PndRecoMultiKalmanTask.cxx
PrimSelector.cxx
PndGFTrackToPndTrackConvertorTask.cxx
)

SET(DEPENDENCIES Base ParBase PndData generalTools TrkBase Geane RecoHits genfitAdapters)
GENERATE_EICROOT_LIBRARY(RecoTasks RecoTasksLinkDef.h)
	
