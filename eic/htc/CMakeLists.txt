
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}

${CMAKE_SOURCE_DIR}/field

${CMAKE_SOURCE_DIR}/eic/ayk
${CMAKE_SOURCE_DIR}/eic/base
${CMAKE_SOURCE_DIR}/eic/field
${CMAKE_SOURCE_DIR}/eic/tracking

${CMAKE_SOURCE_DIR}/eic/htc

${CMAKE_SOURCE_DIR}/trackbase
${CMAKE_SOURCE_DIR}/pnddata
${CMAKE_SOURCE_DIR}/pnddata/TrackData
${CMAKE_SOURCE_DIR}/PndTools/generalTools
)

# List of source files;
set( CXXSOURCES
EicHtcTask.cxx
SensitiveVolume.cxx

KalmanNode.cxx
TrKalmanFilter.cxx

geant.cxx
KalmanFilter.cxx
MediaLayer.cxx
MediaSlice.cxx
MediaBank.cxx
MediaSliceArray.cxx
misc.cxx
runge-kutta.cxx
RungeKuttaRequest.cxx
TrKalmanNode.cxx
TrKalmanNodeLocation.cxx
)

SET(DEPENDENCIES eicbase ayk qfield tracking ${G3VMC}/lib64/libgeant321.so)
GENERATE_EICROOT_LIBRARY(htc eichtcLinkDef.h)
	 
