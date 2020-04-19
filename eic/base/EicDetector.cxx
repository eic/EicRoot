//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  Basic EIC detector class; several small size methods are 
//  here just to interface with basic FairRoot & PandaRoot classes;
//

#include <assert.h>
// FIXME: is it really Mac-OS-specific?;
#ifndef __APPLE__
#include <asm/types.h>
#endif
#include <iostream>
using namespace std;

#include "TGeoManager.h"
#include "TClonesArray.h"
#include "TVirtualMC.h"
#include "TLorentzVector.h"
#include "TList.h"
#include <TParticle.h>
#include "TObjArray.h"
#include <TSystem.h>
#include <TGeoArb8.h>

#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "FairGeoLoader.h"
#include "FairGeoInterface.h"
#include "FairGeoNode.h"
#include "FairGeoVolume.h"
#include "FairVolume.h"
#include "FairRootManager.h"

#include "PndStack.h"
#include <PndGeoHandling.h>

#include "EicGeo.h"
#include "EicGeoPar.h"
#include "EicDetector.h"
#include <EicRunSim.h>

// ---------------------------------------------------------------------------------------

EicDetector::EicDetector(const char *Name, const char *geometryName, EicDetectorId dType, 
	      SteppingType stType, Bool_t Active): FairDetector(Name, Active)
{
  ResetVars();

  // This would basically mean EicFieldMapDetector->DummyDetector chain was invoked
  // with sort of default constructor;
  if (!Name) return;

  fVerboseLevel = 1;

  // Do this check better later; will it work this way at all?;
  assert(DetectorId(dType) <= kHYP);
  fDetType = dType;

  fStType = stType;

  dname = new EicDetName(Name);
  
  fEicMoCaPointCollection     = new TClonesArray("EicMoCaPoint"); 
  // Yes, prefer to create this information flow anyway, no matter it is needed or not;
  //fEicFakeMoCaPointCollection = new TClonesArray("EicFakeMoCaPoint"); 
    
  eicContFact = new EicContFact(this, (dname->Name() + "ContFact").Data(), 
				("Factory for parameter containers in lib" + dname->Name()).Data(),
				(dname->Name() + "GeoPar").Data(), 
				(dname->Name() + " Geometry Parameters").Data(),
				(dname->Name() + "DefaultContext").Data());
  
  SetGeometryFileName(geometryName);
} // EicDetector::EicDetector()

// ---------------------------------------------------------------------------------------
#if _LATER_
int EicDetector::SetEnergyCutOff(Int_t PDG, const Double_t cutMin, const Double_t cutMax)
{
  if (cutMin <= 0.0 || cutMin >= cutMax) return -1;

  if (!_fCutOffMap) _fCutOffMap = new std::map<Int_t, std::pair<Double_t,Double_t> >();

  // No PDG code checks?; Ok; eventually should arrange something like a wildcard
  // (say, all hadrons); also, prefer to store positive value, assuming that eg
  // cutoff for electrons and positrons should be the same; change later if this 
  // proves to be misleading;
  (*_fCutOffMap)[abs(PDG)] = std::pair<Double_t, Double_t>(cutMin, cutMax);

  return 0;
} // EicDetector::SetEnergyCutOff()
#endif

// ---------------------------------------------------------------------------------------

void EicDetector::SetGeometryFileName(TString fname, TString geoVer)
{
  // Allow two exceptions;
  if (fname.BeginsWith("/") || fname.BeginsWith("./") || fname.BeginsWith(".."))
    fgeoName = fname;
  else
    // Otherwise append $VMCWORKDIR or such in a standard FairRoot call;
    FairModule::SetGeometryFileName(fname, geoVer);
} // EicDetector::SetGeometryFileName()

// ---------------------------------------------------------------------------------------

FairParSet* EicDetector::EicGeoParAllocator(FairContainer *c)
{ 
  return new EicGeoPar(c->getConcatName().Data(),c->GetTitle(),c->getContext());
} // EicDetector::EicGeoParAllocator()

// ---------------------------------------------------------------------------------------

//
// FIXME: well, there should be a plenty of stuff here, or?;
//

EicDetector::~EicDetector()
{
} // EicDetector::~EicDetector()

// ---------------------------------------------------------------------------------------

