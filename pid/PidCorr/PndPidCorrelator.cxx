#include <iostream>
using namespace std;

#include "PndDetectorList.h"
#include "PndPidCorrelator.h"
#include "PndTrackID.h"
#include "PndMCTrack.h"

#if _TODAY_
#include "PndTrack.h"

#include "PndSciTPoint.h"
#include "PndSciTHit.h"
#include "PndEmcBump.h"
#include "PndEmcDigi.h"
#include "PndEmcStructure.h"
#include "PndEmcXtal.h"
#include "PndEmcErrorMatrix.h"
#include "PndEmcClusterCalibrator.h"
#include "PndEmcClusterEnergySums.h"
#include "PndMdtPoint.h"
#include "PndMdtHit.h"
#include "PndMdtTrk.h"
#include "PndDrcBarPoint.h"
#include "PndDrcHit.h"
#include "PndDskParticle.h"
#include "FairTrackParH.h"
#include "FairMCApplication.h"
#include "FairRunAna.h"
#include "FairRootManager.h"
#include "FairRuntimeDb.h"

#include "TObjArray.h"
#include "TVector3.h"
#include "TGeoMatrix.h"
#include "TGeoManager.h"
#include "TSystem.h"

#include <cmath>
#endif

// ----------------------------------------------------------
// --- Interface with PidMaker and output ---

//___________________________________________________________
PndPidCorrelator::~PndPidCorrelator() 
{
  //
  FairRootManager *fManager =FairRootManager::Instance();
  fManager->Write();
#if _TODAY_
  delete fEmcErrorMatrix; 
#endif
}

//___________________________________________________________
PndPidCorrelator::PndPidCorrelator() : 
  FairTask(),
  fTrackBranch("EicIdealGenTrack"),
  fTrackIDBranch("EicIdealGenTrackID"), 
  fTrack(new TClonesArray()),
  fTrackOutBranch(""),
  fIdealHyp(kFALSE),
  fCorrErrorProp(kTRUE),
  fGeanePro(kTRUE),
  fMcTrack(new TClonesArray()),
  fPidHyp(0)


#if _TODAY_
, ,, fTrackID(new TClonesArray()), fTrack2(new TClonesArray()), fTrackID2(new TClonesArray()), fPidChargedCand(new TClonesArray()), fPidNeutralCand(new TClonesArray()), fMdtTrack(new TClonesArray()), fMvdHitsStrip(new TClonesArray()), fMvdHitsPixel(new TClonesArray()), fTofHit(new TClonesArray()), fTofPoint(new TClonesArray()), fFtofHit(new TClonesArray()), fFtofPoint(new TClonesArray()), fEmcCluster(new TClonesArray()), fEmcBump(new TClonesArray()), fEmcDigi(new TClonesArray()), fMdtPoint(new TClonesArray()), fMdtHit(new TClonesArray()), fMdtTrk(new TClonesArray()), fDrcPoint(new TClonesArray()), fDrcHit(new TClonesArray()), fDskParticle(new TClonesArray()), fSttHit(new TClonesArray()), fFtsHit(new TClonesArray()), 
  fCorrPar(new PndPidCorrPar()), fEmcGeoPar(new PndEmcGeoPar()), fEmcErrorMatrixPar(new PndEmcErrorMatrixPar()), fEmcErrorMatrix(new PndEmcErrorMatrix()), fSttParameters(new PndGeoSttPar()), fEmcCalibrator(NULL), fEmcClstCount(0), fFscClstCount(0),
  fDebugMode(kFALSE),
  fMdtRefit(kFALSE),
  fMvdMode(-1),
  fSttMode(-1),
  fFtsMode(-1),
  fTofMode(-1), 
  fFtofMode(-1),
  fEmcMode(-1),
  fMdtMode(-1), 
  fDrcMode(-1),
  fDskMode(-1),
  fMixMode(kFALSE),
  fFast(kFALSE),
  fSimulation(kFALSE),
  fIdeal(kFALSE), 
  tofCorr(0),
  emcCorr(0), 
  fscCorr(0),
  drcCorr(0),
  dskCorr(0),
  fTrackBranch2(""),
  fTrackIDBranch2(""),
  sDir(""),
  sFile(""),
  fGeoH(NULL),
  fClusterList(), 
  fClusterQ(),
  mapMdtBarrel(),
  mapMdtEndcap(),
  mapMdtForward()
#endif
{
  fPidChargedCand = new TClonesArray("PndPidCandidate");
  sDir = "./";
  sFile = "./pidcorrelator.root";
  fGeoH = PndGeoHandling::Instance();

#if _TODAY_
  //---
  fPidNeutralCand = new TClonesArray("PndPidCandidate");
  
  // Resetting MDT geometry parameters
  for (Int_t mm=0; mm<3; mm++)
    for (Int_t ll=0; ll<20;ll++)
      {
	mdtLayerPos[mm][ll] = -1;
	mdtIronThickness[mm][ll] = -1;
      }
  
#endif

  Reset();
}

#if _TODAY_
//___________________________________________________________
PndPidCorrelator::PndPidCorrelator(const char *name, const char *title) :
  FairTask(name),
  fMcTrack(new TClonesArray()), fTrack(new TClonesArray()), fTrackID(new TClonesArray()), fTrack2(new TClonesArray()), fTrackID2(new TClonesArray()), fPidChargedCand(new TClonesArray()), fPidNeutralCand(new TClonesArray()), fMdtTrack(new TClonesArray()), fMvdHitsStrip(new TClonesArray()), fMvdHitsPixel(new TClonesArray()), fTofHit(new TClonesArray()), fTofPoint(new TClonesArray()), fFtofHit(new TClonesArray()), fFtofPoint(new TClonesArray()), fEmcCluster(new TClonesArray()), fEmcBump(new TClonesArray()), fEmcDigi(new TClonesArray()), fMdtPoint(new TClonesArray()), fMdtHit(new TClonesArray()), fMdtTrk(new TClonesArray()), fDrcPoint(new TClonesArray()), fDrcHit(new TClonesArray()), fDskParticle(new TClonesArray()), fSttHit(new TClonesArray()), fFtsHit(new TClonesArray()), 
  fCorrPar(new PndPidCorrPar()), fEmcGeoPar(new PndEmcGeoPar()), fEmcErrorMatrixPar(new PndEmcErrorMatrixPar()), fEmcErrorMatrix(new PndEmcErrorMatrix()), fSttParameters(new PndGeoSttPar()), fEmcCalibrator(NULL), fEmcClstCount(0), fFscClstCount(0),
  fDebugMode(kFALSE),
  fGeanePro(kTRUE), 
  fMdtRefit(kFALSE),
  fMvdMode(-1),
  fSttMode(-1), 
  fFtsMode(-1),
  fTofMode(-1), 
  fFtofMode(-1),
  fEmcMode(-1),
  fMdtMode(-1), 
  fDrcMode(-1),
  fDskMode(-1),
  fMixMode(kFALSE),
  fPidHyp(0),
  fIdealHyp(kFALSE), 
  fFast(kFALSE),
  fSimulation(kFALSE),
  fIdeal(kFALSE), 
  fCorrErrorProp(kTRUE),
  tofCorr(0),
  emcCorr(0), 
  fscCorr(0),
  drcCorr(0),
  dskCorr(0),
  fTrackBranch(""),
  fTrackIDBranch(""),
  fTrackBranch2(""),
  fTrackIDBranch2(""),
  fTrackOutBranch(""),
  sDir(""),
  sFile(""),
  fGeoH(NULL),
  fClusterList(), 
  fClusterQ(),
  mapMdtBarrel(),
  mapMdtEndcap(),
  mapMdtForward()
{
  //---
  fPidChargedCand = new TClonesArray("PndPidCandidate");
  fPidNeutralCand = new TClonesArray("PndPidCandidate");
  sDir = "./";
  sFile = "./pidcorrelator.root";
  fGeoH = PndGeoHandling::Instance(); 
  
  // Resetting MDT geometry parameters
  for (Int_t mm=0; mm<3; mm++)
    for (Int_t ll=0; ll<20;ll++)
      {
	mdtLayerPos[mm][ll] = -1;
	mdtIronThickness[mm][ll] = -1;
      }
  
  Reset();
}
#endif

