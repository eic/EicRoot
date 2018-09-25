

#include "CbmRich.h"

#include "CbmGeoRichPar.h"
#include "CbmGeoRich.h"
#include "CbmRichPoint.h"

#include "CbmDetectorList.h"
#include "FairGeoInterface.h"
#include "FairGeoLoader.h"
#include "FairGeoNode.h"
#include "FairGeoRootBuilder.h"
#include "CbmStack.h"
#include "FairRootManager.h"
#include "FairVolume.h"
#include "FairRuntimeDb.h"
#include "FairRun.h"

#include "TObjArray.h"
#include "TClonesArray.h"
#include "TGeoMCGeometry.h"
#include "TGeoManager.h"
#include "TLorentzVector.h"
#include "TParticle.h"
#include "TParticlePDG.h"
#include "TVirtualMC.h"
#include "TGeoMatrix.h"
#include "TGeoNode.h"

#include "FairGeoMedia.h"
#include "FairGeoBuilder.h"

#include "TGDMLParse.h"

#include <iostream>

using std::cout;
using std::endl;


std::map<TString, Int_t> CbmRich::fFixedMats;
Bool_t CbmRich::fIsFirstGDML = kTRUE;

CbmRich::CbmRich() :
   FairDetector("RICH", kTRUE, kRICH),
   fPosIndex(0),

   fRichPoints(NULL),
   fRichRefPlanePoints(NULL),
   fRichMirrorPoints(NULL),
   fRotation(),
   fPositionRotation(),
   mScale(1.0)
{
   fRichPoints = new TClonesArray("CbmRichPoint");
   fRichRefPlanePoints = new TClonesArray("CbmRichPoint");
   fRichMirrorPoints = new TClonesArray("CbmRichPoint");
   fPosIndex = 0;

   fVerboseLevel = 1;
}

CbmRich::CbmRich(
      const char* name,
      Bool_t active,
      Double_t px,
      Double_t py,
      Double_t pz,
      Double_t rx,
      Double_t ry,
      Double_t rz):
   FairDetector(name, active, kRICH),
   fPosIndex(0),
   fRichPoints(new TClonesArray("CbmRichPoint")),
   fRichRefPlanePoints(new TClonesArray("CbmRichPoint")),
   fRichMirrorPoints(new TClonesArray("CbmRichPoint")),

   fRotation(new TGeoRotation("", rx, ry, rz)),
   fPositionRotation(new TGeoCombiTrans(px, py, pz, fRotation)),
   mScale(1.0)
{
   fVerboseLevel = 1;
}

CbmRich::~CbmRich()
{
   if (NULL != fRichPoints) {
      fRichPoints->Delete();
      delete fRichPoints;
   }
   if (NULL != fRichRefPlanePoints) {
      fRichRefPlanePoints->Delete();
      delete fRichRefPlanePoints;
   }

   if (NULL != fRichMirrorPoints) {
      fRichMirrorPoints->Delete();
      delete fRichMirrorPoints;
   }
}

void CbmRich::Initialize()
{
   //return;
   FairDetector::Initialize();
   FairRun* sim = FairRun::Instance();
   FairRuntimeDb* rtdb=sim->GetRuntimeDb();
   CbmGeoRichPar *par=(CbmGeoRichPar*)(rtdb->getContainer("CbmGeoRichPar"));
   TObjArray *fSensNodes = par->GetGeoSensitiveNodes();
}


Bool_t CbmRich::CheckIfSensitive(std::string name)
{
  TString volName = name;
  if ( volName.Contains("rich1d")){
     return kTRUE;
  }
  return kFALSE;
}


