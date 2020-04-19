
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}

${GEANT3_INCLUDE_DIR}
 
${CMAKE_SOURCE_DIR}/trackbase 
${CMAKE_SOURCE_DIR}/pnddata
${CMAKE_SOURCE_DIR}/field
${CMAKE_SOURCE_DIR}/pnddata/TrackData
${CMAKE_SOURCE_DIR}/GenfitTools/trackrep/GeaneTrackRep
${CMAKE_SOURCE_DIR}/GenfitTools/recotasks
${CMAKE_SOURCE_DIR}/PndTools/generalTools
${CMAKE_SOURCE_DIR}/geane
${CMAKE_SOURCE_DIR}/genfit

${CMAKE_SOURCE_DIR}/eic/base
${CMAKE_SOURCE_DIR}/eic/ayk
${CMAKE_SOURCE_DIR}/eic/field
${CMAKE_SOURCE_DIR}/eic/htc
${CMAKE_SOURCE_DIR}/eic/tracking
)

# List of source files;
set( CXXSOURCES 
EicTrackingDigiHit.cxx 
EicTrackingDigiHitProducer.cxx

EicTrackingRecoHit.cxx 

EicIdealTrackingCode.cxx 
EicRecoKalmanTask.cxx
)

SET(DEPENDENCIES eicbase qfield genfit RecoTasks)
GENERATE_EICROOT_LIBRARY(tracking trackingLinkDef.h)
	 