//___________________________________________________________
InitStatus PndPidCorrelator::Init() {
  //  cout << "InitStatus PndPidCorrelator::Init()" << endl;
  
  FairRootManager *fManager =FairRootManager::Instance();	
  
  fTrack = (TClonesArray *)fManager->GetObject(fTrackBranch);
  if ( ! fTrack ) {
    cout << "-I- PndPidCorrelator::Init: No PndTrack array!" << endl;
    return kERROR;
  }
  
  if (fTrackIDBranch!="")
    {
      fTrackID = (TClonesArray *)fManager->GetObject(fTrackIDBranch);
      if ( ! fTrackID ) {
	cout << "-I- PndPidCorrelator::Init: No PndTrackID array! Switching MC propagation OFF" << endl;
	fTrackIDBranch = "";
      }
    }
  
#if _TODAY_  
  if (fTrackBranch2!="")
    {
      fTrack2 = (TClonesArray *)fManager->GetObject(fTrackBranch2);
      if ( ! fTrack2 ) {
	cout << "-I- PndPidCorrelator::Init: No 2nd PndTrack array!" << endl;
	return kERROR;
      }
    }
  
  if (fTrackIDBranch2!="")
    {
      fTrackID2 = (TClonesArray *)fManager->GetObject(fTrackIDBranch2);
      if ( ! fTrackID2 ) {
	cout << "-I- PndPidCorrelator::Init: No 2nd PndTrackID array! Switching MC propagation OFF" << endl;
	fTrackIDBranch2 = "";
      }
    }
  
  // *** STT ***
  if (fSttMode)
    {
      if (fMixMode==kFALSE)
	{
	  fSttHit = (TClonesArray*) fManager->GetObject("STTHit");
	  if ( fSttHit ) 
	    {
	      cout << "-I- PndPidCorrelator::Init: Using STTHit" << endl;
	      fSttMode = 2;
	    }
	  else
	    {
	      cout << "-W- PndPidCorrelator::Init: No STT hits array! Switching STT OFF" << endl;
	      fSttMode = 0;
	    } 
	}
      else
	{
	  fSttHit = (TClonesArray*) fManager->GetObject("STTHitMix");
	  if ( fSttHit )
	    {
	      cout << "-I- PndPidCorrelator::Init: Using STTHitMix" << endl;
	      fSttMode = 2;
	    }
	  else
	    {
	      cout << "-W- PndPidCorrelator::Init: No STT hits mix array! Switching STT OFF" << endl;
	      fSttMode = 0;
	    }
	}
    }
  
  // *** FTS ***
  if (fFtsMode)
    {
      if (fMixMode==kFALSE)
	{
	  fFtsHit = (TClonesArray*) fManager->GetObject("FTSHit");
	  if ( fFtsHit ) 
	    {
	      cout << "-I- PndPidCorrelator::Init: Using FTSHit" << endl;
	      fFtsMode = 2;
	    }
	  else
	    {
	      cout << "-W- PndPidCorrelator::Init: No FTS hits array! Switching FTS OFF" << endl;
	      fFtsMode = 0;
	    } 
	}
      else
	{
	  fFtsHit = (TClonesArray*) fManager->GetObject("FTSHitMix");
	  if ( fFtsHit )
	    {
	      cout << "-I- PndPidCorrelator::Init: Using FTSHitMix" << endl;
	      fFtsMode = 2;
	    }
	  else
	    {
	      cout << "-W- PndPidCorrelator::Init: No FTS hits mix array! Switching FTS OFF" << endl;
	      fFtsMode = 0;
	    }
	}
    }

  // *** MVD ***
  if (fMvdMode)
    {
      if (fMixMode==kFALSE)
	{
	  fMvdHitsStrip = (TClonesArray*) fManager->GetObject("MVDHitsStrip");
	  if ( ! fMvdHitsStrip ) 
	    {
	      cout << "-W- PndPidCorrelator::Init: No MVDHitsStrip array!" << endl;
	    }
	  else fMvdMode = 2;
	  
	  fMvdHitsPixel = (TClonesArray*) fManager->GetObject("MVDHitsPixel");
	  if ( ! fMvdHitsPixel ) 
	    {
	      cout << "-W- PndPidCorrelator::Init: No MVDHitsPixel array!" << endl;
	    }
	  else fMvdMode = 2;
	}
      else
	{
	  fMvdHitsStrip = (TClonesArray*) fManager->GetObject("MVDHitsStripMix");
	  if ( ! fMvdHitsStrip )
	    {
	      cout << "-W- PndPidCorrelator::Init: No MVDHitsStripMix array!" << endl;
	    }
	  else fMvdMode = 2;
	  
	  fMvdHitsPixel = (TClonesArray*) fManager->GetObject("MVDHitsPixelMix");
	  if ( ! fMvdHitsPixel )
	    {
	      cout << "-W- PndPidCorrelator::Init: No MVDHitsPixelMix array!" << endl;
	    }
	  else fMvdMode = 2;
	}
      
      if (( ! fMvdHitsStrip ) &&  ( ! fMvdHitsPixel ))
	{
	  cout << "-W- PndPidCorrelator::Init: No MVD hits array! Switching MVD OFF" << endl;
	  fMvdMode = 0;
	}
      else
	{
	  cout << "-I- PndPidCorrelator::Init: Using MVDHit" << endl;
	}
    }
  
  // *** TOF ***
  if (fTofMode)
    {
      fTofHit = (TClonesArray*) fManager->GetObject("SciTHit");
      if ( ! fTofHit ) 
	{
	  cout << "-W- PndPidCorrelator::Init: No SciTHit array!" << endl;
	  fTofMode = 0;
	}
      else  
	{
	  cout << "-I- PndPidCorrelator::Init: Using SciTHit" << endl;
	  fTofMode = 2;
	}
    if (fIdeal)
      {
        fTofPoint = (TClonesArray*) fManager->GetObject("SciTPoint");
        if ( ! fTofPoint )
          {
            cout << "-W- PndPidCorrelator::Init: No SciTPoint array!" << endl;
            fTofMode = 0;
          }
      }
    }
  
  // *** FTOF ***
  if (fFtofMode)
    {
      fFtofHit = (TClonesArray*) fManager->GetObject("FtofHit");
      if ( ! fFtofHit ) 
	{
	  cout << "-W- PndPidCorrelator::Init: No FtofHit array!" << endl;
	  fFtofMode = 0;
	}
      else  
	{
	  cout << "-I- PndPidCorrelator::Init: Using FtofHit" << endl;
	  fFtofMode = 2;
	}
    if (fIdeal)
      {
        fFtofPoint = (TClonesArray*) fManager->GetObject("FtofPoint");
        if ( ! fFtofPoint )
          {
            cout << "-W- PndPidCorrelator::Init: No FtofPoint array!" << endl;
            fFtofMode = 0;
          }
      }
    }
  
  // *** EMC ***
  if (fEmcMode)
    {
      fEmcCluster = (TClonesArray*) fManager->GetObject("EmcCluster");
      if ( ! fEmcCluster ) 
	{
	  cout << "-W- PndPidCorrelator::Init: No EmcCluster array!" << endl;
	  fEmcMode = 0;
	}
      else 
	{
	  cout << "-I- PndPidCorrelator::Init: Using EmcCluster" << endl;
	  fEmcMode = 2;
	}
      
      fEmcBump = (TClonesArray*) fManager->GetObject("EmcBump");
      if ( ! fEmcBump ) 
	{
	  cout << "-W- PndPidCorrelator::Init: No EmcBump array!" << endl;
	}
      else 
	{
	  cout << "-I- PndPidCorrelator::Init: Using EmcBump" << endl;
	  fEmcMode = 3;
	}	  

      fEmcDigi = (TClonesArray*) fManager->GetObject("EmcDigi");
      if ( ! fEmcDigi)
        {
          cout << "-W- PndPidCorrelator::Init: No EmcDigi array! No EMC E1/E9/E25 information is propagated!" << endl;
        }
    }

  // *** DRC ***
  if (fDrcMode)
    {
      fDrcHit = (TClonesArray*) fManager->GetObject("DrcHit");
      if ( ! fDrcHit ) 
	{
	  cout << "-W- PndPidCorrelator::Init: No DrcHit array!" << endl;
	  fDrcMode = 0;
	}
      else  
	{
	  cout << "-I- PndPidCorrelator::Init: Using DrcHit" << endl;
	  fDrcMode = 2;
	}
    }
  
  // *** DSK ***
  if (fDskMode)
    {
      fDskParticle = (TClonesArray*) fManager->GetObject("DskParticle");
      if ( ! fDskParticle )
	{
	  cout << "-W- PndPidCorrelator::Init: No DskParticle array!" << endl;
	  fDskMode = 0;
	}
      else
	{
	  cout << "-I- PndPidCorrelator::Init: Using DskParticle" << endl;
	  fDskMode = 2;
	}
    }
  
  // *** MDT ***
  if (fMdtMode)
    {
      fMdtHit = (TClonesArray*) fManager->GetObject("MdtHit");
      if ( ! fMdtHit ) 
	{
	  cout << "-W- PndPidCorrelator::Init: No MdtHit array!" << endl;
	  fMdtMode = 0;
	}
      else  
	{
	  cout << "-I- PndPidCorrelator::Init: Using MdtHit" << endl;
	  fMdtMode = 2;
	}
      fMdtTrk = (TClonesArray*) fManager->GetObject("MdtTrk");
      if ( ! fMdtTrk ) 
	{
	  cout << "-W- PndPidCorrelator::Init: No MdtTrk array!" << endl;
	}
      else  
	{
	  cout << "-I- PndPidCorrelator::Init: Using MdtTrk" << endl;
	  fMdtMode = 3;
	}
    }
  
  if (fIdeal)
    {
      cout << "-I- PndPidCorrelator::Init: Using MonteCarlo correlation" << endl;
      fTofPoint = (TClonesArray*) fManager->GetObject("TofPoint");
      if ( ! fTofPoint ) 
	{
	  cout << "-W- PndPidCorrelator::Init: No TofPoint array!" << endl;
	  fTofMode = 0;
	}
      else  
	{
	  cout << "-I- PndPidCorrelator::Init: Using TofPoint" << endl;
	}
      fDrcPoint = (TClonesArray*) fManager->GetObject("DrcBarPoint");
      if ( ! fDrcPoint ) 
	{
	  cout << "-W- PndPidCorrelator::Init: No DrcBarPoint array!" << endl;
	  fDrcMode = 0;
	}
      else  
	{
	  cout << "-I- PndPidCorrelator::Init: Using DrcPoint" << endl;
	}
      fMdtPoint = (TClonesArray*) fManager->GetObject("MdtPoint");
      if ( ! fMdtPoint ) 
	{
	  cout << "-W- PndPidCorrelator::Init: No MdtPoint array!" << endl;
	  fMdtMode = 0;
	}
      else  
	{
	  cout << "-I- PndPidCorrelator::Init: Using MdtPoint" << endl;
	}
    }
#endif
  
  Register();

#if _TODAY_  
  fCorrPar->printParams();
#endif
  
  if (fGeanePro)
    { 
      cout << "-I- PndPidCorrelator::Init: Using Geane for Track propagation" << endl;
      if (!fCorrErrorProp)
	{
	  cout << "-I- PndPidCorrelator::Init: Switching OFF Geane error propagation" << endl;
	}
      if (fIdealHyp)
	{
	  fMcTrack = (TClonesArray *)fManager->GetObject("MCTrack");
	  if ( ! fMcTrack ) {
	    cout << "-I- PndPidCorrelator::Init: No PndMcTrack array! No ideal pid hypothesis is possible!" << endl;
	    return kERROR;
	  }
	  if (fTrackIDBranch=="") {
	    cout << "-I- PndPidCorrelator::Init: No TrackID Branch name! No ideal pid hypothesis is possible!" << endl;
	    return kERROR;
	  }
	}
      else
	{
	  switch (abs(fPidHyp))
	    {
	    case 0:
	      cout << "-I- PndPidCorrelator::Init: No PID set -> Using default PION hypothesis" << endl;
	      fPidHyp = 211;
	      break;
        
	    case 11:
	      cout << "-I- PndPidCorrelator::Init: Using ELECTRON hypothesis" << endl;
	      fPidHyp = -11;
	      break;
        
	    case 13:
	      cout << "-I- PndPidCorrelator::Init: Using MUON hypothesis" << endl;
	      fPidHyp = -13;
	      break;
        
	    case 211:
	      cout << "-I- PndPidCorrelator::Init: Using PION hypothesis" << endl;
	      fPidHyp = 211;
	      break;
        
	    case 321:
	      cout << "-I- PndPidCorrelator::Init: Using KAON hypothesis" << endl;
	      fPidHyp = 321;
	      break;
        
	    case 2212:
	      cout << "-I- PndPidCorrelator::Init: Using PROTON hypothesis" << endl;
	      fPidHyp = 2212;
	      break;
        
	    default:
	      cout << "-I- PndPidCorrelator::Init: Not recognised PID set -> Using default PION hypothesis" << endl;
	      fPidHyp = 211;
	      break;
	    }
	}
    }
  else
    {
      return kFATAL;
    }

#if _TODAY_  
  if (fMdtMode>0)
    {
      if (!MdtGeometry())
	{
	  cout << "-W- PndPidCorrelator::Init: No MDT geometry ???" << endl;
	  fMdtMode = 0;
	}
    }
  
  if   (fMdtRefit)
    {
      fFitter = new PndRecoKalmanFit();
      fFitter->SetGeane(fGeanePro);
      fFitter->SetNumIterations(1);
      if (!fFitter->Init()) return kFATAL;
    } 
  
  if (fDebugMode)
    {
      r = TFile::Open(sDir+sFile,"RECREATE");
    
      tofCorr = new TNtuple("tofCorr","TRACK-TOF Correlation",
			    "track_x:track_y:track_z:track_phi:track_p:track_charge:track_theta:track_z0:tof_x:tof_y:tof_z:tof_phi:chi2:dphi:len:glen");
      ftofCorr = new TNtuple("ftofCorr","TRACK-FTOF Correlation",
                            "track_x:track_y:track_z:ver_x:ver_y:ver_z:ver_px:ver_py:ver_pz:track_p:track_charge:track_theta:track_z0:tof_x:tof_y:tof_z:chi2:len:glen:tlen");
      emcCorr = new TNtuple("emcCorr","TRACK-EMC Correlation",
			    "track_x:track_y:track_z:track_phi:track_p:track_charge:track_theta:track_z0:emc_x:emc_y:emc_z:emc_phi:chi2:dphi:emc_ene:glen:emc_mod");
      fscCorr = new TNtuple("fscCorr","TRACK-FSC Correlation",
			    "track_x:track_y:track_z:track_phi:track_p:track_charge:track_theta:track_z0:emc_x:emc_y:emc_z:emc_phi:chi2:dphi:emc_ene:glen:emc_mod");
      mdtCorr = new TNtuple("mdtCorr","TRACK-MDT Correlation",
			    "track_x:track_y:track_z:track_dx:track_dy:track_dz:track_phi:track_p:track_charge:track_theta:track_z0:mdt_x:mdt_y:mdt_z:mdt_phi:mdt_p:chi2:mdt_mod:dphi:glen:mdt_count:nhits");
      drcCorr = new TNtuple("drcCorr","TRACK-DRC Correlation",
			    "track_x:track_y:track_z:track_phi:track_p:track_charge:track_theta:track_z0:drc_x:drc_y:drc_phi:chi2:drc_thetac:drc_nphot:dphi:glen:flag");
      dskCorr = new TNtuple("dskCorr","TRACK-DSK Correlation",
			    "track_x:track_y:track_z:track_phi:track_p:track_charge:track_theta:track_z0:dsk_x:dsk_y:dsk_z:dsk_phi:chi2:dsk_thetac:dsk_nphot:dphi:glen:track_lx:track_ly:track_lz:track_xp:flag");
      cout << "-I- PndPidCorrelator::Init: Filling Debug histograms" << endl;
    
    }
	
  // Set Parameters for Emc error matrix 
  if (fEmcMode>0) 
  {
    if (fEmcErrorMatrixPar->IsValid())
      {
        fEmcErrorMatrix->Init(fEmcErrorMatrixPar->GetParObject());
        //std::cout<<"PndPidCorrelator: Emc error matrix is read from RTDB"<<std::endl;
      } else
      {
        Int_t emcGeomVersion=fEmcGeoPar->GetGeometryVersion();
        fEmcErrorMatrix->InitFromFile(emcGeomVersion);
        fEmcErrorMatrixPar->SetErrorMatrixObject(fEmcErrorMatrix->GetParObject());
        //std::cout<<"PndPidCorrelator: Emc error matrix is read from file"<<std::endl;
      }
    fEmcCalibrator= PndEmcClusterCalibrator::MakeEmcClusterCalibrator(2, 1);	
  }

  if (fFast)  cout << "-W- PndPidCorrelator::Init: Using fast correlator!!" << endl;
#endif
    
  cout << "-I- PndPidCorrelator::Init: Success!" << endl;
  fEventCounter = 1;

  return kSUCCESS;
}