int EicDetector::DeclareGeantSensitiveVolume(const char *name, SteppingType stType)
{
  if (!fListOfGeantSensitives) fListOfGeantSensitives = new EicNamePatternHub<SteppingType>();

  // I guess no double-counting check is really needed here?; if non-trivial stepping 
  // type is given, use it; otherwise pick up the stepping type given at detector initialization;
  fListOfGeantSensitives->AddExactMatch(name, stType == qSteppingTypeDefault ? fStType : stType);

  return 0;
} // EicDetector::DeclareGeantSensitiveVolume()

// ---------------------------------------------------------------------------------------

int EicDetector::DeclareGeantSensitiveVolumePrefix(const char *name, SteppingType stType)
{
  if (!fListOfGeantSensitives) fListOfGeantSensitives = new EicNamePatternHub<SteppingType>();

  // Same logic as in DeclareGeantSensitiveVolume();
  fListOfGeantSensitives->AddPrefixMatch(name, stType == qSteppingTypeDefault ? fStType : stType);

  return 0;
} // EicDetector::DeclareGeantSensitiveVolumePrefix()

// ---------------------------------------------------------------------------------------

bool EicDetector::CheckIfSensitive(std::string name)
{
  if (mAllVolumesSensitiveFlag) return true;

  //return (fListOfGeantSensitives ? fListOfGeantSensitives->AnyMatch(name.c_str()) : false);
  return (fListOfGeantSensitives && fListOfGeantSensitives->AnyMatch(name.c_str()));
} // EicDetector::CheckIfSensitive()

// ---------------------------------------------------------------------------------------

bool EicDetector::IsKillerVolume(const char *name)
{
  //return (mKillerVolumes ? mKillerVolumes->AnyMatch(name) : false);
  return (mKillerVolumes && mKillerVolumes->AnyMatch(name));
} // EicDetector::IsKillerVolume()

// ---------------------------------------------------------------------------------------

void EicDetector::EndOfEvent() 
{
   if(fVerboseLevel) Print();

   // FIXME: figure out how to use point->GetPrimaryMotherID() here;
#if _LATER_
   // If database creation is requested, fill out;
   if (dbFile)
   {
     // Assume ID=0 (see comment below); can this call fail?; fix later;
     TParticle *primary = ((PndStack*)gMC->GetStack())->GetParticle(0);

     dbEntry->setPrimaryVtx             (TVector3(primary->Vx(), primary->Vy(), primary->Vz()));
     dbEntry->setPrimaryParticleMomentum(TVector3(primary->Px(), primary->Py(), primary->Pz()));

     Int_t PDG = primary->GetPdgCode();
     // Well, prefer yet another sanity check;
     if (_fCutOffMap->find(abs(PDG)) == _fCutOffMap->end())
     {
       printf("\n *** Primary mother PDG is not present in original MC file map; "
	      "(must be the wrong input MC file)! ***\n\n");
       exit(0);
     } //if
     dbEntry->setPrimaryParticlePDG(PDG);

     TClonesArray *arr = fThreadVar->GetArrayPtr();

     //unsigned int hNum = _fEicMoCaPointCollection[th]->GetEntriesFast();
     unsigned int hNum = arr->GetEntriesFast();
     // Loop through all the hits and put them into array;
     printf("--> %4d hits ...\n", hNum);

     for(unsigned iq=0; iq<hNum; iq++)
     {
       //EicMoCaPoint *point = (EicMoCaPoint*)_fEicMoCaPointCollection[th]->At(iq); 
       EicMoCaPoint *point = (EicMoCaPoint*)arr->At(iq);     

       // Well, if input MC file was properly cooked and no mess happened 
       // to generator setup (so - and better a single - EicFakeMoCaPointGenerator() 
       // call was used), primary mother should have been a single - and most likely 
       // one of the {e+,e-,gamma} PDG - track;
       if (point->_GetPrimaryMotherID())
       {
	 // Check whether I need a primary or a secondary mother ID here;
	 assert(0);
	 printf("\n *** Primary mother ID is != 0; "
		"(must be the wrong input MC file and/or generator)! ***\n\n");
	 exit(0);
       } //if

       EicFakeMoCaPointDbHit hit(point->GetPosIn(), point->GetEnergyLoss());

       new((*dbEntry->fHits)[dbEntry->fHits->GetEntriesFast()]) EicFakeMoCaPointDbHit(hit);
     } //for iq

     dbTree->Fill();
     dbEntry->fHits->Delete();
   } //if
#endif

   // Eventually delete MC point clone arrays;
   fEicMoCaPointCollection->Delete();
   // fEicFakeMoCaPointCollection->Delete();

   // Clear registered track lists in energy monitors (to avoid double-counting);
  for(unsigned mn=0; mn<mEnergyMonitorVolumes.size(); mn++) {
    EicEnergyMonitor *monitor = mEnergyMonitorVolumes[mn];

    monitor->mRegisteredTracks.clear();
  } //for mn
} // EicDetector::EndOfEvent()

