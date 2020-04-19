
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}
 
${CMAKE_SOURCE_DIR}/trackbase 
${CMAKE_SOURCE_DIR}/pnddata
${CMAKE_SOURCE_DIR}/field
${CMAKE_SOURCE_DIR}/PndTools/generalTools

${CMAKE_SOURCE_DIR}/eic/ayk
${CMAKE_SOURCE_DIR}/eic/base
${CMAKE_SOURCE_DIR}/eic/field
${CMAKE_SOURCE_DIR}/eic/htc
${CMAKE_SOURCE_DIR}/eic/tracking

${CMAKE_SOURCE_DIR}/eic/htree
)

# List of source files;
set( CXXSOURCES
HoughNodeGroup.cxx
FwdHoughNodeGroup.cxx
HoughCell.cxx
ResolutionLevel.cxx
MatchCandidate.cxx
MatchCandidateGroup.cxx
HoughTree.cxx
FwdHoughTree.cxx
FwdKalmanFilter.cxx
FwdIdealTrackingCode.cxx
FwdMatchCandidate.cxx
FwdTrackFinder.cxx
FwdMcTrackMapper.cxx
)

SET(DEPENDENCIES eicbase htc)
GENERATE_EICROOT_LIBRARY(htree htreeLinkDef.h)

