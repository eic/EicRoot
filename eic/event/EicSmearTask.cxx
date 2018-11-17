//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  Interface to eic-smear package;
//

#include <iostream>
#include <cassert>

#include "eicsmear/smear/Device.h"

#include <PndPidCandidate.h>
#include <PndMCTrack.h>

#include <EicSmearTask.h>

// ---------------------------------------------------------------------------------------

InitStatus EicSmearTask::Init()
{
  // Open eic-smear input file and get access to EICTree;
  inFile = new TFile(inFileName, "READ");
  if(not inFile->IsOpen()) {
    std::cerr << "Unable to open " << inFileName << std::endl;
    return kFATAL;
  } // if
  inFile->GetObject("EICTree", mcTree);
  if(not mcTree) {
    std::cerr << "Unable to find EICTree in " << inFileName << std::endl;
    return kFATAL;
  } // if

  // Unless provided via command line, create a dummy smearing detector;
  if (!detector)
  {
    detector = new Smear::Detector;   
    //Smear::Device momentum(Smear::kP, "0.01 * P"); 
    Smear::Device momentum(Smear::kP,     "0");   
    Smear::Device theta   (Smear::kTheta, "0");      
    Smear::Device phi     (Smear::kPhi,   "0");   
    detector->AddDevice(momentum);    
    detector->AddDevice(theta);
    detector->AddDevice(phi);
    detector->SetEventKinematicsCalculator("NM");
  } /*if*/

  // Create smeared event builder; follow the logic of SmearTree.cxx;
  TClass branchClass(mcTree->GetBranch("event")->GetClassName());
  if(branchClass.InheritsFrom("erhic::EventDis")) {
    builder = new Smear::EventDisFactory(*detector, *(mcTree->GetBranch("event")));
  } // if
  // Leave it here for now (but OFF); will need to modify Exec() call if ever 
  // want to activate;
#ifdef WITH_PYTHIA6_OFF
  else if(branchClass.InheritsFrom("erhic::hadronic::EventMC")) {
    builder = new Smear::HadronicEventBuilder(detector, *(mcTree->GetBranch("event")));
  } // else if
#endif
  else {
    std::cerr << branchClass.GetName() << " is not supported for smearing" <<
      std::endl;
    return kFATAL;
  } // else

  // Open the output file;
  TString outName = TString(inFileName).ReplaceAll(".root", ".smear.root");
  outFile = new TFile(outName, "RECREATE");
  if(not outFile->IsOpen()) {
    std::cerr << "Unable to create " << outName << std::endl;
    return kFATAL;
  } // if
  smearedTree = new TTree("Smeared", "A tree of smeared Monte Carlo events");

  eventbranch = builder->Branch(*smearedTree, "eventS");
  
  // Get access to arrays with PandaRoot MC tracks and charged track candidates;
  FairRootManager *fManager = FairRootManager::Instance();
  fMCTracks = (TClonesArray*)fManager->GetObject("MCTrack");
  if ( ! fMCTracks ) {
    std::cout << "-E-  EicSmearTask::Init: No MCTrack array found!" << std::endl;
    return kERROR;
  }
  fPidChargedCand = (TClonesArray *)fManager->GetObject("PidChargedCand");
  if ( ! fPidChargedCand) {
    std::cout << "-E- EicSmearTask::Init: No PidChargedCand array found!" << std::endl;
    return kERROR;
  }

  return kSUCCESS;
} // EicSmearTask::Init()

// ---------------------------------------------------------------------------------------

void EicSmearTask::SetParContainers()
{
} // EicSmearTask::SetParContainers()

// ---------------------------------------------------------------------------------------

