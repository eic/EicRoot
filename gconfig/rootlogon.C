
// rename, because of th econflict with VMC g4libs.C;
Bool_t isLibraryLocal(const char* libName)
{
  if (TString(gSystem->DynamicPathName(libName, kTRUE)) != TString(""))
    return kTRUE;
  else  
    return kFALSE;
}  

void rootlogon()
{
  gROOT->Macro("$VMCWORKDIR/gconfig/basiclibs.C");
  // Do this more clean later;
  gSystem->Load("libEve");
  gSystem->Load("libEventDisplay");
  
  // Load Panda libraries
  if(isLibraryLocal("libDpmEvtGen"))gSystem->Load("libDpmEvtGen");
  if(isLibraryLocal("libpythia8"))gSystem->Load("libpythia8");
  if(isLibraryLocal("libFlukaResults"))gSystem->Load("libFlukaResults");
  if(isLibraryLocal("libFairTools"))gSystem->Load("libFairTools");
  if(isLibraryLocal("libFairDB"))gSystem->Load("libFairDB"); 
  if(isLibraryLocal("libGeoBase"))gSystem->Load("libGeoBase");
  if(isLibraryLocal("libParBase"))gSystem->Load("libParBase");
  if(isLibraryLocal("libBase"))gSystem->Load("libBase");
  if(isLibraryLocal("libPhotos")){// these three depend on each other
    gSystem->Load("libPhotos");
    if(isLibraryLocal("libEvtGen")){
      gSystem->Load("libEvtGen");
      if(isLibraryLocal("libEvtGenDirect"))gSystem->Load("libEvtGenDirect");
    }
  }
 
  if(isLibraryLocal("libPndBase"))gSystem->Load("libPndBase");
  if(isLibraryLocal("libGlobalTasks"))gSystem->Load("libGlobalTasks");
  if(isLibraryLocal("libTrkBase"))gSystem->Load("libTrkBase");
  if(isLibraryLocal("libPndData"))gSystem->Load("libPndData");
  if(isLibraryLocal("libgeneralTools"))gSystem->Load("libgeneralTools");
  if(isLibraryLocal("libbuffers"))gSystem->Load("libbuffers");

  // NB: this EicRoot-specific library should be loaded before PandaRoot-specific libField; 
  if(isLibraryLocal("libfield"))          gSystem->Load("libfield");
  //+if(isLibraryLocal("libField"))gSystem->Load("libField");

  if(isLibraryLocal("libPassive"))gSystem->Load("libPassive");
  if(isLibraryLocal("libGen"))gSystem->Load("libGen");
  if(isLibraryLocal("libPGen"))gSystem->Load("libPGen");
  //if(isLibraryLocal("libEmc"))gSystem->Load("libEmc");
  if(isLibraryLocal("libgenfit"))gSystem->Load("libgenfit");
  if(isLibraryLocal("libtrackrep"))gSystem->Load("libtrackrep");
  if(isLibraryLocal("libgenfitLSL"))gSystem->Load("libgenfitLSL");
  //if(isLibraryLocal("libgenfitGeane"))gSystem->Load("libgenfitGeane");
  if(isLibraryLocal("libgenfitRK"))gSystem->Load("libgenfitRK");
  //if(isLibraryLocal("libGeaneTrackRep"))gSystem->Load("libGeaneTrackRep");
  if(isLibraryLocal("libgenfitAdapters"))gSystem->Load("libgenfitAdapters");
  if(isLibraryLocal("libriemann"))gSystem->Load("libriemann");
  //if(isLibraryLocal("libStt"))gSystem->Load("libStt");
  //if(isLibraryLocal("libSds"))gSystem->Load("libSds");
  //if(isLibraryLocal("libSdsReco"))gSystem->Load("libSdsReco");
  //if(isLibraryLocal("libMvd"))gSystem->Load("libMvd");
  //if(isLibraryLocal("libMvdReco"))gSystem->Load("libMvdReco");
  //if(isLibraryLocal("libMvdTrk"))gSystem->Load("libMvdTrk");
  //if(isLibraryLocal("libSttMvdTracking"))gSystem->Load("libSttMvdTracking");
  if(isLibraryLocal("libTracking"))gSystem->Load("libTracking");
  if(isLibraryLocal("libGem"))gSystem->Load("libGem");
  //if(isLibraryLocal("libFts"))gSystem->Load("libFts");
  //if(isLibraryLocal("libSciT"))gSystem->Load("libSciT");
  //if(isLibraryLocal("libDrcProp"))gSystem->Load("libDrcProp");
  //if(isLibraryLocal("libDrc"))gSystem->Load("libDrc");
  if(isLibraryLocal("libRich"))gSystem->Load("libRich");
  //if(isLibraryLocal("libMdt"))gSystem->Load("libMdt");
  if(isLibraryLocal("libGeane"))gSystem->Load("libGeane");
  //if(isLibraryLocal("libLmd"))gSystem->Load("libLmd");
  //if(isLibraryLocal("libRho"))gSystem->Load("libRho");
  if(isLibraryLocal("libTMVA"))gSystem->Load("libTMVA.so");
  if(isLibraryLocal("libAnalysisTools"))gSystem->Load("libAnalysisTools"); 
  if(isLibraryLocal("libPid"))gSystem->Load("libPid");
  if(isLibraryLocal("libRecoHits"))gSystem->Load("libRecoHits");
  if(isLibraryLocal("libRecoTasks"))gSystem->Load("libRecoTasks");
  //if(isLibraryLocal("libEnDrc"))gSystem->Load("libEnDrc");
  //if(isLibraryLocal("libDsk"))gSystem->Load("libDsk");
  if(isLibraryLocal("libGlobal"))gSystem->Load("libGlobal");
  if(isLibraryLocal("libMCMatch"))gSystem->Load("libMCMatch");
  //if(isLibraryLocal("libMva"))gSystem->Load("libMva");
  //if(isLibraryLocal("libFtof"))gSystem->Load("libFtof");

  //if(isLibraryLocal("libCLHEP"))gSystem->Load("libCLHEP");

  //if(isLibraryLocal("libTpcBase"))gSystem->Load("libTpcBase");
  //if(isLibraryLocal("libTpcRecoBase"))gSystem->Load("libTpcRecoBase");
  //if(isLibraryLocal("libTpcPar"))gSystem->Load("libTpcPar"); 
  //if(isLibraryLocal("libTpcANA"))gSystem->Load("libTpcANA"); 
  //if(isLibraryLocal("libTpcTools"))gSystem->Load("libTpcTools"); 
  //if(isLibraryLocal("libTpcMC"))gSystem->Load("libTpcMC");
  //if(isLibraryLocal("libTpcDigi"))gSystem->Load("libTpcDigi");
  //+if(isLibraryLocal("librecotasks"))gSystem->Load("librecotasks");
  //if(isLibraryLocal("libTpcReco"))gSystem->Load("libTpcReco");
  //if(isLibraryLocal("libTpcFOPI"))gSystem->Load("libTpcFOPI");
  //if(isLibraryLocal("libTpcBase")) { printf("Ku!\n"); exit(0); };
  //if(isLibraryLocal("libMva")) { printf("Ku!\n"); exit(0); };
  

  // EIC-specific libraries;
  if(isLibraryLocal("libeicsmear"))       gSystem->Load("libeicsmear");

  if(isLibraryLocal("libeicbase"))        gSystem->Load("libeicbase");
  if(isLibraryLocal("libhtc"))            gSystem->Load("libhtc");
  if(isLibraryLocal("libhtree"))          gSystem->Load("libhtree");
  //if(isLibraryLocal("libcad"))         gSystem->Load("libcad");
  //+if(isLibraryLocal("libtracking"))       gSystem->Load("libtracking");
  if(isLibraryLocal("libcalorimetry"))    gSystem->Load("libcalorimetry");
  if(isLibraryLocal("libsit"))            gSystem->Load("libsit");
  if(isLibraryLocal("libmaps"))           gSystem->Load("libmaps");
  if(isLibraryLocal("libgem"))            gSystem->Load("libgem");
  if(isLibraryLocal("libmumegas"))        gSystem->Load("libmumegas");
  if(isLibraryLocal("libtpc"))            gSystem->Load("libtpc");
  if(isLibraryLocal("libcalorimeter"))    gSystem->Load("libcalorimeter");
  if(isLibraryLocal("liblqst"))           gSystem->Load("liblqst");
  if(isLibraryLocal("libinfrastructure")) gSystem->Load("libinfrastructure");
  //gSystem->Load("libMemStat");
}
  