Bool_t CbmRich::ProcessHits(
      FairVolume* vol)
{
   Int_t pdgCode = gMC->TrackPid();
   Int_t iVol = vol->getMCid();
   TString volName = TString(vol->GetName());
  // cout << volName << endl;
   //Treat photodetectors : All particles
   if (volName.Contains("rich1d") || volName.Contains("RICH_PMT") ){
      if (gMC->IsTrackEntering()){

         TParticle* part = gMC->GetStack()->GetCurrentTrack();
         Double_t charge = part->GetPDG()->Charge() / 3. ;
         Int_t trackID = gMC->GetStack()->GetCurrentTrackNumber();
         Double_t time = gMC->TrackTime() * 1.0e09;
         Double_t length = gMC->TrackLength();
         Double_t eLoss = gMC->Edep();
         TLorentzVector tPos, tMom;
         gMC->TrackPosition(tPos);
         gMC->TrackMomentum(tMom);

	 //printf("%4d: %d\n", trackID, pdgCode);
         if ( pdgCode == 50000050) { // Cherenkovs only
	   AddHit(trackID, pdgCode, iVol, TVector3(tPos.X(), tPos.Y(), tPos.Z()), TVector3(tMom.Px(), tMom.Py(), tMom.Pz()), time, length, eLoss);

            // Increment number of RichPoints for this track
            CbmStack* stack = (CbmStack*) gMC->GetStack();
            stack->AddPoint(kRICH);
            return kTRUE;
         } else {
            if (charge == 0.) {
               return kFALSE; // no neutrals
            } else { // charged particles
	      AddHit(trackID, pdgCode, iVol, TVector3(tPos.X(), tPos.Y(), tPos.Z()), TVector3(tMom.Px(), tMom.Py(), tMom.Pz()), time, length, eLoss);

               // Increment number of RichPoints for this track
               CbmStack* stack = (CbmStack*) gMC->GetStack();
               stack->AddPoint(kRICH);
               return kTRUE;
            }
         }
      }
   }

   // Treat imaginary plane in front of the mirrors: Only charged particles at entrance
   if (volName == "rich1gas2") {
      // Collecting points of tracks and imaginary plane intersection
      if ( gMC->IsTrackEntering() ) {
         TParticle* part    = gMC->GetStack()->GetCurrentTrack();
         Double_t charge = part->GetPDG()->Charge() / 3. ;
         if (charge == 0.) {
            return kFALSE; // no neutrals
         } else {
            Int_t trackID = gMC->GetStack()->GetCurrentTrackNumber();
            Double_t time = gMC->TrackTime() * 1.0e09;
            Double_t length = gMC->TrackLength();
            Double_t eLoss = gMC->Edep();
            TLorentzVector tPos, tMom;

            gMC->TrackPosition(tPos);
            gMC->TrackMomentum(tMom);

            AddRefPlaneHit(trackID, iVol, TVector3(tPos.X(), tPos.Y(), tPos.Z()), TVector3(tMom.Px(), tMom.Py(), tMom.Pz()), time, length, eLoss);

            //Increment number of RefPlanePoints for this track
            CbmStack* stack = (CbmStack*) gMC->GetStack();
            stack->AddPoint(kREF);
            return kTRUE;
         }
      }
   }

   // Treat mirror points
   if (volName.Contains("rich1mgl") || volName.Contains("rich1mglLU") || volName.Contains("rich1mglRU") ) {

      // Collecting points of tracks and imaginary plane intersection
      if (gMC->IsTrackEntering()) {
         TParticle* part = gMC->GetStack()->GetCurrentTrack();
         Double_t charge = part->GetPDG()->Charge() / 3.;
         if (charge == 0.) {
            return kFALSE; // no neutrals
         } else {

            Int_t trackID = gMC->GetStack()->GetCurrentTrackNumber();

            Double_t time = gMC->TrackTime() * 1.0e09;
            Double_t length = gMC->TrackLength();
            Double_t eLoss = gMC->Edep();
            TLorentzVector tPos, tMom;

            gMC->TrackPosition(tPos);
            gMC->TrackMomentum(tMom);

            // check number of STS points
            //UInt_t points = gMC->GetStack()->GetCurrentTrack()->GetMother(1);
            //Int_t nStsPoints = (points & 15);

            //if (nStsPoints > 0) { // store only particles with STSpoints (at least 1)
               AddMirrorHit(trackID, iVol, TVector3(tPos.X(), tPos.Y(), tPos.Z()), TVector3(tMom.Px(), tMom.Py(), tMom.Pz()), time, length, eLoss);
               return kTRUE;
            //}
         }
      }
   }


   return kFALSE;
}

