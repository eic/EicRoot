//
// AYK (ayk@bnl.gov), 2013/06/13
//
// A simple split interface to FairEventManager
//

#include "TEveManager.h"
#include "TGeoManager.h"
#include "TEveGeoNode.h"

#include <EicEventManager.h>
#include <EicRunAna.h>

// ---------------------------------------------------------------------------------------

EicEventManager::EicEventManager(): mInitCallHappened(false)
{
  // Figure out whether FairRunAna instance exists; initialize it if not (and save fRunAna);
  if (!FairRunAna::Instance()) fRunAna = new EicRunAna();

  // I guess output is not needed?;
  fRunAna->SetOutputFile("/dev/null");
} // EicEventManager::EicEventManager()

// ---------------------------------------------------------------------------------------

void EicEventManager::SetInputFile(TString fname)
{
  fRunAna->SetInputFile(fname);
} // EicEventManager::SetInputFile()

// ---------------------------------------------------------------------------------------

void EicEventManager::Init()
{
  TEveManager::Create();

  EicRunAna *fRun = dynamic_cast<EicRunAna*>(fRunAna);
  if (fRun)
    fRun->Init();
  // Leave this fall-back option as well, so that both new-style (EicRunAna-based)
  // and old-style (FairRunAna-based + all the crap to initialize FairRuntimeDb by hand)
  // will hopefully work;
  else
    fRunAna->Init();

  // Make certain volumes either invisible or transparent; the rest of visual 
  // attributes should be encoded in geometry ROOT files (and propagated to 
  // the overall geometry tree of the simulation setup);
  if(gGeoManager) {
    TIter next( gGeoManager->GetListOfVolumes() );

    TGeoVolume *volume;

    while ((volume=(TGeoVolume*)next())) {
      // Make 'cave' volume invisible; FIXME: yes, looks like a hack (may want to 
      // create cave.root files instead of cave.geo ones and fix this);
      if (TString(volume->GetName()).BeginsWith("cave")) volume->SetVisibility(kFALSE);

      // Also do not want to see container volumes;
      if (TString(volume->GetName()).Contains("ContainerVolume")) volume->SetVisibility(kFALSE);

      // For whatever reason TGeant3 mode in simulation.C requires this;
      volume->SetLineColor(volume->GetFillColor());
      volume->SetFillColor(volume->GetFillColor());

      // FIXME: base-4000 here also looks like a hack; also it should be kept in mind, 
      // that transparency value is set on per-material basis rather than on per-volume
      // (see SetTransparency() call source code in TGeoVolume); so there better be no 
      // overlap in different subdetector materials (like iron yoke) which are meant to 
      // be transparent per default;
      if (volume->IsTransparent()) volume->SetTransparency(volume->GetFillStyle()-4000);
    } //while
  } //if

  mInitCallHappened = true;
} // EicEventManager::Init()

// ---------------------------------------------------------------------------------------

void EicEventManager::Run(Int_t visopt, Int_t vislvl, Int_t maxvisnds)
{
  if (!mInitCallHappened) Init();

  if(gGeoManager) {
    TGeoNode* N=  gGeoManager->GetTopNode();
    TEveGeoTopNode* TNod=new  TEveGeoTopNode(gGeoManager, N, visopt, vislvl, maxvisnds);
    gEve->AddGlobalElement(TNod);
    gEve->FullRedraw3D(kTRUE);
    //gEve->FullRedraw3D(kTRUE, kTRUE);
    //gGeoManager->SetNsegments(1000);
  //printf("%d\n", gGeoManager->GetNsegments()); exit(0);

    fEvent= gEve->AddEvent(this);
  }
} // EicEventManager::Run()

// ---------------------------------------------------------------------------------------

#if _SAVE_
  fMan->Init();  
  // Change colors and/or visibility (transparency) if needed;
  setColors();              
fMan->Run(0, 6);                     
} // eventDisplay()
  
setColors()
{
  TIter next( gGeoManager->GetListOfVolumes() );

  while ((volume=(TGeoVolume*)next())) {
    TString name = volume->GetName();

    if (name.BeginsWith("Whatever")) {
      volume->SetLineColor(kMagenta);
      volume->SetFillColor(kMagenta); 
    } //if
  } //while
} // setColors()
#endif

// ---------------------------------------------------------------------------------------

ClassImp(EicEventManager)