//______________________________________________________
void PndPidCorrelator::SetParContainers() {
#if _TODAY_  
  // Get run and runtime database
  FairRun* run = FairRun::Instance();
  if ( ! run ) Fatal("PndPidCorrelator:: SetParContainers", "No analysis run");
  
  FairRuntimeDb* db = run->GetRuntimeDb();
  if ( ! db ) Fatal("PndPidCorrelator:: SetParContainers", "No runtime database");
  
  // Get PID Correlation parameter container
  fCorrPar = (PndPidCorrPar*) db->getContainer("PndPidCorrPar");
  
  // Get Emc geometry parameter container
  fEmcGeoPar = (PndEmcGeoPar*) db->getContainer("PndEmcGeoPar");
  
  // Get Emc error matrix parameter container
  fEmcErrorMatrixPar = (PndEmcErrorMatrixPar*) db->getContainer("PndEmcErrorMatrixPar");

  // Get Stt parameter
  fSttParameters = (PndGeoSttPar*) db->getContainer("PndGeoSttPar");
  fFtsParameters = (PndGeoFtsPar*) db->getContainer("PndGeoFtsPar");
#endif
}

//______________________________________________________
void PndPidCorrelator::Exec(Option_t * option) {
  //-
  Reset();
  cout << " =====   PndPidCorrelator - Event: " << fEventCounter;
  Int_t nTracksTot=0;
  if (fTrack)
    {
      nTracksTot += fTrack->GetEntriesFast();
    }
#if _TODAY_
  if (fTrack2)
    {
      nTracksTot += fTrack2->GetEntriesFast();
    }
#endif

  cout << " - Number of tracks for pid " << nTracksTot;
#if _TODAY_
  if (fEmcMode>0)
    {
      PndEmcCluster *tmp_cluster;
      for(Int_t i=0; i < fEmcCluster->GetEntriesFast();i++)
	{
	  tmp_cluster = (PndEmcCluster*)fEmcCluster->At(i);
	  if(tmp_cluster->GetModule() < 5 && tmp_cluster->GetModule() >0)
	    {
	      fEmcClstCount++;
	    }
	  if(tmp_cluster->GetModule() == 5)
	    {
	      fFscClstCount++;
	    }
	}
      ResetEmcQ();
      cout << " - Number of Clusters for pid: ";
      cout << " EMC: " << fEmcClstCount;
      cout << " FSC: " << fFscClstCount;
    }
#endif
  cout<<endl;

  if (fTrack)     ConstructChargedCandidate();
#if _TODAY_
  if ((fEmcMode>0)  && (!fFast)) ConstructNeutralCandidate();
#endif
  fEventCounter++;
}

