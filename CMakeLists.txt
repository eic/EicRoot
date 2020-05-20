# ----------------------------------------------------------------------------------------------
#
#    A minimal installation in the current 'build' directory; be aware that either GEANT3-VMC
#  or GEANT4-VMC are needed to actually run any simulation scripts;
#
# cmake -DCMAKE_INSTALL_PREFIX=. -Wno-dev ..
#
#  cmake options:
#
# -DCAD2ROOT=yes
# -DHTC=yes
#
# -DG3VMC=<path-to-geant3-vmc-installation>
# -DG4VMC=<path-to-geant4-vmc-installation>
# -DEICSMEAR=<path-to-eic-smear-installation>
# -DJANA=<path-to-jana-installation>
#
# ----------------------------------------------------------------------------------------------
#
#   It is assumed that ROOT, GEANT4, GEANT3, VMC(G4) are pre-installed, and also 
# that respective thisroot.sh and geant4.sh files are sourced, like
#
# . /scratch/root-6.20.04.install/bin/thisroot.sh
# . /scratch/geant4.10.05.p01.build/bin/geant4.sh 
#
# ----------------------------------------------------------------------------------------------
#
#   Legacy, examples and expert stuff;
#
# -DCMAKE_C_COMPILER=/DATA00/ayk/local/bin/gcc -DCMAKE_CXX_COMPILER=/DATA00/ayk/local/bin/g++ 
# -DOPENCASCADE=/DATA00/ayk/oce/install
# -DJANA=/scratch/JANA2/Linux_CentOS7-x86_64-gcc4.8.5 ..
#
# cmake -DCMAKE_INSTALL_PREFIX=. -Wno-dev -DG3VMC=/scratch -DEICSMEAR=/scratch -DG4VMC=/scratch ..
#
# ----------------------------------------------------------------------------------------------

PROJECT(EICROOT)

# Should suffice?; for sure does not work with 2.8.10;
cmake_minimum_required(VERSION 3.5.2)

SET(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Very few command line options left;
SET(VARS EICSMEAR CBMROOT CAD2ROOT OPENCASCADE JANA G3VMC G4VMC HTC)
foreach (var ${VARS})
  IF(DEFINED ${var})
    message("-> " ${var} " is defined to " ${${var}})
  ENDIF()
  IF(NOT DEFINED ${var})
    message("!! " ${var} " is not defined!")
  ENDIF()		
endforeach(var)

# Assume ROOTSYS was set up in the environment;
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

# Find ROOT;
FIND_PACKAGE(ROOT 6.0.0 REQUIRED COMPONENTS RIO Tree Hist)
#---Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})

# Keep this original stuff activated for config.sh creation, etc;
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")
include(FairMacros)
include(WriteConfigFile)
SetBasicVariables()

#find_package(GEANT3 REQUIRED)
#find_package(GEANT4)
#find_package(GEANT4DATA)
#find_package(GEANT4VMC)

# Basically just a simplified GENERATE_LIBRARY() macro;
include(eicroot)

# FIXME: a bit too much of the warnings disabled?;
SET( CMAKE_CXX_FLAGS "-g -Wall -Wno-deprecated-declarations -Wno-parentheses -Wno-comment -Wno-unused -Wno-sign-compare -Wno-reorder -Wno-format -std=c++11 -Wno-narrowing -Wno-address -pthread -fPIC" )

if (DEFINED G3VMC)
  set (GEANT3_INCLUDE_DIR ${G3VMC}/include/TGeant3)
endif()

# FairRoot;
add_subdirectory(fairtools)
add_subdirectory(dbase)
add_subdirectory(parbase)
add_subdirectory(geobase)
add_subdirectory(base)
add_subdirectory(trackbase)
add_subdirectory(eventdisplay)
add_subdirectory(geane)
#add_subdirectory(generators)

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
  add_subdirectory(cbmdata)
  # FIXME: this stuff requires Boost; 
  #@@add_subdirectory(cbmbase)
  #@@add_subdirectory(rich)
endif()

# EicRoot, eventually;
add_subdirectory(eic/ayk)
if (DEFINED EICSMEAR) 
  # Well, this is only needed to disable part of the codes in FluxMonitorGrid::FillOutHistograms();
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_EICSMEAR_")

  add_subdirectory(eic/event) 
endif()
add_subdirectory(eic/proto)
add_subdirectory(eic/base)
add_subdirectory(eic/field)
if (DEFINED CAD2ROOT)
  # Well, OpenCascade is not needed to import either STL or COMSOL .mphtxt or already
  # converted ROOT TGeo files -> optional;
  if(DEFINED OPENCASCADE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_OPENCASCADE_")

    add_subdirectory(eic/cad/wizard)
  endif()

  add_subdirectory(eic/cad)
endif()
add_subdirectory(eic/tracking)
add_subdirectory(eic/calorimetry)
add_subdirectory(eic/detectors/tpc)
add_subdirectory(eic/detectors/maps)
add_subdirectory(eic/detectors/lqst)
add_subdirectory(eic/detectors/calorimeter)
add_subdirectory(eic/detectors/gem)
add_subdirectory(eic/detectors/mumegas)
add_subdirectory(eic/infrastructure)
if (DEFINED HTC)
  add_subdirectory(eic/htc)
  add_subdirectory(eic/htree)
endif()

if (DEFINED JANA)
  add_subdirectory(eic/jana)
endif()

WRITE_CONFIG_FILE(config.sh)
WRITE_CONFIG_FILE(config.csh)