void CbmRich::EndOfEvent()
{
  if (fVerboseLevel)  Print();
  Reset();
}

void CbmRich::Register()
{
   FairRootManager::Instance()->Register("RichPoint","Rich", fRichPoints, kTRUE);
   FairRootManager::Instance()->Register("RefPlanePoint","RichRefPlane", fRichRefPlanePoints, kTRUE);
   FairRootManager::Instance()->Register("RichMirrorPoint","RichMirror", fRichMirrorPoints, kFALSE);
}

TClonesArray* CbmRich::GetCollection(
      Int_t iColl) const
{
   if (iColl == 0) return fRichPoints;
   if (iColl == 1) return fRichRefPlanePoints;
   if (iColl == 2) return fRichMirrorPoints;
   return NULL;
}

void CbmRich::Print() const
{
   Int_t nHits = fRichPoints->GetEntriesFast();
   cout << "-I- CbmRich: " << nHits << " points registered in this event." << endl;
	
   if (fVerboseLevel > 1) for (Int_t i=0; i<nHits; i++) (*fRichPoints)[i]->Print();
}

void CbmRich::Reset()
{
   fRichPoints->Delete();
   fRichRefPlanePoints->Delete();
   fRichMirrorPoints->Delete();
   fPosIndex = 0;
}

void CbmRich::CopyClones(
      TClonesArray* cl1,
      TClonesArray* cl2,
      Int_t offset )
{
   Int_t nEntries = cl1->GetEntriesFast();
   cout << "-I- CbmRich: " << nEntries << " entries to add." << endl;
   TClonesArray& clref = *cl2;
   CbmRichPoint* oldpoint = NULL;
   for (Int_t i=0; i< nEntries ; i++ ) {
      oldpoint = (CbmRichPoint*) cl1->At(i);
      Int_t index =  oldpoint->GetTrackID() + offset;
      oldpoint->SetTrackID(index);
      new (clref[fPosIndex]) CbmRichPoint(*oldpoint);
      fPosIndex++;
   }
   cout << "-I- CbmRich: " << cl2->GetEntriesFast() << " merged entries." << endl;
}

void CbmRich::ConstructOpGeometry() 
{
	cout<< "CbmRich::ConstructOpGeometry() " <<endl;
}

void CbmRich::ConstructGeometry()
{
   TString fileName = GetGeometryFileName();
     if ( fileName.EndsWith(".root") ) {
        cout << "Constructing RICH geometry from ROOT file: " << fileName.Data() << endl;
        ConstructRootGeometry();
     } else if ( fileName.EndsWith(".geo") ) {
        cout << "-I- Constructing RICH geometry from ASCII file: " << fileName.Data() << endl;
        ConstructAsciiGeometry();
     } else if (fileName.EndsWith(".gdml") ) {
        cout << "-I- Constructing RICH geometry from GDML file: " << fileName.Data() << endl;
        ConstructGdmlGeometry(fPositionRotation);
     } else {
        Fatal("CbmRich::ConstructGeometry", "Geometry format of RICH geometry file is not supported");
     }
}

