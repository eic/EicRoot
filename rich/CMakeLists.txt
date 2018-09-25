# Create a library called "libRich" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES} 
${Boost_INCLUDE_DIR}
${ROOT_INCLUDE_DIR} 

${CMAKE_SOURCE_DIR}/rich
${CMAKE_SOURCE_DIR}/rich/fitter
${CMAKE_SOURCE_DIR}/rich/finder
#${CMAKE_SOURCE_DIR}/rich/qa
${CMAKE_SOURCE_DIR}/rich/tracks
${CMAKE_SOURCE_DIR}/rich/selection

${CMAKE_SOURCE_DIR}/cbmbase 
${CMAKE_SOURCE_DIR}/cbmbase/report
${CMAKE_SOURCE_DIR}/cbmbase/draw

${CMAKE_SOURCE_DIR}/cbmdata
${CMAKE_SOURCE_DIR}/cbmdata/rich
${CMAKE_SOURCE_DIR}/cbmdata/global

${GEANT3_INCLUDE_DIR}
${CMAKE_SOURCE_DIR}/geane
${CMAKE_SOURCE_DIR}/trackbase
${CMAKE_SOURCE_DIR}/pnddata
${CMAKE_SOURCE_DIR}/pnddata/TrackData

${CMAKE_SOURCE_DIR}/littrack

${CMAKE_SOURCE_DIR}/KF
${CMAKE_SOURCE_DIR}/KF/Interface

${CMAKE_SOURCE_DIR}/L1/OffLineInterface

${CMAKE_SOURCE_DIR}/eic/cad
)

If(TBB_FOUND)
  Set(INCLUDE_DIRECTORIES 
      ${INCLUDE_DIRECTORIES}
      ${TBB_INCLUDE_DIRS}
     )
EndIf(TBB_FOUND)

include_directories( ${INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
)

If(TBB_FOUND)
  Set(LINK_DIRECTORIES
      ${LINK_DIRECTORIES} 
      ${TBB_LIBRARY_DIRS}
     )
EndIf(TBB_FOUND)

link_directories( ${LINK_DIRECTORIES})

set(SRCS
CbmGeoRich.cxx 
CbmRich.cxx 
CbmRichHitProducer.cxx   
CbmRichEventDisplay.cxx   
CbmGeoRichPar.cxx 
CbmRichContFact.cxx
CbmRichMatchRings.cxx
CbmRichElectronIdAnn.cxx
CbmRichTrainAnnSelect.cxx
CbmRichTrainAnnElectrons.cxx
CbmRichReconstruction.cxx
CbmRichConverter.cxx
finder/CbmRichRingFinderIdeal.cxx 
#finder/CbmRichRingFinderHough.cxx
finder/CbmRichRingFinderHoughImpl.cxx
fitter/CbmRichRingFitterCircle.cxx  
fitter/CbmRichRingFitterCOP.cxx  
fitter/CbmRichRingFitterRobustCOP.cxx 
fitter/CbmRichRingFitterTAU.cxx 
fitter/CbmRichRingFitterEllipseMinuit.cxx
fitter/CbmRichRingFitterEllipseTau.cxx
tracks/CbmRichProjectionProducer.cxx 
tracks/CbmRichRingTrackAssignClosestD.cxx
tracks/CbmRichRingTrackAssignIdeal.cxx  
tracks/CbmRichTrackExtrapolationIdeal.cxx
tracks/CbmRichTrackExtrapolationKF.cxx
tracks/CbmRichTrackExtrapolationMirrorIdeal.cxx
qa/CbmRichTestSim.cxx  
qa/CbmRichTestHits.cxx 
qa/CbmRichRingFitterQa.cxx
selection/CbmRichRingSelectAnn.cxx
qa/CbmRichGeoTest.cxx 
qa/CbmRichUrqmdTest.cxx
qa/CbmRichGeoTestStudyReport.cxx 
)

If(TBB_FOUND)
  Set(SRCS 
      ${SRCS}
      finder/CbmRichRingFinderHoughParallel.cxx
     )
EndIf(TBB_FOUND)


set(HEADERS
CbmGeoRich.h
CbmRich.h
CbmRichHitProducer.h  
CbmRichEventDisplay.h  
CbmGeoRichPar.h
CbmRichContFact.h
CbmRichMatchRings.h
CbmRichTrainAnnSelect.h
CbmRichTrainAnnElectrons.h
CbmRichReconstruction.h
qa/CbmRichTestSim.h 
qa/CbmRichTestHits.h 
qa/CbmRichRingFitterQa.h 
finder/CbmRichRingFinderHough.h
prototype/CbmRichProtProjectionProducer.h
qa/CbmRichGeoTest.h 
qa/CbmRichUrqmdTest.h
)

If(TBB_FOUND)
  Set(HEADERS 
      ${HEADERS}
      finder/CbmRichRingFinderHoughParallel.h
     )
EndIf(TBB_FOUND)

IF (SSE_FOUND)
  ADD_DEFINITIONS(-DHAVE_SSE)
  SET_SOURCE_FILES_PROPERTIES(${SRCS} PROPERTIES COMPILE_FLAGS "-msse -O3")
  Message(STATUS "Rich will be compiled with SSE support")
ELSE (SSE_FOUND)
  Message(STATUS "Rich will be compiled without SSE support")
  SET_SOURCE_FILES_PROPERTIES(${SRCS} PROPERTIES COMPILE_FLAGS "-O3")
ENDIF (SSE_FOUND)

# END SSE

#If(TBB_FOUND)
#  set(LINKDEF  RichLinkDef_tbb.h)
#Else(TBB_FOUND)
#  set(LINKDEF  RichLinkDef.h)
#EndIf(TBB_FOUND)

Set(LIBRARY_NAME Rich)
Set(DEPENDENCIES
    # KF L1 ????
    #@@KF CbmBase CbmData Base MLP
    CbmBase CbmData
)

If(TBB_FOUND)
  Set(DEPENDENCIES
      ${DEPENDENCIES} ${TBB_LIBRARIES}
  )    
EndIf(TBB_FOUND)

GENERATE_LIBRARY()
