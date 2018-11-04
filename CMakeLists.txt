#
# FIXME: perhaps find easier way to handle ROOT5/ROOT6 installations; see also 
# eicroot.cmake; NB: as it stands on 2018/09/25 ROOT6 version compiles but 
# crashes in digitization.C scripts;
#

PROJECT(EICROOT)

# Should suffice?;
cmake_minimum_required(VERSION 2.8.10)

# Very few command line options left;
SET(VARS EICSMEAR CBMROOT)
foreach (var ${VARS})
  IF(DEFINED ${var})
    message("-> " ${var} " is defined to " ${${var}})
  ENDIF()
  IF(NOT DEFINED ${var})
    message("!! " ${var} " is not defined!")
  ENDIF()		
endforeach(var)

if (ROOT5)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")
endif (ROOT5)
# Assume ROOTSYS was setup in the environment;
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

IF(NOT DEFINED ENV{SIMPATH})
   MESSAGE(FATAL_ERROR 
   "You did not define the environment variable SIMPATH which is needed to find "
   "the external packages. Please set this variable and execute cmake once again.") 
ENDIF(NOT DEFINED ENV{SIMPATH})
STRING(REGEX MATCHALL "[^:]+" PATH $ENV{PATH})
SET(SIMPATH $ENV{SIMPATH})

# Find ROOT;
if (ROOT5)
FIND_PACKAGE(ROOT 5.34.0 REQUIRED COMPONENTS RIO Tree Hist)
else (ROOT5)
FIND_PACKAGE(ROOT 6.0.0 REQUIRED COMPONENTS RIO Tree Hist)
#---Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})
endif (ROOT5)

# Keep this original stuff activated for config.sh creation, etc;
if (NOT ROOT5)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")
endif (ROOT5)
include(FairMacros)
include(WriteConfigFile)
SetBasicVariables()

find_package(GEANT3 REQUIRED)
find_package(GEANT4)
find_package(GEANT4DATA)
find_package(GEANT4VMC)

SET(EICLIBDIR ${CMAKE_BINARY_DIR}/lib)
SET(LD_LIBRARY_PATH ${EICLIBDIR} ${LD_LIBRARY_PATH})

# Basically just a simplified GENERATE_LIBRARY() macro;
include(eicroot)

SET( CMAKE_CXX_FLAGS "-g -Wall -Wno-deprecated-declarations -Wno-parentheses -Wno-comment -Wno-unused -Wno-sign-compare -Wno-reorder -Wno-format -std=c++11 -Wno-narrowing" )
#if (NOT ROOT5)
#SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-narrowing")
#endif (ROOT5)

# FairRoot;
add_subdirectory(fairtools)
add_subdirectory(dbase)
add_subdirectory(parbase)
add_subdirectory(geobase)
add_subdirectory(base)
add_subdirectory(trackbase)
add_subdirectory(eventdisplay)
add_subdirectory(geane)
add_subdirectory(generators)

# GenFit;
add_subdirectory(genfit)
add_subdirectory(GenfitTools/trackrep/RKTrackRep)
add_subdirectory(GenfitTools/trackrep/LSLtrackRep)

# GenFit + PandaRoot;
add_subdirectory(GenfitTools/trackrep)

# PandaRoot;
add_subdirectory(passive)
add_subdirectory(field)
add_subdirectory(pnddata)
add_subdirectory(PndTools/generalTools)
add_subdirectory(GenfitTools/adapters)
add_subdirectory(GenfitTools/recohits)
add_subdirectory(GenfitTools/recotasks)
add_subdirectory(pid)

# CbmRoot (RICH);
if (DEFINED CBMROOT)
  #@@add_subdirectory(cbmdata)
  #@@add_subdirectory(cbmbase)
  #@@add_subdirectory(rich)
endif()

# EicRoot, eventually;
add_subdirectory(eic/ayk)
add_subdirectory(eic/base)
add_subdirectory(eic/field)
#@@if (DEFINED CAD2ROOT)
#@@  ###add_subdirectory(eic/cad)
#@@endif()
add_subdirectory(eic/tracking)
add_subdirectory(eic/htc)
add_subdirectory(eic/calorimetry)
add_subdirectory(eic/detectors/tpc)
add_subdirectory(eic/detectors/maps)
add_subdirectory(eic/detectors/lqst)
add_subdirectory(eic/detectors/calorimeter)
add_subdirectory(eic/detectors/gem)
add_subdirectory(eic/detectors/mumegas)
add_subdirectory(eic/infrastructure)
#@@add_subdirectory(eic/htree)

WRITE_CONFIG_FILE(config.sh)
WRITE_CONFIG_FILE(config.csh)
