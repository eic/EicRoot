
set(INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}
)

# List of source files;
set( CXXSOURCES
FairBaseContFact.cxx 
FairBaseParSet.cxx
FairLink.cxx
FairMultiLinkedData.cxx 
FairDetector.cxx 
FairEventHeader.cxx 
FairFileHeader.cxx 
FairGenerator.cxx 
FairHit.cxx 
FairIon.cxx 
FairMCApplication.cxx 
FairGeaneApplication.cxx 
FairMCEventHeader.cxx 
FairMCPoint.cxx 
FairModule.cxx 
FairParticle.cxx 
FairPrimaryGenerator.cxx 
FairRootManager.cxx 
FairRun.cxx 
FairRunAna.cxx 
FairRunIdGenerator.cxx 
FairRunSim.cxx 
FairGenericStack.cxx 
FairTask.cxx 
FairTrackParam.cxx 
FairTrajFilter.cxx 
FairVolume.cxx 
FairVolumeList.cxx 
FairField.cxx
FairFieldFactory.cxx
FairRadLenPoint.cxx
FairRadMapManager.cxx
FairRadMapPoint.cxx
FairRadLenManager.cxx
FairRadGridManager.cxx
FairMesh.cxx
FairRKPropagator.cxx
FairTimeStamp.cxx
FairTSBufferFunctional.cxx
FairFileInfo.cxx
FairRunInfo.cxx
FairWriteoutBuffer.cxx
FairRingSorter.cxx
FairRingSorterTask.cxx
FairAnaSelector.cxx
)

SET(DEPENDENCIES FairTools ParBase GeoBase ${ROOT_LIBRARIES} Proof GeomPainter Geom VMC )
GENERATE_EICROOT_LIBRARY(Base FairLinkDef.h)
	 