//______________________________________________________
void PndPidCorrelator::ConstructChargedCandidate() {
  //-
  //FIXME: Use Clear() to save time. 
  //Call Delete() only for too busy events to save Memory
  fPidChargedCand->Delete();
#if _TODAY_
  if (fMdtRefit) fMdtTrack->Delete();
  if (fMdtMode>0) MdtMapping();
#endif
  
  Int_t nTracks = fTrack->GetEntriesFast();
  for (Int_t i = 0; i < nTracks; i++) {
    PndTrack* track = (PndTrack*) fTrack->At(i);

    Int_t ierr = 0;
    FairTrackParP par = track->GetParamLast();
    cout << par.GetMomentum().Mag() << endl;
    if ((par.GetMomentum().Mag()<0.1) /*|| (par.GetMomentum().Mag()>100.)*/ )continue;
    FairTrackParH *helix = new FairTrackParH(&par, ierr);
    
    PndPidCandidate* pidCand = 	new PndPidCandidate();

    //for(unsigned iq=0; iq<track->mSmoothedValues.size(); iq++)
    //pidCand->mSmoothedValues.push_back(track->mSmoothedValues[iq]);

    if (fTrackIDBranch!="")
      {
	PndTrackID* trackID = (PndTrackID*) fTrackID->At(i);
	if (trackID->GetNCorrTrackId()>0)
	  {
	    pidCand->SetMcIndex(trackID->GetCorrTrackID());
	    if (fIdealHyp)
	      {
		PndMCTrack *mcTrack = (PndMCTrack*)fMcTrack->At(trackID->GetCorrTrackID());
		if ( ! mcTrack ) 
		  {
		    fPidHyp = 211;
                    cout << "-I- PndPidCorrelator::ConstructChargedCandidate: PndMCTrack does not exist!! (why?) -> let's try with pion hyp " << endl;
		  }
                else
		  {
		    fPidHyp = abs(mcTrack->GetPdgCode());
		  }
                if (fPidHyp>=100000000)
                  {
                    fPidHyp = 211;
                    std::cout << "-I- PndPidCorrelator::ConstructChargedCandidate: Track is an ion (PDGCode>100000000) -> let's try with pion hyp" << std::endl;
                  }

		if ( abs(fPidHyp)==13 ) fPidHyp = -13;
		if ( abs(fPidHyp)==11 ) fPidHyp = -11;
	      }
	  }
      } else { // added for PndAnalysis, TODO: remove after Fairlinks work with Associators
      PndTrackCand trackCand = track->GetTrackCand();
      pidCand->SetMcIndex(trackCand.getMcTrackId());
    }
    pidCand->SetTrackIndex(i);
    pidCand->SetTrackBranch(FairRootManager::Instance()->GetBranchId(fTrackBranch));
    pidCand->AddLink(FairLink(fTrackBranch, i));
    if (!GetTrackInfo(track, pidCand)) continue;
#if _TODAY_
    if ( (fMvdMode==2) && ((fMvdHitsStrip->GetEntriesFast()+fMvdHitsPixel->GetEntriesFast())>0) ) GetMvdInfo(track, pidCand); 
    if ( (fSttMode == 2) && (fSttHit    ->GetEntriesFast()>0) ) GetSttInfo(track, pidCand);
    GetGemInfo(track, pidCand);
    if (!fFast)
      {
	if ( (fTofMode==2) && (fTofHit    ->GetEntriesFast()>0) ) GetTofInfo(helix, pidCand);
	if ( (fEmcMode>0)  && (fEmcClstCount>0) ) GetEmcInfo(helix, pidCand);
	if ( (fMdtMode>0)  && (fMdtHit    ->GetEntriesFast()>0) ) GetMdtInfo(track, pidCand);  
	if ( (fDrcMode>0)  && (fDrcHit    ->GetEntriesFast()>0) ) GetDrcInfo(helix, pidCand);
	if ( (fDskMode>0)  && (fDskParticle->GetEntriesFast()>0)) GetDskInfo(helix, pidCand); 
      }
#endif
#if 0
    for(unsigned iq=0; iq<track->mSmoothedValues.size(); iq++) {
      TVector3 &pos = track->mSmoothedValues[iq].first;
      TVector3 &mom = track->mSmoothedValues[iq].second;
      
      printf("%10.4f %10.4f %10.4f -> %10.4f %10.4f %10.4f\n", 
	     pos.X(), pos.Y(), pos.Z(), mom.X(), mom.Y(), mom.Z());
    } //for iq
#endif
    PndPidCandidate *cand = AddChargedCandidate(pidCand);
    for(unsigned iq=0; iq<track->mSmoothedValues.size(); iq++) {
      cand->mSmoothedPositions.push_back(track->mSmoothedValues[iq].first);
      cand->mSmoothedMomenta.push_back  (track->mSmoothedValues[iq].second);
    } //for iq
    //cand->PrintMe();
  } 
  
#if _TODAY_
  if (fTrackBranch2!="")
    {
      Int_t nTracks2 = fTrack2->GetEntriesFast();
      for (Int_t i = 0; i < nTracks2; i++) {
	PndTrack* track = (PndTrack*) fTrack2->At(i);
	Int_t ierr = 0;
	FairTrackParP par = track->GetParamLast();
	if ((par.GetMomentum().Mag()<0.1) || (par.GetMomentum().Mag()>20.) )continue;
	FairTrackParH *helix = new FairTrackParH(&par, ierr);
      
	PndPidCandidate* pidCand =  new PndPidCandidate();
	if (fTrackIDBranch2!="")
	  {
	    PndTrackID* trackID = (PndTrackID*) fTrackID2->At(i);
	    if (trackID->GetNCorrTrackId()>0)
	      {
		pidCand->SetMcIndex(trackID->GetCorrTrackID());
		if (fIdealHyp)
		  {
		    PndMCTrack *mcTrack = (PndMCTrack*)fMcTrack->At(trackID->GetCorrTrackID());
		    if ( ! mcTrack ) 
		      {
			fPidHyp = 211;
			cout << "-I- PndPidCorrelator::ConstructChargedCandidate: PndMCTrack does not exist!! (why?) -> let's try with pion hyp " << endl;
		      }
		    else
		      {
			fPidHyp = abs(mcTrack->GetPdgCode());
		      }
		    if (fPidHyp>=100000000)
		      {
			fPidHyp = 211;
			std::cout << "-I- PndPidCorrelator::ConstructChargedCandidate: Track is an ion (PDGCode>100000000) -> let's try with pion hyp" << std::endl;
		      }
		    
		    if ( abs(fPidHyp)==13 ) fPidHyp = -13;
		    if ( abs(fPidHyp)==11 ) fPidHyp = -11;
		  }
	      }
	  } else { // added for PndAnalysis, TODO: remove after Fairlinks work with Associators
	  PndTrackCand trackCand = track->GetTrackCand();
	  pidCand->SetMcIndex(trackCand.getMcTrackId());
	}
	pidCand->SetTrackIndex(i);
        pidCand->SetTrackBranch(FairRootManager::Instance()->GetBranchId(fTrackBranch2));
	pidCand->AddLink(FairLink("PndTrack", i));
	if (!GetTrackInfo(track, pidCand)) continue;
	if ( (fMvdMode==2) && ((fMvdHitsStrip->GetEntriesFast()+fMvdHitsPixel->GetEntriesFast())>0) ) GetMvdInfo(track, pidCand); 
	if ( (fFtsMode == 2) && (fFtsHit    ->GetEntriesFast()>0) ) GetFtsInfo(track, pidCand); 
	GetGemInfo(track, pidCand);
	if (!fFast)
	  {
	    if ( (fFtofMode==2) && (fFtofHit->GetEntriesFast()>0) ) GetFtofInfo(helix, pidCand);
	    if ( (fEmcMode>0)  && (fFscClstCount>0) ) GetFscInfo(helix, pidCand);
	    //if ( (fMdtMode>0)  && (fMdtHit    ->GetEntriesFast()>0) ) GetMdtInfo(track, pidCand);
	    if (mapMdtForward.size()>0)  GetFMdtInfo(&par, pidCand);
	  } // end of fast mode
	AddChargedCandidate(pidCand);
      }
    }
#endif  
}