void CbmRich::ConstructAsciiGeometry()
{
   FairGeoLoader* geoLoad = FairGeoLoader::Instance();
   FairGeoInterface* geoFace = geoLoad->getGeoInterface();
   CbmGeoRich* richGeo = new CbmGeoRich();
   richGeo->setGeomFile(GetGeometryFileName());
   geoFace->addGeoModule(richGeo);

   Bool_t rc = geoFace->readSet(richGeo);
   if (rc) richGeo->create(geoLoad->getGeoBuilder());
   TList* volList = richGeo->getListOfVolumes();

   // store geo parameter
   FairRun *fRun = FairRun::Instance();
   FairRuntimeDb *rtdb= FairRun::Instance()->GetRuntimeDb();
   CbmGeoRichPar* par=(CbmGeoRichPar*)(rtdb->getContainer("CbmGeoRichPar"));
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
   par->setChanged();
   par->setInputVersion(fRun->GetRunId(),1);

   ProcessNodes ( volList );

   // add support structure for mirrors
   TGeoMaterial * matAl = new TGeoMaterial("Al", 26.98, 13, 2.7);
   TGeoMedium * Al = new TGeoMedium("Al",1, matAl);
   TGeoVolume * volume = gGeoManager->MakeTube("grid", Al, 1.3, 1.5, 180);

   gGeoManager->Matrix(123456, 180, 0, 90, 90 , 90 , 0);//z rotation
   gGeoManager->Matrix(123457, 90, 0, 180, 0, 90, 90);// y rotation
   
   Double_t * buf = 0;
   for (Int_t i = 0; i< 11; i++) {
      if (i == 5) continue;
      gGeoManager->Node("grid", 2*i+1, "rich1gas3", 36*i - 180, 0, 40, 123457, kTRUE, buf, 0);
      gGeoManager->Node("grid", 2*i+2, "rich1gas3", 0, 36*i - 180, 48, 123456, kTRUE, buf, 0);
   }
}

#define _RICH_GAS_VOLUME_ ("RICH_gas_1")
#define _AEROGEL_MEDIUM_  ("Aerogel")

TGeoVolume *CbmRich::GetGasVolume(TGeoVolume *current)
{
  // Hardcoded to the moment;
  if (!strcmp(current->GetName(), _RICH_GAS_VOLUME_)) return current;

  for (unsigned nd=0; nd<current->GetNdaughters(); nd++) {
    TGeoNode *node = current->GetNode(nd);
    TGeoVolume *volume = node->GetVolume();
    
    if (!strcmp(volume->GetName(), _RICH_GAS_VOLUME_)) return volume;

    TGeoVolume *ret = GetGasVolume(volume);
    if (ret) return ret;
  } //for nd
  
  return 0;
} // CbmRich::GetGasVolume()

#include <assert.h>
#include <TGeoBBox.h>
//#include <EicMediaHub.h>

//
//  NB: one needs to re-define GetScale() function in TGDMLParse as
//
// protected:
//    virtual TString GetScale(const char* unit);
//
//   and recompile (make install) ROOT to have this working;
//

class EicGDMLParse: public TGDMLParse 
{
public:
  EicGDMLParse(): mScale(1.0) {};
  ~EicGDMLParse() {};

  TGeoVolume *GDMLReadFile(const char* filename, double scale = 1.0) {
    mScale = scale;

    return TGDMLParse::GDMLReadFile(filename);
  };

  TString GetScale(const char* unit) {
    double extraScale = 0.0;

    // Well, same quality code as original TGDMLParse.cxx, sorry;
    if (strcmp(unit, "mm") == 0) 
      extraScale = 0.1;
    else if (strcmp(unit, "milimeter") == 0) 
      extraScale = 0.1;
    else if (strcmp(unit, "cm") == 0)  
      extraScale = 1.0;
    else if (strcmp(unit, "centimeter") == 0)   
      extraScale = 1.0;
    else if (strcmp(unit, "m") == 0)   
      extraScale = 100.0;
    else if (strcmp(unit, "meter") == 0)   
      extraScale = 100.0;
    else if (strcmp(unit, "km") == 0)  
      extraScale = 100000.0;
    else if (strcmp(unit, "kilometer") == 0)  
      extraScale = 100000.0;

    if (extraScale) 
      // Pattern found -> proceed and return;
      return TString::Format("%f", extraScale * mScale);
    else
      // Perhaps angular units, etc -> fall back to the original TGDMLParse call;
      //return TGDMLParse::GetScale(unit);
    assert(0);
  };

private:
  double mScale;
};

