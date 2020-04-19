
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}

${CMAKE_SOURCE_DIR}/pnddata 
${CMAKE_SOURCE_DIR}/pnddata/TrackData
${CMAKE_SOURCE_DIR}/trackbase
${CMAKE_SOURCE_DIR}/PndTools/generalTools
)

# List of source files;
set( CXXSOURCES
PndFileNameCreator.cxx
PndStringSeparator.cxx
PndGeoHandling.cxx
PndMcCloner.cxx
PndSensorNameContFact.cxx
PndSensorNamePar.cxx
PndBlackBoxTask.cxx
PndEventCounterTask.cxx
PndTrackArrayMerger.cxx
)

SET(DEPENDENCIES Base ParBase PndData)
GENERATE_EICROOT_LIBRARY(generalTools generalToolsLinkDef.h)