#if _TODAY_
//______________________________________________________
void PndPidCorrelator::ConstructNeutralCandidate() {
  //-
  fPidNeutralCand->Delete();
  TString emcType;
  
  Int_t nBumps = 0;
  if (fEmcMode == 2)
    {
      nBumps = fEmcCluster->GetEntriesFast();
      emcType = "EmcCluster";
    }
  else if(fEmcMode == 3)
    {
      nBumps = fEmcBump->GetEntriesFast();
      emcType = "EmcBump";
    }
  
  for (Int_t i = 0; i < nBumps; i++)
    {
      PndEmcBump* bump;
      PndEmcCluster *clu;
      Float_t quality = -1.;
      if (fEmcMode==2)
	{ 
	  if (fClusterList[i]) continue;
	  bump = (PndEmcBump*) fEmcCluster->At(i);
	  clu  = (PndEmcBump*) fEmcCluster->At(i);
          quality = fClusterQ[i];
	}
      else if(fEmcMode == 3)
	{
	  bump = (PndEmcBump*) fEmcBump->At(i);
	  if (fClusterList[bump->GetClusterIndex()]) continue; // skip correlated clusters
	  clu = (PndEmcCluster*)fEmcCluster->At(bump->GetClusterIndex());
          quality = fClusterQ[bump->GetClusterIndex()];
	}
    
      TVector3 vtx(0,0,0);
      TVector3 v1=bump->where();
      TVector3 p3;
      p3.SetMagThetaPhi(fEmcCalibrator->Energy(bump), v1.Theta(), v1.Phi());
      TLorentzVector lv(p3,p3.Mag());
      TMatrixD covP4=fEmcErrorMatrix->Get4MomentumErrorMatrix(*clu);
    
      PndPidCandidate* pidCand = new PndPidCandidate(0, vtx, lv);
      pidCand->SetP4Cov(covP4);
      pidCand->SetEmcRawEnergy(bump->energy());
      pidCand->SetEmcCalEnergy(fEmcCalibrator->Energy(bump));
      pidCand->SetEmcIndex(i);
      pidCand->SetEmcModule(bump->GetModule());
      pidCand->SetEmcNumberOfCrystals(bump->NumberOfDigis());
      pidCand->SetEmcNumberOfBumps(clu->NBumps());
      pidCand->SetEmcQuality(quality);

      pidCand->SetEmcClusterZ20(bump->Z20());
      pidCand->SetEmcClusterZ53(bump->Z53());
      pidCand->SetEmcClusterLat(bump->LatMom()); 
      if (fEmcDigi)
	{
	  PndEmcClusterEnergySums esum(*clu, fEmcDigi);
	  pidCand->SetEmcClusterE1(esum.E1());
	  pidCand->SetEmcClusterE9(esum.E9());
	  pidCand->SetEmcClusterE25(esum.E25());
	}
      pidCand->SetLink(FairLink(emcType, i));
    
      std::vector<Int_t> mclist = clu->GetMcList();
      if (mclist.size()>0)
	{
	  pidCand->SetMcIndex(mclist[0]);
	}
      AddNeutralCandidate(pidCand);
    }
}

//_________________________________________________________________
Bool_t PndPidCorrelator::GetTofInfo(FairTrackParH* helix, PndPidCandidate* pidCand) {
  
  if (!fIdeal)
    {
      if ((helix->GetMomentum().Theta()*TMath::RadToDeg())<20.) return kFALSE; 
      if ((helix->GetMomentum().Theta()*TMath::RadToDeg())>150.) return kFALSE;
    }
  FairGeanePro *fProTof = new FairGeanePro();
  if (!fCorrErrorProp) fProTof->PropagateOnlyParameters(); 
  FairGeanePro *fProVertex = new FairGeanePro();
  if (!fCorrErrorProp) fProVertex->PropagateOnlyParameters();
  //---
  PndSciTHit *tofHit = NULL; 
  Int_t tofEntries = fTofHit->GetEntriesFast();
  Int_t tofIndex = -1;
  Float_t tofTof = 0., tofLength = -1000, tofGLength = -1000, tofLengthTemp = -1000;
  Float_t tofQuality = 1000000;
  
  Float_t chi2 = 0;
  TVector3 vertex(0., 0., 0.);
  TVector3 tofPos(0., 0., 0.);
  TVector3 momentum(0., 0., 0.);
  for (Int_t tt = 0; tt<tofEntries; tt++)
    {
      tofHit = (PndSciTHit*)fTofHit->At(tt);
      if ( fIdeal && ( ((PndSciTPoint*)fTofPoint->At(tofHit->GetRefIndex()))->GetTrackID() !=pidCand->GetMcIndex()) ) continue;
      tofHit->Position(tofPos);
    
      if (fGeanePro) // Overwrites vertex if Geane is used
	{ 
     
	  fProTof->SetPoint(tofPos);
	  fProTof->PropagateToPCA(1, 1);
	  FairTrackParH *fRes= new FairTrackParH();
	  Bool_t rc =  fProTof->Propagate(helix, fRes, fPidHyp*pidCand->GetCharge());	
	  if (!rc) continue;
      
	  vertex.SetXYZ(fRes->GetX(), fRes->GetY(), fRes->GetZ());
     
	  fProVertex->SetPoint(TVector3(0,0,0));
	  fProVertex->PropagateToPCA(1, -1);
	  FairTrackParH *fRes2= new FairTrackParH();
	  Bool_t rc2 =  fProVertex->Propagate(fRes, fRes2, fPidHyp*pidCand->GetCharge());
	  if (rc2) tofLengthTemp = fProVertex->GetLengthAtPCA();
	}
    
      Float_t dist = (tofPos-vertex).Mag2();
    
      if ( tofQuality > dist)
	{
	  tofIndex = tt;
	  tofQuality = dist;
	  tofTof = tofHit->GetTime();
	  tofLength = tofLengthTemp; // abs(phi * track->GetRadius() / TMath::Sin(track->GetMomentum().Theta()));
	}
      if (fDebugMode)
	{
	  Float_t ntuple[] = {vertex.X(), vertex.Y(), vertex.Z(), vertex.Phi(),
			      helix->GetMomentum().Mag(), helix->GetQ(), helix->GetMomentum().Theta(), helix->GetZ(),
			      tofPos.X(), tofPos.Y(), tofPos.Z(), tofPos.Phi(),
			      dist, vertex.DeltaPhi(tofPos), tofLength, tofGLength};
	  tofCorr->Fill(ntuple);
	}
    }
  
  if ( (tofQuality<fCorrPar->GetTofCut()) || (fIdeal && tofIndex!=-1) )
    {
      pidCand->SetTofQuality(tofQuality);
      pidCand->SetTofStopTime(tofTof);
      pidCand->SetTofTrackLength(tofLength);
      pidCand->SetTofIndex(tofIndex);
      if (tofLength>0.)
	{
	  // mass^2 = p^2 * ( 1/beta^2 - 1 )
	  Float_t mass2 = helix->GetMomentum().Mag()*helix->GetMomentum().Mag()*(30.*30.*tofTof*tofTof/tofLength/tofLength-1.);
	  pidCand->SetTofM2(mass2);
	}
    }
  
  return kTRUE;
}