// ---------------------------------------------------------------------------------------

void EicDetector::Register() 
{
  // This call creates a branch with a given name in the output tree; setting the last 
  // parameter to kFALSE means: this collection will not be written to the file (AYK ?), 
  // it will exist only during the simulation;
  FairRootManager::Instance()->Register(dname->Name() + "MoCaPoint", dname->Name(), 
					fEicMoCaPointCollection, kTRUE);

  //FairRootManager::Instance()->Register(dname->Name() + "FakeMoCaPoint", dname->Name(), 
  //					fEicFakeMoCaPointCollection, kTRUE);
} // EicDetector::Register()

// ---------------------------------------------------------------------------------------

#define _64BIT_VALUE_INVALID_ (~ULong64_t(0))

ULong64_t EicDetector::GetNodeMultiIndex()
{
  ULong64_t ret = _64BIT_VALUE_INVALID_;

  // In particular HADES-style geometries have no chance to share their mapping; so 
  // just return back nonsense index; NB: also new-style geometry files may be missing 
  // this info;
  if (!gptr) return ret;

  //
  //  Later may want to create a look-up table (STL map) based on node ID;
  //  really needed?;
  //

  UInt_t lvVolumeIds[gptr->GetMaxVolumeLevelNum()], lvNodeIds[gptr->GetMaxVolumeLevelNum()];

  // If current path is not the same as it was upon sensitive volume entry, 
  // have to fool the geometry manager; check later, what's wrong with GEANT4 here;
  TString returnBackPath;
  if (strcmp(gGeoManager->GetPath(), fPathUponEntry.Data()))
  {
    returnBackPath = gGeoManager->GetPath();
    gGeoManager->cd(fPathUponEntry);
  } //if

  for(int lv=0; lv<gptr->GetMaxVolumeLevelNum(); lv++)
  {
    // May also use gGeoManager->GetMother() for lv=0; who cares;
    TGeoNode *node = lv ?  gGeoManager->GetMother(lv) : gGeoManager->GetCurrentNode();

    // Could this crash if node=0 (for instance levels exhausted in this branch)?;
    // well, check on that; setting values to 0 do not hurt (may also leave them 
    // uninitialized, but Ok);
    lvVolumeIds        [lv] = node ? node->GetVolume()->GetNumber()  : 0;
    lvNodeIds          [lv] = node ? node->             GetNumber()  : 0; 
  } // for lv

  // Loop through all maps and find matching one;
  for(int iq=0; iq<gptr->GetMapNum(); iq++)
  {
    EicGeoMap *fmap = gptr->GetMapPtrViaMapID(iq);
  
    if (fmap->IsMySignature(lvVolumeIds)) 
    {
      ret = (ULong64_t(iq) & _SERVICE_BIT_MASK_) << _GEANT_INDEX_BIT_NUM_; 

      // Tune shifts to this particular map;
      for(int lv=0; lv<fmap->GetGeantVolumeLevelNum(); lv++)
      {
	const GeantVolumeLevel *lptr          = fmap->GetGeantVolumeLevelPtr(lv);
	const EicBitMask<UGeantIndex_t> *mask = lptr->GetBitMaskPtr();

	// Skip "dummy" levels (for instance individual fibers);
	if (!lptr->GetMaxEntryNum()) continue;

	// Check range;
	if (lvNodeIds[lv] >= lptr->GetMaxEntryNum()) 
	{
	  // Just a warning; want to see this situation first;
	  cout<<"-E- Eic"<< dname->Name() <<": " << 
	    "Sensitive volume multi-index too large!" << endl;

	  // Restore GEANT4 geo manager state if had to switch it;
	  if (!returnBackPath.IsNull()) gGeoManager->cd(returnBackPath);
	  return _64BIT_VALUE_INVALID_;
	} //if

	// Well, in fact there is no need to mask out bits here (see check above);
	ret |= (lvNodeIds[lv] & mask->GetBitMask()) << mask->GetShift();
      } //for lv

      // Well, consider to fill out base name if not done so yet; obviously, 
      // if no hits were registered at this map, string will not be filled 
      // out at all, which is Ok;
      if (fmap->GetBaseVolumePath()->IsNull()) 
      {
	// Record current path; 
	TString path = gGeoManager->GetPath();
	
	// Go few levels up;
	for(int lv=0; lv<fmap->GetGeantVolumeLevelNum(); lv++)
	  gGeoManager->CdUp();

	fmap->AssignBaseVolumePath(gGeoManager->GetPath());

	// Return back in the node tree;
	gGeoManager->cd(path);
      } //if

      break;
    } /*if*/
  } //for iq

  // Return back to the insensitive neighboring volume where GEANT4 probably 
  // ended up exiting from the "true" sensitive volume;
  if (!returnBackPath.IsNull()) gGeoManager->cd(returnBackPath);

  // Do it better later; at least don't care about this warning for all-sensitive
  // (basically dummy) detectors in a special-purpose mode (say neutron flux calculation);
  if (ret == _64BIT_VALUE_INVALID_ && !mAllVolumesSensitiveFlag) 
    printf("%s vs %s\n", gGeoManager->GetPath(), fPathUponEntry.Data());

  return ret;
} // EicDetector::GetNodeMultiIndex()