void CbmRich::ConstructGdmlGeometry(TGeoMatrix* geoMatrix)
{
   TFile *old = gFile;
   //@@@TGDMLParse parser;
   EicGDMLParse parser;
   TGeoVolume* gdmlTop;
   gdmlTop = parser.GDMLReadFile(GetGeometryFileName(), mScale);
   //gdmlTop = parser.GDMLReadFile(GetGeometryFileName().Data());
   gGeoManager->GetTopVolume()->AddNode(gdmlTop,1,geoMatrix);
   //gGeoManager->GetTopVolume()->AddNode(gdmlTop,1, new TGeoScale(0.5, 0.5, 0.5));
   ExpandNodeForGdml(gGeoManager->GetTopVolume()->GetNode(gGeoManager->GetTopVolume()->GetNdaughters()-1));
   fIsFirstGDML = 0;
   gFile = old;

   // Hardcode colors, sorry; this crap should disappear from the distribution eventually anyway
   // and be replaced by a proper RICH geometry and other stuff;
   {
     // Set up colors and transparency values;
     TIter next( gGeoManager->GetListOfVolumes() );

     TGeoVolume *volume;

     while ((volume=(TGeoVolume*)next())) {
       TString name = volume->GetName();

       if (name.BeginsWith("RICH_pipe")) 
	 volume->SetVisibility(kFALSE);
       else if (name.BeginsWith("RICH_gas") || name.EqualTo("RICH_small_frame_container")) 
	 volume->SetVisibility(kFALSE);
       else if (name.BeginsWith("RICH_PMT")) {
	 volume->SetLineColor(kRed);
	 volume->SetFillColor(kRed);
	 volume->SetFillStyle(4000+40);
       }
       else if (name.BeginsWith("RICH_covering")) {
	 volume->SetLineColor(kMagenta);
	 volume->SetFillColor(kMagenta);
	 volume->SetFillStyle(4000+40);
       }
       else if (name.BeginsWith("RICH_main_frame_trap_pillar")) {
	 volume->SetLineColor(kMagenta);
	 volume->SetFillColor(kMagenta);
	 volume->SetFillStyle(4000+70);
       }
       else if (name.BeginsWith("RICH_mirror_support_belt_pillar")) {
	 volume->SetLineColor(kBlack);
	 volume->SetFillColor(kBlack);
	 volume->SetFillStyle(4000+50);
       }
       else if (name.BeginsWith("RICH_mirror")) {
	 volume->SetLineColor(kGray);
	 volume->SetFillColor(kGray);
	 //volume->SetTransparency(40);
       } //if
     } //while
   }

   // Add aerogel volume by hand; not really possible with the present CBM design; 
#if _LATER_
   //EicMediaHub *mediaHub = new EicMediaHub((char *)"Aerogel");
   //assert(mediaHub);
   //mediaHub->Init();
   //assert(mediaHub->fSingleMedium);

   // Not the most economic way, but GDML file have very few nodes in 
   // the tree, so overhead is small;
   TGeoVolume *gasVolume = GetGasVolume(gdmlTop);
   assert(gasVolume);
   TGeoBBox *aerogel = new TGeoBBox("Aerogel", 10., 10., 2.5);
   TGeoMedium *medium = gGeoManager->GetMedium(_AEROGEL_MEDIUM_);
   //assert(medium);
   if (!medium) {
     FairGeoLoader *geoLoad = FairGeoLoader::Instance();
     FairGeoInterface *geoFace = geoLoad->getGeoInterface();
  
     FairGeoMedia *Media =  geoFace->getMedia();
     FairGeoBuilder *geobuild = geoLoad->getGeoBuilder();

     FairGeoMedium *fmedium  = Media->getMedium(_AEROGEL_MEDIUM_);
     // Would mean no such medium in media.geo file;
     assert(fmedium);

     geobuild->createMedium(fmedium);

     // Should not happen;
     medium = gGeoManager->GetMedium(_AEROGEL_MEDIUM_);
     assert(medium);
   }

   TGeoVolume *vaerogel = new TGeoVolume("Aerogel", aerogel, /*mediaHub->fSingleMedium*/medium);
   gasVolume->AddNode(vaerogel, 0, 0, 0);
   vaerogel->RegisterYourself();
#endif
}