void PndPidCorrelator::ResetEmcQ()
{
  // Fuction to reset all the quality values for emc-track correlation to -1
  fClusterQ.clear();
  fClusterList.clear();
  for (Int_t ii=0; ii<fEmcCluster->GetEntriesFast(); ii++)
    {
      fClusterQ[ii] = -1;
      fClusterList[ii] = kFALSE;
    }
}

//_________________________________________________________________
Bool_t PndPidCorrelator::GetEmcInfo(FairTrackParH* helix, PndPidCandidate* pidCand) { 
  if(! helix){
    std::cerr << "<Error> PndPidCorrelator EMCINFO: FairTrackParH NULL pointer parameter."
	      <<std::endl;
    return kFALSE;
  }
  if(! pidCand){
    std::cerr << "<Error> PndPidCorrelator EMCINFO: pidCand NULL pointer parameter."
	      <<std::endl;
    return kFALSE;
  }
  FairGeanePro *fProEmc = new FairGeanePro(); 
  if (!fCorrErrorProp) fProEmc->PropagateOnlyParameters();
  //---
  Float_t trackTheta = helix->GetMomentum().Theta()*TMath::RadToDeg();
  
  Int_t emcEntries = fEmcCluster->GetEntriesFast();
  Int_t emcIndex = -1, emcModuleCorr = -1, emcNCrystals = -1, emcNBumps = -1;
  Float_t emcEloss = 0., emcElossCorr = 0., emcGLength = -1000;
  Float_t emcQuality = 1000000;
  Float_t chi2 = 0;
  TVector3 vertex(0., 0., 0.); TVector3 emcPos(0., 0., 0.);// TVector3 momentum(0., 0., 0.);
  
  // Cluster zenike moments
  Double_t Z20 = 0.0, Z53 = 0.0, secLatM = 0.00, E1 = 0., E9 = 0., E25 = 0.;
  
  for (Int_t ee = 0; ee<emcEntries; ee++)
    {
      PndEmcCluster *emcHit = (PndEmcCluster*)fEmcCluster->At(ee);
      
      if ( fIdeal )
	{
	  std::vector<Int_t> mclist = emcHit->GetMcList();
	  if (mclist.size()==0) continue;
	  if (mclist[0]!=pidCand->GetMcIndex()) continue;
	}
      
      if (emcHit->energy() < fCorrPar->GetEmc12Thr()) continue;
      Int_t emcModule = emcHit->GetModule();
      if (emcModule>4) continue;
      
      emcPos = emcHit->where();
      if (fGeanePro)
	{ // Overwrites vertex if Geane is used
	  fProEmc->SetPoint(emcPos);
	  fProEmc->PropagateToPCA(1, 1);
	  vertex.SetXYZ(-10000, -10000, -10000); // reset vertex
	  FairTrackParH *fRes= new FairTrackParH();
	  Bool_t rc =  fProEmc->Propagate(helix, fRes, fPidHyp*pidCand->GetCharge()); // First propagation at module
	  if (!rc) continue;
	  
	  emcGLength = fProEmc->GetLengthAtPCA();
	  vertex.SetXYZ(fRes->GetX(), fRes->GetY(), fRes->GetZ());
	  //std::map<PndEmcTwoCoordIndex*, PndEmcXtal*> tciXtalMap=PndEmcStructure::Instance()->GetTciXtalMap();
	  //PndEmcDigi *lDigi= (PndEmcDigi*)emcHit->Maxima();
	  //PndEmcXtal* xtal = tciXtalMap[lDigi->GetTCI()];
	  //emcPos = xtal->frontCentre();
	}
      
      Float_t dist = (emcPos-vertex).Mag2();
      if ( emcQuality > dist )
	{
	  emcIndex = ee;
	  emcQuality = dist;
	  emcEloss = emcHit->energy();
	  emcElossCorr = fEmcCalibrator->Energy(emcHit);
	  emcModuleCorr = emcModule;
	  emcNCrystals = emcHit->NumberOfDigis();
          emcNBumps  = emcHit->NBumps();
	  Z20 = emcHit->Z20();// Z_{n = 2}^{m = 0}
	  Z53 = emcHit->Z53();// Z_{n = 5}^{m = 3}
	  secLatM = emcHit->LatMom();
	  if (fEmcDigi)
	    {
	      PndEmcClusterEnergySums esum(*emcHit, fEmcDigi);
	      E1  = esum.E1(); 
	      E9  = esum.E9();
	      E25 = esum.E25();
	    }
	}
      
      if ( (fClusterQ[ee]<0) || (dist < fClusterQ[ee])) 
	// If the track-emc distance is less than the previous stored value (or still not initialized)
	{
	  fClusterQ[ee] = dist; // update the param
	}
      
      if (fDebugMode){
	Float_t ntuple[] = {vertex.X(), vertex.Y(), vertex.Z(), vertex.Phi(),
			    helix->GetMomentum().Mag(), helix->GetQ(), helix->GetMomentum().Theta(), helix->GetZ(),
			    emcPos.X(), emcPos.Y(), emcPos.Z(), emcPos.Phi(),
			    dist, vertex.DeltaPhi(emcPos), emcHit->energy(), emcGLength, emcModule};
	emcCorr->Fill(ntuple);
      }
    }// End for(ee = 0;)
  
  if ( (emcQuality < fCorrPar->GetEmc12Cut()) || ( fIdeal && emcIndex!=-1) ){
    fClusterList[emcIndex] = kTRUE;
    pidCand->SetEmcQuality(emcQuality);
    pidCand->SetEmcRawEnergy(emcEloss);
    pidCand->SetEmcCalEnergy(emcElossCorr);
    pidCand->SetEmcIndex(emcIndex);
    pidCand->SetEmcModule(emcModuleCorr);
    pidCand->SetEmcNumberOfCrystals(emcNCrystals);
    pidCand->SetEmcNumberOfBumps(emcNBumps);
    //======= 
    pidCand->SetEmcClusterZ20(Z20);
    pidCand->SetEmcClusterZ53(Z53);
    pidCand->SetEmcClusterLat(secLatM); 
    pidCand->SetEmcClusterE1(E1);
    pidCand->SetEmcClusterE9(E9);
    pidCand->SetEmcClusterE25(E25);
    //=====
  }
  
  return kTRUE;
}