// ---------------------------------------------------------------------------------------

void EicDetector::CheckEnergyMonitors(const char *name, Int_t trackID, 
				      Int_t PDG, bool isPrimary,
				      bool isEntering, bool isExiting, 
				      double energy)
{
  // Should either be entering or exiting -> check on that;
  if (!isEntering && !isExiting) return;

  // Carelessly loop through all declared monitor volumes; no optimization, 
  // since this is a debugging stuff anyway;
  for(unsigned mn=0; mn<mEnergyMonitorVolumes.size(); mn++) {
    EicEnergyMonitor *monitor = mEnergyMonitorVolumes[mn];

    // Check volume name match;
    if (!monitor->mName.EqualTo(name)) continue;

    // Check entrance/exit condition match;
    if (isEntering && !monitor->mAtEntrance) continue;
    if (isExiting  &&  monitor->mAtEntrance) continue;

    // Check PDG and primary/any particle type match;
    if (PDG != monitor->mPDG) continue;
    if (!isPrimary && monitor->mPrimaryOnly) continue;

    // Double-entry protection;
    if (monitor->mRegisteredTracks.find(trackID) != monitor->mRegisteredTracks.end()) continue;
    monitor->mRegisteredTracks.insert(trackID);

#if 0
    {
      static unsigned counter = 0;

      printf("%4d (%d .. %d) %d -> %f\n", counter++, isPrimary, monitor->mPrimaryOnly, isEntering, energy);
    }
#endif    

    // Full match, fine; fill out hiftogram entry; FIXME: there is no control over 
    // 2-d entry of a given particle into the same volume, so use with care;
    {
      TAxis *xaxis = monitor->mHistogram->GetXaxis();

      // Do not bother to fill out-of-range values;
      if (energy >= xaxis->GetXmin() && energy <= xaxis->GetXmax())
	monitor->mHistogram->Fill(energy);
    }
  } //for mn
} // EicDetector::CheckEnergyMonitors()

// ---------------------------------------------------------------------------------------

void EicDetector::ResetSteppingVariables()
{
  // Set parameters at entrance of volume. Reset ELoss.
  fELoss  = fStep = 0.0;
  fTime   = gMC->TrackTime() * 1.0e09;
  fLength = gMC->TrackLength();
  gMC->TrackPosition(fPosIn);
  gMC->TrackMomentum(fMomIn);
  
  fPathUponEntry = gGeoManager->GetPath();
} // EicDetector::ResetSteppingVariables()

// ---------------------------------------------------------------------------------------