void CbmRich::ExpandNodeForGdml(TGeoNode* node)
{
   TGeoVolume* curVol = node->GetVolume();

   //! Assembly-volumes are skipped as they do not have any material
   if (!curVol->IsAssembly()) {
      TString curMedName = node->GetMedium()->GetName();
      TGeoMedium* curMedInGeoManager = gGeoManager->GetMedium(curMedName);
      Int_t matIndToDel = gGeoManager->GetMaterialIndex(curMedName);

      if (curMedName.BeginsWith("G4_")) {
         curMedName.Remove(0, 3);
      }

      Int_t nmed;

      FairGeoLoader* geoLoad = FairGeoLoader::Instance();
      FairGeoInterface* geoFace = geoLoad->getGeoInterface();
      FairGeoMedia* geoMediaBase =  geoFace->getMedia();
      FairGeoBuilder* geobuild = geoLoad->getGeoBuilder();
      FairGeoMedium* curMedInGeo;

      if (curMedInGeoManager == 0) {
         std::cout << "[ExpandNodeForGDML] New medium found in gmdl - it is not in gGeoManager list." << std::endl;
         //! New medium found in gmdl - it is not in gGeoManager list.
         //! This should never happen as GDML parser adds medium into the list.
         //! If happens - something is extremely strange.
      } else {
         //! Medium is in the list in gGeoManager.
         //! Trying to replace it with the one from the Geo file.

         curMedInGeo = geoMediaBase->getMedium(curMedName);
         if (curMedInGeo == 0)
         {
	   printf("Q: %s!\n", curMedName.Data());
            std::cout << "[ExpandNodeForGDML] Media not found in Geo file." << std::endl;
            //! This should not happen.
            //! This means that somebody uses material in GDML that is not in the media.geo file.
            //! Most probably this is the sign to the user to check materials' names in the CATIA model.
         }
         else
         {
            if (fFixedMats.find(curMedName) == fFixedMats.end()) {
               nmed = geobuild->createMedium(curMedInGeo);
               fFixedMats[curMedName] = gGeoManager->GetListOfMedia()->GetEntries();
            }
            node->GetVolume()->SetMedium(gGeoManager->GetMedium(curMedName));
            gGeoManager->SetAllIndex();
         }
      }

      //! The volume is sensitive => add it to the list
      if (curMedInGeo->getSensitivityFlag()) {
         AddSensitiveVolume(curVol);
      }
   }

   //! Recursevly go down the tree of nodes
   if (curVol->GetNdaughters() != 0)
   {
      TObjArray* NodeChildList = curVol->GetNodes();
      TGeoNode* curNodeChild;
      for (Int_t j=0; j<NodeChildList->GetEntriesFast(); j++)
      {
         curNodeChild = (TGeoNode*)NodeChildList->At(j);
         ExpandNodeForGdml(curNodeChild);
      }
   }
}

CbmRichPoint* CbmRich::AddHit(
      Int_t trackID,
      Int_t pdg,
      Int_t detID,
      TVector3 pos,
	   TVector3 mom,
	   Double_t time,
	   Double_t length,
	   Double_t eLoss)
{
   TClonesArray& clref = *fRichPoints;
   Int_t size = clref.GetEntriesFast();
   //printf("%d\n", trackID);
   return new(clref[size]) CbmRichPoint(trackID, pdg, detID, pos, mom, time,length, eLoss);
}

CbmRichPoint* CbmRich::AddRefPlaneHit(
      Int_t trackID,
      Int_t detID,
		TVector3 pos,
		TVector3 mom,
		Double_t time,
		Double_t length,
		Double_t eLoss)
{
   TClonesArray& clref = *fRichRefPlanePoints;
   Int_t tsize = clref.GetEntriesFast();
   return new(clref[tsize]) CbmRichPoint(trackID, 0, detID, pos, mom, time,length, eLoss);
}

CbmRichPoint* CbmRich::AddMirrorHit(
         Int_t trackID,
         Int_t detID,
         TVector3 pos,
         TVector3 mom,
         Double_t time,
         Double_t length,
         Double_t eLoss)
{
   TClonesArray& clref = *fRichMirrorPoints;
   Int_t tsize = clref.GetEntriesFast();
   return new(clref[tsize]) CbmRichPoint(trackID, 0, detID, pos, mom, time,length, eLoss);
}

ClassImp(CbmRich)