//_________________________________________________________________
Bool_t PndPidCorrelator::GetMdtInfo(PndTrack* track, PndPidCandidate* pidCand) {
  //--- 
  FairTrackParP par = track->GetParamLast();
  Int_t ierr = 0;
  FairTrackParH *helix = new FairTrackParH(&par, ierr);
 
  map<Int_t, Int_t>mapMdtTrk;
  FairGeanePro *fProMdt = new FairGeanePro();
  if (!fCorrErrorProp) fProMdt->PropagateOnlyParameters();

  if (fMdtMode == 3)
    { 
      for (Int_t tt = 0; tt<fMdtTrk->GetEntriesFast(); tt++)
	{
	  PndMdtTrk *mdtTrk = (PndMdtTrk*)fMdtTrk->At(tt);
	  mapMdtTrk[mdtTrk->GetHitIndex(0)] = tt;
	}
    }
  PndMdtHit *mdtHit = NULL;
  Int_t mdtEntries = fMdtHit->GetEntriesFast();
  Int_t mdtIndex = -1, mdtMod = 0, mdtLayer = 0, mdtHits = 0;
  Float_t mdtGLength = -1000;
  Float_t mdtQuality = 1000000;
  Float_t mdtIron = 0., mdtMom = 0, mdtTempMom = 0;
  
  Float_t chi2 = 0;
  TVector3 vertex(0., 0., 0.);
  TVector3 vertexD(0., 0., 0.);
  TVector3 mdtPos(0., 0., 0.);
  TVector3 momentum(0., 0., 0.);
  for (Int_t mm = 0; mm<mdtEntries; mm++)
    {
      mdtHit = (PndMdtHit*)fMdtHit->At(mm);
      if ( fIdeal && ( ((PndMdtPoint*)fMdtPoint->At(mdtHit->GetRefIndex()))->GetTrackID() !=pidCand->GetMcIndex()) ) continue;
      if (mdtHit->GetLayerID()!=0) continue;
      if (mdtHit->GetModule()>2) continue;
      mdtHit->Position(mdtPos);
      if (fGeanePro) // Overwrites vertex if Geane is used
	{ 
     
	  fProMdt->SetPoint(mdtPos);
	  fProMdt->PropagateToPCA(1, 1);
	  vertex.SetXYZ(-10000, -10000, -10000); // reset vertex
	  vertexD.SetXYZ(-10000, -10000, -10000); // reset vertex
	  FairTrackParH *fRes= new FairTrackParH();
	  Bool_t rc =  fProMdt->Propagate(helix, fRes, fPidHyp*pidCand->GetCharge()); 
	  if (!rc) continue;
	  mdtTempMom = fRes->GetMomentum().Mag(); 
	  vertex.SetXYZ(fRes->GetX(), fRes->GetY(), fRes->GetZ());
	  vertexD.SetXYZ(fRes->GetDX(), fRes->GetDY(), fRes->GetDZ());
	  mdtGLength = fProMdt->GetLengthAtPCA();
	}
    
      Float_t dist;
      if (mdtHit->GetModule()==1) 
	{
	  dist = (mdtPos-vertex).Mag2();
	}
      else
	{
	  dist = (vertex.X()-mdtPos.X())*(vertex.X()-mdtPos.X())+(vertex.Y()-mdtPos.Y())*(vertex.Y()-mdtPos.Y());
	}
    
      if ( mdtQuality > dist)
	{
	  mdtIndex = mm;
	  mdtQuality = dist;
	  mdtMod = mdtHit->GetModule();
	  mdtMom = mdtTempMom;
	  mdtLayer = 1;
	  if (fMdtMode==3)
	    {
	      PndMdtTrk *mdtTrk = (PndMdtTrk*)fMdtTrk->At(mapMdtTrk[mdtIndex]);
	      mdtIndex = mapMdtTrk[mm];
	      mdtLayer = mdtTrk->GetLayerCount();
	      mdtIron = mdtTrk->GetIronDist();
	      mdtMod = mdtTrk->GetModule();
	      mdtHits = 0;
	      for (Int_t iLayer=0; iLayer<mdtLayer; iLayer++)
		{
		  mdtHits = mdtHits + mdtTrk->GetHitMult(iLayer);
		  //std::cout << iLayer << "\t" << mdtTrk->GetHitMult(iLayer) << "\t" << mdtHits << std::endl;
		}
	    }
	}
      if (fDebugMode)
	{
	  Float_t ntuple[] = {vertex.X(), vertex.Y(), vertex.Z(),
			      vertexD.X(), vertexD.Y(), vertexD.Z(), vertex.Phi(), 
			      helix->GetMomentum().Mag(), helix->GetQ(), helix->GetMomentum().Theta(), helix->GetZ(),
			      mdtPos.X(), mdtPos.Y(), mdtPos.Z(), mdtPos.Phi(), mdtTempMom,
			      dist, mdtHit->GetModule(), vertex.DeltaPhi(mdtPos), mdtGLength, mdtLayer, mdtHits};
	  mdtCorr->Fill(ntuple);
	}
    }
  
  if ((mdtQuality<fCorrPar->GetMdtCut()) || ( fIdeal && mdtIndex!=-1))
    {
      pidCand->SetMuoIndex(mdtIndex);
      pidCand->SetMuoQuality(mdtQuality);
      pidCand->SetMuoIron(mdtIron);
      pidCand->SetMuoMomentumIn(mdtMom);
      pidCand->SetMuoModule(mdtMod);
      pidCand->SetMuoNumberOfLayers(mdtLayer); 
      pidCand->SetMuoHits(mdtHits);
    }
  
  if (fMdtRefit && (mdtIndex!=-1) && (mdtMom>0.)  )
    {
      PndMdtTrk *mdtTrk = (PndMdtTrk*)fMdtTrk->At(mdtIndex); 
      PndTrack *mdtTrack = new PndTrack(*track);
      PndTrackCand *oldCand = track->GetTrackCandPtr();
      PndTrackCand *newCand = mdtTrk->AddTrackCand(oldCand);
      mdtTrack->SetTrackCand(*newCand);
      Int_t fCharge= mdtTrack->GetParamFirst().GetQ();
      Int_t PDGCode = fPidHyp*fCharge;
    
      PndTrack *fitTrack = new PndTrack();
      fitTrack = fFitter->Fit(mdtTrack, PDGCode);
      PndTrack* pndTrack = new PndTrack(fitTrack->GetParamFirst(), fitTrack->GetParamLast(), fitTrack->GetTrackCand(),
					fitTrack->GetFlag(), fitTrack->GetChi2(), fitTrack->GetNDF(), fitTrack->GetPidHypo(), fitTrack->GetRefIndex(), kLheTrack);
      AddMdtTrack(pndTrack);
    }
  return kTRUE;
}