Bool_t EicDetector::ProcessHits(FairVolume *v)
{
  //printf("Here! -> %s %d %d\n", v->GetName(), gMC->IsTrackEntering(), gMC->IsTrackExiting());

  // Well, even if SuppressHitProduction flag is set I still may want to record
  // track trajectories in FairMCApplication::Stepping(); however return here
  // immediately since no hits are wanted;
  EicRunSim *fRun = EicRunSim::Instance();
  if (fRun && fRun->SuppressHitProductionFlag()) return kTRUE;

  Int_t trackID  = gMC->GetStack()->GetCurrentTrackNumber();
  Int_t volumeID = v->getMCid();
  TParticle *particle = ((PndStack*)gMC->GetStack())->GetParticle(trackID);
  //printf("%d\n", particle->GetPdgCode());

  //   For now consider the following logic of which particles will make it 
  // into the AddMoCaPoint() call:
  //   - any changed particle with fELoss>0; 
  //   - neutral particles with PDG codes strictly bound to specific volumes;
  //     consider to let detector geometry know, which ones are those (so the 
  //     respective call should happen in geometry creation script like rich.C 
  //     in order to enable optical photons to produce hits; in fact this 
  //     scheme also allows to have charged particle hits with fELoss=0; fine;
  //
  //   No threshold is applied on either fELoss or fStep; digitizer may want to 
  // reject some of the hits later (especially since merged energy deposit from 
  // two tracks can go above threshold; makes sense?;
  bool wanted = gptr ? gptr->IsWantedParticle(v->GetName(), particle->GetPdgCode()) : false;

  // May reconsider this return call once frozen shower code (see below) is 
  // back to life; in this case may want photons to be included in the database;
  if (!wanted && !gMC->TrackCharge()) return kTRUE;
  
  // In Geant4 mode upon volume entry check whether max step should be set explicitely;
  if ((gMC->IsTrackEntering() || gMC->IsNewTrack()) && gptr) {
    double step = gptr->GetEnforcedStep(v->getMCid());
    
    // step=0.0 simply means "don't bother for this volume";
    if (step) gMC->SetMaxStep(step);
    //gMC->SetMaxStep(0.1);//step);
  } //if

  // Track either just crossed volume boundary or is new born -> record time, length, 
  // entry point and momentum; reset energy loss counter and return;
  if (gMC->IsTrackEntering() || gMC->IsNewTrack()) {
    ResetSteppingVariables();

#if _LATER_
    // If energy cutoff is defined for this type of particle, check current
    // energy against it and if below the limit, stop this track and fill out
    // respective entry in fake MC point array;
    {
      // Unify these cheap calls later (see below);
      //Int_t trackID  = gMC->GetStack()->GetCurrentTrackNumber();
      //TParticle *particle = ((PndStack*)gMC->GetStack())->GetParticle(trackID);
      Int_t PDG = particle->GetPdgCode();
      
      // Check that such an entry exists in fCutOffMap table;
      if ((_fCutOffMap && !dbFile) && _fCutOffMap->find(abs(PDG)) != _fCutOffMap->end()) {
	TLorentzVector fMom;
	gMC->TrackMomentum(fMom);
	  
	// Well, I guess kinetic energy should be considered here, right?;
	//printf("%2d: %f %f\n", particle->GetPdgCode(), fMom.E(), particle->GetMass());
	double eKin = fMom.E() - particle->GetMass();
	
	if (eKin >= (*_fCutOffMap)[abs(PDG)].first && eKin <= (*_fCutOffMap)[abs(PDG)].second) {
	  // Also unify with the below call later?;
	  Int_t volumeID = v->getMCid();
#if _LATER_
	  AddFakeMoCaPoint(trackID, GetPrimaryMotherId(), PDG, volumeID, gGeoManager->GetPath(), 
			   GetNodeMultiIndex(), 
			   fPosIn.Vect(), fMomIn.Vect(), fTime, fLength/*, eKin*/);
#endif

	  //printf("  -> will stop this track!\n");
	  gMC->StopTrack();
	  return kTRUE;
	} //if
      } //if
    }
#endif
    
    return kTRUE;
  } /*if*/
  
  fELoss += gMC->Edep();
  fStep  += gMC->TrackStep();

  // Figure out stepping type for this volume;
  SteppingType effectiveSteppingType = qSteppingTypeUndefined;
  // Try sensitive volume list first;
  if (fListOfGeantSensitives) {
    const std::pair<TString, SteppingType> *steppingType = 
      fListOfGeantSensitives->AnyMatch(v->GetName());

    if (steppingType) effectiveSteppingType = steppingType->second;
  } //if
  // Then check if all volumes should be sensitive;
  if (effectiveSteppingType == qSteppingTypeUndefined && mAllVolumesSensitiveFlag) 
    effectiveSteppingType = fStType;
  // Eventually check, that assignment resulted in a meaningful value;
  if (effectiveSteppingType == qSteppingTypeUndefined || 
      // This can hardly happen; just for completeness;
      effectiveSteppingType == qSteppingTypeDefault)
    fLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Sensitive volume with undefined stepping type found (%s)!  \033[0m", 
		   v->GetName());

  {
    //TLorentzVector fPosOut;
    //gMC->TrackPosition(fPosOut);
    //printf("%7.3f %7.3f %d %d %d %d\n", fPosIn.Vect().Perp(), fPosOut.Vect().Perp(), effectiveSteppingType == qOneStepOneHit, 
    //	     gMC->IsTrackExiting(), gMC->IsTrackStop(), gMC->IsTrackDisappeared()); 
    //printf("%7.3f %8.5f\n", fStep, fELoss);
  }
  if (effectiveSteppingType == qOneStepOneHit || gMC->IsTrackExiting() || gMC->IsTrackStop() || 
      gMC->IsTrackDisappeared() ) {
    TLorentzVector fPosOut, fMomOut;
    gMC->TrackPosition(fPosOut);
    gMC->TrackMomentum(fMomOut);
    
    // In fact neutrals can not reach this point unless they were booked 
    // for this sensitive volum explicitely; keep charge check here though;
    if (wanted || (gMC->TrackCharge() && fELoss)) {
      std::pair<int, int> parents = EicBlackHole::GetParentIDs();
      
      //printf("track: %3d; pdg: %d; parents: %4d %4d\n", trackID, particle->GetPdgCode(), parents.first, parents.second);

      //     printf("%7.3f %7.3f %d %d %d %d\n", fPosIn.Vect().Perp(), fPosOut.Vect().Perp(), effectiveSteppingType == qOneStepOneHit, 
      //     gMC->IsTrackExiting(), gMC->IsTrackStop(), gMC->IsTrackDisappeared()); 
      // For now be dumb and record full path; figure out how to deal with sensor IDs later;
      AddMoCaPoint(trackID, parents.first, parents.second, volumeID, 
		   GetNodeMultiIndex(), fPosIn.Vect(), fPosOut.Vect(), 
		   // NB: fStep is not necessarily calculabe from fPosIn and fPosOut
		   // for a curved track -> keep as extra variable;
		   fMomIn.Vect(), fMomOut.Vect(), fTime, fLength, fELoss, fStep);
    } //if
    
    ((PndStack*)gMC->GetStack())->AddPoint(DetectorId(fDetType));
    
    // Reset working stepping variables;
    if (effectiveSteppingType == qOneStepOneHit) ResetSteppingVariables();
  } //if

  return kTRUE;
} // EicDetector::ProcessHits()

