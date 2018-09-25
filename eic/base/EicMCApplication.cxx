//
// AYK (ayk@bnl.gov), 2015/08/04
//
//  A derived class with the only purpose to expand functionality 
// of FairMCApplication::Stepping() call; basically want to bypass 
// FairMCApplication sensitive volume branch (say avoid declaring 
// killer volumes sensitive with th eonly purpose to enter 
// EicDetector::ProcessHits();
//

#include <TVirtualMC.h>
#include <TParticle.h>

#include <FairRunSim.h>
#include <FairModule.h>

#include <PndStack.h>

#include <EicDetector.h>
#include <EicRunSim.h>
#include <EicMCApplication.h>

// ---------------------------------------------------------------------------------------

//
//  NB: functionality has not been checked thoroughly after moving the codes from
// EicDetector::ProcessHits() to this call; do LATER; 
//

void EicMCApplication::Stepping()
{
  bool stopped = false;
  int copyNo, volumeID = gMC->CurrentVolID(copyNo);
  fModVolIter =fModVolMap.find(volumeID);
  //printf("%3d -> %s\n", fModVolIter->second, GetModule(fModVolIter->second)->GetName());

  FairModule *module = (FairModule *)FairRunSim::Instance()->GetListOfModules()->At(fModVolIter->second);
  //printf("-> %s (%s)\n", module->GetName(), gMC->CurrentVolName());
   
  Int_t trackID  = gMC->GetStack()->GetCurrentTrackNumber();
  TParticle *particle = ((PndStack*)gMC->GetStack())->GetParticle(trackID);

  // Theoretically EicRunSim singleton may be NULL (simulation started via FairRunSim 
  // rather than EicRunSim); practically EicMCApplication is only instantiated from 
  // EicRunSim::Init(); yet check that pointer is not NULL in all places;
  EicRunSim *fRun = EicRunSim::Instance();

  // Well, yes, it looks like this call should be here, next to AddMoCaPoint();
  // FIXME: check what's wrong with small steps;
  //if (fStep > 0.001) {
  if (/*gMC->TrackStep() > 0.001 &&*/ fRun) {
    FluxMonitorGrid *flgrid = fRun->GetFluxMonitorGrid();
	
    // Let FluxMonitorGrid class do the actual job;
    if (flgrid) {
      TLorentzVector fPosOut, fMomOut;
      gMC->TrackPosition(fPosOut);
      gMC->TrackMomentum(fMomOut);
      //double step = gMC->TrackStep();
      
#if _OFF_
      {
	static int last = -1;

	if (particle->GetPdgCode() == 2212) {
	  //if (trackID != last) printf("\n");
	  last = trackID;

	  //printf("track#%4d (%d %d %d %d %d -> E: %9.4f) -> XYZ: %8.2f %8.2f %8.2f -> step %8.5f\n", trackID, 
	  //	 1000*(gMC->Etot() - particle->GetMass()),
	  //	 gMC->IsNewTrack(), gMC->IsTrackEntering(), gMC->IsTrackExiting(), gMC->IsTrackStop(), gMC->IsTrackDisappeared(),
	  //	 fPosOut.Vect()[0], fPosOut.Vect()[1], fPosOut.Vect()[2], gMC->TrackStep()); 
	} //if    
      }
#endif

      //if (1E9*gMC->TrackTime() > 1E7 && particle->GetPdgCode() == 2112 && 
      //  1E9*(gMC->Etot() - particle->GetMass()) > 1E5)
      //printf("%3d (%4d)-> %7.1f ms, eKin: %7.1f eV\n", particle->GetPdgCode(), trackID, 
      //       1E3*gMC->TrackTime(), 1E9*(gMC->Etot() - particle->GetMass()));
      flgrid->AddEntry(particle->GetPdgCode(), gMC->Etot() - particle->GetMass(), gMC->Edep(), 
		       //fPosIn.Vect(), fPosOut.Vect());
		       // FIXME: this assumes ~straight track (or small step); 
		       fPosOut.Vect() - gMC->TrackStep() * fMomOut.Vect().Unit(), 
		       fPosOut.Vect());
    } //if
  } //if

  // If this is a valid EicDetector, few more actions;
  EicDetector *detector = dynamic_cast<EicDetector*> (module);
  if (detector) {
    //printf("  -> %s\n", detector->GetName());

    // Check 1) secondary kill flag, 2) whether this volume kills such particles (for now 
    // do not distinguish PDG codes; extend functionality later if necessary);
    if ((!particle->IsPrimary() && fRun && fRun->SuppressSecondariesFlag()) ||
	detector->IsKillerVolume(gMC->CurrentVolName())) {
      gMC->StopTrack();
      stopped = true;
    } //if
    
    // Check energy monitor volumes;
    detector->CheckEnergyMonitors(gMC->CurrentVolName(), trackID, particle->GetPdgCode(), 
				  particle->IsPrimary(), 
				  gMC->IsTrackEntering(), gMC->IsTrackExiting(), gMC->Etot());

    // Record track ID in the global list;
    if (fRun && !fRun->IgnoreBlackHoleVolumesFlag() && 
	detector->gptr && detector->gptr->IsBlackHoleVolume(gMC->CurrentVolName())) 
      // Yes, primary particle(s) are parents anyway?; THINK on this later;
      /*if (!particle->IsPrimary())*/ EicBlackHole::InsertIntoTrackList(trackID);
  } //if

  // THINK: so, do not call FairMCApplication stepper at all; it looks I can ignore FLUKA 
  // PreTrack() workaround as well (see FairMCApplication::Stepping() call);
  if (!stopped && (!fRun || !fRun->SuppressFairRootSteppingCallFlag()))
    FairMCApplication::Stepping();
} // EicMCApplication::Stepping()

// ---------------------------------------------------------------------------------------

ClassImp(EicMCApplication)