//_________________________________________________________________
Bool_t PndPidCorrelator::GetDrcInfo(FairTrackParH* helix, PndPidCandidate* pidCand) {
  if (helix->GetZ()>120.) return kFALSE; // cut fwd endcap tracks
  FairGeanePro *fProDrc = new FairGeanePro();
  if (!fCorrErrorProp) fProDrc->PropagateOnlyParameters();
  //---
  PndDrcHit *drcHit = NULL;
  Int_t drcEntries = fDrcHit->GetEntriesFast();
  Int_t drcIndex = -1, drcPhot = 0;
  Float_t drcThetaC = -1000, drcThetaCErr = 0, drcGLength = -1000;
  Float_t drcQuality = 1000000;
  
  TVector3 vertex(0., 0., 0.);
  Float_t vertex_z = -1000;
  TVector3 drcPos(0., 0., 0.);
  TVector3 momentum(0., 0., 0.);

  if (fGeanePro) // Overwrites vertex if Geane is used
    {     
      fProDrc->PropagateToVolume("BarrelDIRC",0,1);
      vertex.SetXYZ(-10000, -10000, -10000); // reset vertex
      FairTrackParH *fRes= new FairTrackParH();
      Bool_t rc =  fProDrc->Propagate(helix, fRes, fPidHyp*pidCand->GetCharge()); 	
      if (!rc) return kFALSE;
      vertex.SetXYZ(fRes->GetX(), fRes->GetY(), 0.);
      vertex_z = fRes->GetZ();
      drcGLength = fProDrc->GetLengthAtPCA();
      if (drcGLength<25.) return kFALSE;  // additional cut on extrapolation distance to avoid fake correlations
    }
  
  for (Int_t dd = 0; dd<drcEntries; dd++)
    {
      drcHit = (PndDrcHit*)fDrcHit->At(dd); 
      if ( fIdeal && ( ((PndDrcBarPoint*)fDrcPoint->At(drcHit->GetRefIndex()))->GetTrackID() !=pidCand->GetMcIndex()) ) continue;
      drcHit->Position(drcPos);
      
      Float_t dphi = vertex.DeltaPhi(drcPos);
      Float_t dist = dphi * dphi;
      if (drcQuality > dist)
	{
	  drcIndex = dd;
	  drcQuality = dist;
	  drcThetaC = drcHit->GetThetaC();
	  drcThetaCErr = drcHit->GetErrThetaC();
	  drcPhot = 0; // ** to be filled **
	}
      if (fDebugMode)
	{
	  Float_t ntuple[] = {vertex.X(), vertex.Y(), vertex_z, vertex.Phi(),  
			      helix->GetMomentum().Mag(), helix->GetQ(), helix->GetMomentum().Theta(), helix->GetZ(),
			      drcPos.X(), drcPos.Y(), drcPos.Phi(), dist, drcHit->GetThetaC(), 0., vertex.DeltaPhi(drcPos), drcGLength,
			      pidCand->GetFitStatus()
	  };
	  drcCorr->Fill(ntuple);
	}
    }
  
  if ((drcQuality<fCorrPar->GetDrcCut()) || (fIdeal && drcIndex!=-1))
    {
      pidCand->SetDrcQuality(drcQuality);
      pidCand->SetDrcThetaC(drcThetaC);
      pidCand->SetDrcThetaCErr(drcThetaCErr);
      pidCand->SetDrcNumberOfPhotons(drcPhot);
      pidCand->SetDrcIndex(drcIndex);
    }
  return kTRUE;
}

//_________________________________________________________________
Bool_t PndPidCorrelator::GetDskInfo(FairTrackParH* helix, PndPidCandidate* pidCand) {
  if (helix->GetZ()<180.) return kFALSE; // consider tracks only from last gem plane
  
  FairGeanePro *fProDsk = new FairGeanePro(); 
  if (!fCorrErrorProp) fProDsk->PropagateOnlyParameters();
  //---
  PndDskParticle *dskParticle = NULL;
  Int_t dskEntries = fDskParticle->GetEntriesFast();
  Int_t dskIndex = -1, dskPhot = 0;
  Float_t dskThetaC = -1000, dskThetaCErr = 0, dskGLength = -1000;
  Float_t dskQuality = 1000000;
  Float_t x_p = -1000;
  
  TVector3 vertex(0., 0., 0.);
  TVector3 dskPos(0., 0., 0.);
  TVector3 momentum(0., 0., 0.);

  if (fGeanePro) // Overwrites vertex if Geane is used
    {     
      fProDsk->PropagateToVolume("Plate",0,1);
      vertex.SetXYZ(-10000, -10000, -10000); // reset vertex
      FairTrackParH *fRes= new FairTrackParH();
      Bool_t rc =  fProDsk->Propagate(helix, fRes, fPidHyp*pidCand->GetCharge());
      if (!rc) return kFALSE;
      vertex.SetXYZ(fRes->GetX(), fRes->GetY(), fRes->GetZ());
      dskGLength = fProDsk->GetLengthAtPCA();
      x_p = fRes->GetMomentum().Mag();
    }
  
  for (Int_t dd = 0; dd<dskEntries; dd++)
    {
      dskParticle = (PndDskParticle*)fDskParticle->At(dd);
      if ( fIdeal && (dskParticle->GetTrackID() !=pidCand->GetMcIndex()) ) continue;
      dskParticle->Position(dskPos);
      
      Float_t dist = (vertex-dskPos).Mag2();    
      if ( dskQuality > dist)
	{
	  dskIndex = dd;
	  dskQuality = dist;
	  dskThetaC = dskParticle->GetThetaC();
	  //dskThetaCErr = dskParticle->GetErrThetaC();
	  dskPhot = 0; // ** to be filled **
	}
      if (fDebugMode)
	{
	  Float_t ntuple[] = {vertex.X(), vertex.Y(), vertex.Z(), vertex.Phi(),
			      helix->GetMomentum().Mag(), helix->GetQ(), helix->GetMomentum().Theta(), helix->GetZ(),
			      dskPos.X(), dskPos.Y(), dskPos.Z(), dskPos.Phi(), dist, dskParticle->GetThetaC(), 0., vertex.DeltaPhi(dskPos), dskGLength,
			      helix->GetX(), helix->GetY(), helix->GetZ(), x_p, pidCand->GetFitStatus()
};
	  dskCorr->Fill(ntuple);
	}
    }
  
  if ((dskQuality<fCorrPar->GetDskCut()) || (fIdeal && dskIndex!=-1))
    {
      pidCand->SetDiscQuality(dskQuality);
      pidCand->SetDiscThetaC(dskThetaC);
      //pidCand->SetDskThetaCErr(dskThetaCErr);
      pidCand->SetDiscNumberOfPhotons(dskPhot);
      pidCand->SetDiscIndex(dskIndex);
    }
  return kTRUE;
}
#endif

//_________________________________________________________________
void PndPidCorrelator::Register() {
  //---
  TString chargName = "PidChargedCand" + fTrackOutBranch;
  FairRootManager::Instance()->Register(chargName,"Pid", fPidChargedCand, kTRUE); 

#if _TODAY_
  FairRootManager::Instance()->
    Register("PidNeutralCand","Pid", fPidNeutralCand, kTRUE);
  if (fMdtRefit)
    {
      FairRootManager::Instance()->
	Register("MdtTrack","Pid", fMdtTrack, kTRUE);
    }
#endif
}

//_________________________________________________________________
void PndPidCorrelator::Finish() {
#if _TODAY_
  //---
  if (fDebugMode)
    {
      //TFile *r = TFile::Open(sDir+sFile,"RECREATE");
      r->cd();
      tofCorr->Write();
      ftofCorr->Write();
      emcCorr->Write(); 
      fscCorr->Write();
      mdtCorr->Write();  
      drcCorr->Write();
      dskCorr->Write();

      r->Save();
    
      tofCorr->Delete();
      ftofCorr->Delete();
      emcCorr->Delete(); 
      fscCorr->Delete();
      mdtCorr->Delete();
      drcCorr->Delete();
      dskCorr->Delete();
    
      tofCorr = 0;
      ftofCorr = 0;
      emcCorr = 0; 
      fscCorr = 0;
      mdtCorr = 0;
      drcCorr = 0;
      dskCorr = 0;
    
      r->Close();
      r->Delete();
    }
#endif  
}
//_________________________________________________________________
void PndPidCorrelator::Reset() {
#if _TODAY_
  //---
  fMvdPath = 0.;
  fMvdELoss = 0.;
  fMvdHitCount = 0;
  fEmcClstCount = 0;
  fFscClstCount = 0;
  fClusterList.clear();
  fClusterQ.clear();
  mapMdtBarrel.clear();
  mapMdtEndcap.clear();
  mapMdtForward.clear();
#endif
}

//_________________________________________________________________
PndPidCandidate* PndPidCorrelator::AddChargedCandidate(PndPidCandidate* cand) {
  // Creates a new hit in the TClonesArray.
  TClonesArray& pidRef = *fPidChargedCand;
  Int_t size = pidRef.GetEntriesFast();
  return new(pidRef[size]) PndPidCandidate(*cand);
}

#if _TODAY_
//_________________________________________________________________
PndPidCandidate* PndPidCorrelator::AddNeutralCandidate(PndPidCandidate* cand) {
  // Creates a new hit in the TClonesArray.
  
  TClonesArray& pidRef = *fPidNeutralCand;
  Int_t size = pidRef.GetEntriesFast();
  return new(pidRef[size]) PndPidCandidate(*cand);
}

//_________________________________________________________________
PndTrack* PndPidCorrelator::AddMdtTrack(PndTrack* track) {
  // Creates a new hit in the TClonesArray.
  
  TClonesArray& pidRef = *fMdtTrack;
  Int_t size = pidRef.GetEntriesFast();
  return new(pidRef[size]) PndTrack(*track);
}
#endif

ClassImp(PndPidCorrelator)