// ---------------------------------------------------------------------------------------

TClonesArray* EicDetector::GetCollection(Int_t iColl) const 
{
  if (iColl == 0) 
    return fEicMoCaPointCollection;
  else 
    return NULL;
} // EicDetector::GetCollection()

// ---------------------------------------------------------------------------------------

void EicDetector::Initialize()
{
  // Once the geometry is defined (so volume names are associated with 
  // unique IDs), calculate binary signatures for all maps;
  if (gptr) gptr->CalculateMappingTableSignatures();

  FairDetector::Initialize();
} // EicDetector::Initialize()

// ---------------------------------------------------------------------------------------
#if _LATER_
int EicDetector::createFakeMoCaDatabase(const char *outFileName)
{
  // Open ROOT file;
  fFakeMoCaDatabaseFile = TString(outFileName);
  dbFile = new TFile(fFakeMoCaDatabaseFile, "RECREATE");

  // Declare ROOT tree which will hold entries; 
  dbTree = new TTree(dname->Name() + "EicFakeMoCaPointDatabase", 
		     dname->Name() + "fake MC hit database"); 

  // Alocate buffer variable; think on split, etc; later;
  dbEntry = new EicFakeMoCaPointDbEntry();
  TBranch *branch = dbTree->Branch("EicFakeMoCaPointDbEntry", 
				   "EicFakeMoCaPointDbEntry", &dbEntry, 16000, 1); 
  branch->SetFile(outFileName);

  TFile fgeo(GetGeometryFileName());

  // Yes, expect object with a predefined name; do a better check later;
  fgeo.GetObject(dname->Name() + "EnergyCutOffTable", _fCutOffMap);
  assert(_fCutOffMap); 

  fgeo.Close(); 

  // And allocate the DB array right away;
  //fakeDB->allocateDbMap();

  return 0;
} // EicDetector::createFakeMoCaDatabase()
#endif
// ---------------------------------------------------------------------------------------

