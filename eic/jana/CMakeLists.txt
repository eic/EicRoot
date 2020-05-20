
set(INCLUDE_DIRECTORIES

${CMAKE_SOURCE_DIR}/base
${CMAKE_SOURCE_DIR}/pnddata
${CMAKE_SOURCE_DIR}/pnddata/PidData
${CMAKE_SOURCE_DIR}/pnddata/TrackData
${CMAKE_SOURCE_DIR}/trackbase

${CMAKE_SOURCE_DIR}/field
${CMAKE_SOURCE_DIR}/parbase
${CMAKE_SOURCE_DIR}/geobase
${CMAKE_SOURCE_DIR}/fairtools
${CMAKE_SOURCE_DIR}/PndTools/generalTools
${CMAKE_SOURCE_DIR}/GenfitTools/recotasks
${CMAKE_SOURCE_DIR}/pid/PidCorr
${CMAKE_SOURCE_DIR}/geane
${CMAKE_SOURCE_DIR}/genfit

${CMAKE_SOURCE_DIR}/external/geant3/TGeant3

${CMAKE_SOURCE_DIR}/eic/base
${CMAKE_SOURCE_DIR}/eic/tracking

${JANA}/src/lib
${CMAKE_SOURCE_DIR}/eic/jana
)

include_directories( ${INCLUDE_DIRECTORIES} )

SET(JANA_PLUGIN eicroot)

# List of source files;
set( CXXSOURCES 
EicJanaPlugin.cc
EicJEventSource.cc
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

ADD_LIBRARY( ${JANA_PLUGIN} SHARED ${CXXSOURCES}
 
EicJEventProcessor.h
EicJEventSource.h
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

TARGET_LINK_LIBRARIES(${JANA_PLUGIN} ${JANA}/lib/libJANA.a tracking Pid ${ROOT_LIBRARIES} )

#set_target_properties(${JANA_PLUGIN} PREFIX "" SUFFIX ".so")
INSTALL(TARGETS ${JANA_PLUGIN} DESTINATION lib)

# ----------------

#SET(DEPENDENCIES eicbase /scratch/JANA2/build/libjana2.a ${ROOT_LIBRARIES} ${CMAKE_DL_LIBS} Threads::Threads /scratch/EicRoot/build/lib/libtracking.so /scratch/EicRoot/build/lib/libPid.so)
#GENERATE_EICROOT_LIBRARY(ejana ejanaLinkDef.h)
	 
