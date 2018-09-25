# Check if cmake has the required version
cmake_minimum_required(VERSION 2.4.3 FATAL_ERROR)
       
# Project name                    
project(standalone)

# Where to look first for cmake modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")


#find_package(TBB)

#if(NOT TBB_FOUND)
#    MESSAGE (STATUS "TBB not found. ")
#else(NOT TBB_FOUND)
#	MESSAGE (STATUS "TBB found.")
#endif(NOT TBB_FOUND)

set(TBB_INCLUDE_DIRS "/u/andrey/soft/tbb/tbb22_009oss/include")
set(TBB_LIBRARIES "/u/andrey/soft/tbb/Lenny64/libtbb.so")
set(VC_INCLUDE_DIRS "/u/slebedev/Baykal/soft/Vc")
set(VC_LIBRARIES "/u/slebedev/Baykal/soft/Vc/build/libVc.a")


set(INCLUDE_DIRECTORIES
${PROJECT_SOURCE_DIR}/algorithm
${TBB_INCLUDE_DIRS}
${VC_INCLUDE_DIRS}
)

include_directories(${INCLUDE_DIRECTORIES})


set(LINK_DIRECTORIES
${TBB_LIBRARY_DIRS}
)

link_directories( ${LINK_DIRECTORIES})


set(STANDALONE_SRCS
main.cxx
algorithm/CbmRichRingFinderHough.cxx
algorithm/CbmRichRingFinderHoughImpl.cxx
algorithm/NNfunction.cxx
algorithm/CbmRichRingFinderHoughSimd.cxx
)

#set(STANDALONE_HEADERS
#)

IF (SSE_FOUND)
  ADD_DEFINITIONS(-DHAVE_SSE)
  SET_SOURCE_FILES_PROPERTIES(${STANDALONE_SRCS} PROPERTIES COMPILE_FLAGS "-msse -O3")
  Message(STATUS "Standalone Rich will be compiled with SSE support")
ELSE (SSE_FOUND)
  Message(STATUS "Standalone Rich will be compiled without SSE support")
  SET_SOURCE_FILES_PROPERTIES(${STANDALONE_SRCS} PROPERTIES COMPILE_FLAGS "-O3")
ENDIF (SSE_FOUND)


# Executable name
add_executable(standalone main.cxx)

add_library(parallel SHARED ${STANDALONE_SRCS})
target_link_libraries(standalone parallel)

target_link_libraries(parallel ${TBB_LIBRARIES})
target_link_libraries(parallel ${VC_LIBRARIES})