void EicDetector::FinishRun()
{
  // Write out energy monitor histogram(s);
  for(unsigned mn=0; mn<mEnergyMonitorVolumes.size(); mn++) {
    EicEnergyMonitor *monitor = mEnergyMonitorVolumes[mn];
    
    monitor->mHistogram->Write();
  } //for mn

  if (gptr || vptr /*|| (_fCutOffMap && !dbFile)*/)
    {
      FairRun *fRun = FairRun::Instance();

      // I guess there is no need to save/restore current directory here?;
      fRun->GetOutputFile()->cd();

      if (gptr) gptr->Write(dname->Name() + "GeoParData");

      // As of 2013/09/29 just dump detector geometry ROOT record into the MC output
      // file; disk space overhead is indeed small; MC files can now be used to import 
      // detector geometry in calls like EicEmc() which comes handy for instance for 
      // safe shower database creation;
      if (vptr) vptr->Write(dname->Name() + "GeantGeoWrapper"); 

      // And also save fast simulation cutoff parameters if specified; but only if fake
      // database creation is NOT requested (since otherwise cutoff table is in fact 
      // imported rather than created);
      //if (_fCutOffMap && !dbFile)
      //fRun->GetOutputFile()->WriteObject(_fCutOffMap, dname->Name() + "EnergyCutOffTable");
    } //if

#if _LATER_
  // Yes, goes to a separate file;
  if (dbFile)
    {
      dbFile->Write();
      dbFile->WriteObject(_fCutOffMap, dname->Name() + "EnergyCutOffTable");
      // Crashes paramater database dump; WHY?; fix later;
      //dbFile->Close();

      // Do this better later, please!;
      //FairRun *fRun = FairRun::Instance();
      //fRun->GetOutputFile()->cd();
    } //if
#endif
} // EicDetector::FinishRun()

// ---------------------------------------------------------------------------------------

void EicDetector::Reset() {
  fEicMoCaPointCollection->Delete();
  //fEicFakeMoCaPointCollection->Delete();

  //ResetParameters();
} // EicDetector::Reset()

// ---------------------------------------------------------------------------------------

void EicDetector::Print() const
{
  Int_t nHits = fEicMoCaPointCollection->GetEntriesFast();
  cout<<"-I- Eic"<< dname->Name() <<": " << nHits << " points registered in this event." << endl;

  if(fVerboseLevel > 1)
    for(Int_t i=0; i<nHits; i++) (*fEicMoCaPointCollection)[i]->Print();
} // EicDetector::Print()

// ---------------------------------------------------------------------------------------