void EicSmearTask::Exec(Option_t* opt)
{
  static unsigned evcounter;

  if (!mcTree->GetEntry(evcounter++)) return;
  
  //std::cout << fMCTracks->GetEntriesFast() << " MCTrack entries" << std::endl;
  //std::cout << fPidChargedCand->GetEntriesFast() << " PidChargedCand entries" << std::endl;

  {
    erhic::VirtualEvent *mcEvent = builder->GetEvBufferPtr();
    unsigned id[mcEvent->GetNTracks()];

    // Cook compressed index array of original (eic-smear) particles; indeed 
    // only those were given to the GEANT4 in simulation.C, which had status = 1;
    {
      unsigned rcounter = 0;

      for(unsigned tr=0; tr<mcEvent->GetNTracks(); tr++)
      { 
	erhic::VirtualParticle *vp = mcEvent->GetTrack(tr);

	// 'vp' pointer itself can not go NULL here; or should I check on that?;
	if (vp->GetStatus() == 1) id[rcounter++] = tr;
      } /*for tr*/
    }

    // Loop through the charged track candidates forund by PandaRoot and modify 
    // 4-momentum fields in original eic-smear arrays before running smearing 
    // builder code;
    for(unsigned ch=0; ch<fPidChargedCand->GetEntriesFast(); ch++)
    {
      PndPidCandidate* pidcand = (PndPidCandidate*)fPidChargedCand->At(ch);

      // Later may want to append eic-smear array with entries which are not 
      // associated with the original (MC) tracks (ghosts, etc);
      Int_t mcid = pidcand->GetMcIndex();
      if(mcid<0) continue; // no specified MC id... do nothing
      PndMCTrack *mctrack = (PndMCTrack*)fMCTracks->At(mcid);
      if( 0==mctrack) continue; // better do nothing on a null pointer
      Int_t mcpdg = mctrack->GetPdgCode();
    
      // Find respective particle in the original MC array (eic-smear input);
      erhic::VirtualParticle *vp = mcEvent->GetTrack(id[mcid]);

      // Modify 4-momentum; consider to use PDG code to obtain mass later?;
      // for now mass is taken as PDG hypothesis provided to Kalman filter, 
      // which is pion per default I guess;
      vp->Set4Vector(pidcand->GetLorentzVector());
      
      //printf("  %02d# -> %3d, PDG %5d (was %5d), rec.mass: %8.4f\n", 
      //     ch, mcid, mcpdg, int(vp->Id()), pidcand->GetLorentzVector().Mag()); 
    } /*for ch*/
  }

  // Run eic-smear code, eventually;
  builder->Fill(*eventbranch);
} // EicSmearTask::Exec()

// ---------------------------------------------------------------------------------------

void EicSmearTask::FinishTask()
{
  outFile->cd();
  smearedTree->Write();

  // Is this stuff really needed?;
  detector->Write("detector");
  outFile->Purge();

#if _OFF_
  std::cout << smearedTree->GetEntries() << " entries in the output tree" << std::endl;
  {
    //erhic::VirtualEvent *event(0);
    Smear::Event *event(0);
    smearedTree->SetBranchAddress("eventS", &event);

    for(unsigned ev=0; ev<smearedTree->GetEntries(); ev++)
    {
      smearedTree->GetEntry(ev);

      std::cout << event->GetNTracks() << " tracks in the event" << std::endl;
      // Loop through all the tracks and feed them to the FairRoot primary generator;
      for(unsigned iq=0; iq<event->GetNTracks(); iq++)
      { 
	//erhic::VirtualParticle *vp = event->GetTrack(iq);
	Smear::ParticleMCS *vp = event->GetTrack(iq);
    
	// May want to do it better later; suffices for now;
	if (!vp || vp->GetStatus() != 1) continue;

	std::cout << vp->id << " " << vp->GetPx() << " " << vp->GetPy() << " " << vp->GetPz() << std::endl;
      } /*for iq*/
    } /*for ev*/
  }

  std::cout << "About to finish!" << std::endl;
#endif

  FairTask::FinishTask();
} // EicSmearTask::FinishTask()

// ---------------------------------------------------------------------------------------

ClassImp(EicSmearTask)