void EicDetector::ConstructGeometry() 
{
  std::cout<<" --- Building " << dname->NAME() << " Geometry ---"<<std::endl;
  std::cout << GetGeometryFileName() << std::endl;
  
  if(GetGeometryFileName().EndsWith(".root")) {
    // For now assume it is quite natural to pack basic geometric parameter 
    // information into the same ROOT file used to describe the GEANT geometry;
    // ideally this should go into GeoPar stuff, but I fail to make it working;
    // so for now just infect the output MC file with this class information;
    // if this ever becomes problematic (say, because of compatibility issues), 
    // consider to look once again into the GeoPar mechanism available in FairRoot;
    {
      TFile fgeo(GetGeometryFileName());

      // Yes, expect object with a predefined name;
      fgeo.GetObject(dname->Name() + "GeoParData", gptr);

      // May want to print out some service info (and then perhaps exit);
      if (gptr && mPrintGeometryInfoFlag) {
	gptr->Print();

	if (mPrintGeometryInfoOption.EqualTo("and exit")) exit(0);
      } //if
      if (gptr && !mAttachedFilePrintoutRequestName.IsNull()) {
	gptr->PrintAttachedSourceFile(mAttachedFilePrintoutRequestName.Data());

	if (mAttachedFilePrintoutOption.EqualTo("and exit")) exit(0);
      } //if

      // Loop through all maps and declare sensitive volumes (top-level
      // volume names of all maps); do this only if no sensitive volumes were
      // declared by hand already (then assume user is able to define him/herself
      // which ones does he want; perhaps activate only fiber cores and not absorber
      // volumes to save CPU time);
      if (gptr && !fListOfGeantSensitives)
	for(int iq=0; iq<gptr->GetMapNum(); iq++)
	  {
	    const EicGeoMap *fmap = gptr->GetMapPtrViaMapID(iq);

	    //printf("@SV@ %s\n", fmap->GetInnermostVolumeName()->Data());
	    // No 2-d argument -> will use stepping type defined at detector initialization;
	    DeclareGeantSensitiveVolume(fmap->GetInnermostVolumeName()->Data());
	  } //if..for iq

      fgeo.GetObject(dname->Name() + "GeantGeoWrapper", vptr); 

      fgeo.Close(); 
    }

    // And make a standard FairRoot call to parse GEANT geometry information;
    ConstructRootGeometry();

    // Once all the volumes are added to the geometry (so their IDs are known), create 
    // step enforced volume lookup table;
    if (gptr && !strcmp(FairRun::Instance()->GetName(), "TGeant4")) {
      // Could not have this all been arranged in a bit more straighforward way?;
      FairGeoLoader *fgLoader = FairGeoLoader::Instance();
      FairGeoInterface *fgInterface = fgLoader->getGeoInterface();
      FairGeoMedia *fgMedia = fgInterface->getMedia();

      for (std::set<TString>::iterator it=gptr->GetStepEnforcedVolumes().begin(); 
	   it!=gptr->GetStepEnforcedVolumes().end(); ++it) {
	TGeoVolume *volume = gGeoManager->GetVolume(it->Data());

	// Well, volume with such name should exist, otherwise something went wrong;
	if (!volume)
	  fLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Step enforcement for unknown "
			 "volume attempted (%s)!  \033[0m", it->Data());
	
	FairGeoMedium *fgMedium = fgMedia->getMedium(volume->GetMedium()->GetName());

	// Assume medium at this point exists for sure; extract max step value;
	// FIXME: well, hardcode for now both 10 and 4?;
	double params[10];
	fgMedium->getMediumPar(params);

	gptr->AddStepEnforcedVolumeLookupEntry(volume->GetNumber(), params[4]);
      } //for it 
    } //if

    //
    // If ever return back to FairRoot GeoPar interface, need to call the same 
    // "par"-stuff as in the ASCII section below;
    //
  } 
  else {
    FairGeoLoader*    geoLoad = FairGeoLoader::Instance();
    FairGeoInterface* geoFace = geoLoad->getGeoInterface();
    EicGeo*              Geo  = new EicGeo(dname->name());
    Geo->setGeomFile(GetGeometryFileName());
    geoFace->addGeoModule(Geo);

    Bool_t rc = geoFace->readSet(Geo);
    if (rc) 
      Geo->create(geoLoad->getGeoBuilder());
    else 
      std::cerr<< dname->Name() <<"Det:: geometry could not be read!"<<std::endl;

    TList* volList = Geo->getListOfVolumes();

    // store geo parameter
    FairRun *fRun = FairRun::Instance();
    FairRuntimeDb *rtdb= FairRun::Instance()->GetRuntimeDb();
    {
      EicGeoPar* par=(EicGeoPar*)(rtdb->getContainer(dname->Name() + "GeoPar"));
#if _LATER_
      TObjArray *fSensNodes = par->GetGeoSensitiveNodes();
      TObjArray *fPassNodes = par->GetGeoPassiveNodes();

      TListIter iter(volList);
      FairGeoNode* node   = NULL;
      FairGeoVolume *aVol=NULL;
    
      while( (node = (FairGeoNode*)iter.Next()) ) {
	aVol = dynamic_cast<FairGeoVolume*> ( node );
	if ( node->isSensitive()  ) {
	  fSensNodes->AddLast( aVol );
	}else{
	  fPassNodes->AddLast( aVol );
	}
      }
#endif

      par->setChanged();
      par->setInputVersion(fRun->GetRunId(),1);

      ProcessNodes ( volList );
    }
  } // if
} // EicDetector::ConstructGeometry() 

// ---------------------------------------------------------------------------------------

ClassImp(EicDetector)
ClassImp(EicDummyDetector) 
ClassImp(EicNamePatternHub<SteppingType>) 
