// -------------------------------------------------------------------------
// -----                      PndMCTrack source file                   -----
// -----                  Created 03/08/04  by V. Friese (CbmMCTrack)  -----
// -----                  Created 11/02/09  by  M. Al-Turany           -----
// -------------------------------------------------------------------------


#include <iostream>

#include "PndMCTrack.h"
#include <limits>
using namespace std;
// -----   Default constructor   -------------------------------------------
PndMCTrack::PndMCTrack():
  fPdgCode(0),
  fMotherID(-1),
  fSecondMotherID(-1),
  fPoints(0),
  fStartX(0), fStartY(0), fStartZ(0), fStartT(0),
  fPx(0), fPy(0), fPz(0), fE(0),
  fGeneratorFlags(0)
{
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
/*// Not used at all?
PndMCTrack::PndMCTrack(Int_t pdgCode, Int_t motherID, TVector3 startVertex, 
		       Double_t startTime, TLorentzVector momentum, Int_t  nPoint){
  fPdgCode  = pdgCode;
  fMotherID = motherID;
  fSecondMotherID = -1;
  fStartX   = startVertex.X();
  fStartY   = startVertex.Y();
  fStartZ   = startVertex.Z();
  fStartT   = startTime;
  fPx       = momentum.Px();
  fPy       = momentum.Py();
  fPz       = momentum.Pz();
  fE        = momentum.E();
  if (nPoint >= 0) fPoints = nPoint;
  else             fPoints = 0;
  fGeneratorFlags=0;
}
*/
// -------------------------------------------------------------------------



// -----   Copy constructor   ----------------------------------------------
PndMCTrack::PndMCTrack(const PndMCTrack& track) :
  fPdgCode(track.fPdgCode),
  fMotherID(track.fMotherID),
  fSecondMotherID(track.fSecondMotherID),
  fPoints(track.fPoints),
  fStartX(track.fStartX), fStartY(track.fStartY), fStartZ(track.fStartZ), fStartT(track.fStartT),
  fPx(track.fPx), fPy(track.fPy), fPz(track.fPz), fE(track.fE),
  fGeneratorFlags(track.fGeneratorFlags)
{
}
// -------------------------------------------------------------------------



// -----   Constructor from TParticle   ------------------------------------
PndMCTrack::PndMCTrack(TParticle* part) :
  fPdgCode(part->GetPdgCode()),
  fMotherID(part->GetMother(0)),
  fSecondMotherID(part->GetMother(1)),
  fStartX(part->Vx()),
  fStartY(part->Vy()),
  fStartZ(part->Vz()),
  fStartT(part->T()*1e09),
  fPx(part->Px()),
  fPy(part->Py()),
  fPz(part->Pz()),
  fE(part->Energy()),
  fPoints(0),
  fGeneratorFlags(0)
{
}
// -------------------------------------------------------------------------


  
// -----   Destructor   ----------------------------------------------------
PndMCTrack::~PndMCTrack() { }
// -------------------------------------------------------------------------



// -----   Public method Print   -------------------------------------------
void PndMCTrack::Print(Int_t trackID) const {
  cout << "Track " << trackID << ", mother : " << fMotherID <<", secondmother : " << fSecondMotherID << ", Type "
       << fPdgCode << ", momentum (" << fPx << ", " << fPy << ", " << fPz<< ", " << fE
       << ") GeV" << " , Generatorflags: "<<fGeneratorFlags<<endl;
}
// -------------------------------------------------------------------------

TLorentzVector PndMCTrack::Get4Momentum() const {
/*
    Double_t mass=0.0;
    Double_t ene=0.0;
    TParticlePDG*
    fParticlePDG = TDatabasePDG::Instance()->GetParticle(fPdgCode);

    if (fParticlePDG)
     mass   = fParticlePDG->Mass();

    if ( mass >= 0 ) {
	ene  = TMath::Sqrt(mass*mass + fPx*fPx +fPy*fPy +fPz*fPz);
    }
*/
   return TLorentzVector(fPx,fPy,fPz,fE);
}

// -----   Public method GetNPoints   --------------------------------------
Int_t  PndMCTrack::GetNPoints(DetectorId detId) const {
  if      ( detId == kDRC ) return ( (fPoints  & (3 <<  0) ) >>  0 );
  else if ( detId == kMDT ) return ( (fPoints  & (3 <<  2) ) >>  2 );
  else if ( detId == kMVD ) return ( (fPoints  & (3 <<  4) ) >>  4 );
  else if ( detId == kRICH) return ( (fPoints  & (3 <<  6) ) >>  6 );
  else if ( detId == kEMC ) return ( (fPoints  & (3 <<  8) ) >>  8 ); 
  else if ( detId == kSTT ) return ( (fPoints  & (3 << 10) ) >> 10 );
  else if ( detId == kFTOF) return ( (fPoints  & (3 << 12) ) >> 12 );
  else if ( detId == kTOF ) return ( (fPoints  & (3 << 14) ) >> 14 );
  else if ( detId == kGEM ) return ( (fPoints  & (3 << 16) ) >> 16 );
  else if ( detId == kDSK ) return ( (fPoints  & (3 << 18) ) >> 18 );
  else if ( detId == kHYP ) return ( (fPoints  & (3 << 20) ) >> 20 );
  else if ( detId == kRPC ) return ( (fPoints  & (3 << 22) ) >> 22 );
  else if ( detId == kLUMI) return ( (fPoints  & (3 << 24) ) >> 24 );	
  else if ( detId == kHYPG) return ( (fPoints  & (3 << 26) ) >> 26 );
  else if ( detId == kFTS)  return ( (fPoints  & (3 << 28) ) >> 28 );
  		
  else {
    cout << "-E- PndMCTrack::GetNPoints: Unknown detector ID "
	 << detId << endl;
    return 0;
  }
}
void PndMCTrack::SetNPoints(Int_t iDet, Int_t  nPoints) {

  if        ( iDet == kDRC ) {
    if      ( nPoints < 0 ) nPoints = 0;
    else if ( nPoints > 3 ) nPoints = 3;
    fPoints = ( fPoints & ( ~ ( 3 <<  0 ) ) )  |  ( nPoints <<  0 );
  }

  else if ( iDet == kMDT ) {
    if      ( nPoints < 0 ) nPoints = 0;
    else if ( nPoints > 3 ) nPoints = 3;
    fPoints = ( fPoints & ( ~ ( 3 <<  2 ) ) )  |  ( nPoints <<  2 );
  }

  else if ( iDet == kMVD ) {
    if      ( nPoints < 0 ) nPoints = 0;
    else if ( nPoints > 3 ) nPoints = 3;
    fPoints = ( fPoints & ( ~ (  3 << 4 ) ) )  |  ( nPoints <<  4 );
  }

  else if ( iDet == kRICH ) {
    if      ( nPoints < 0 ) nPoints = 0;
    else if ( nPoints > 3 ) nPoints = 3;
    fPoints = ( fPoints & ( ~ ( 3 <<  6 ) ) )  |  ( nPoints <<  6);
  }

  else if ( iDet == kEMC ) {
    if      ( nPoints < 0 ) nPoints = 0;
    else if ( nPoints > 3 ) nPoints = 3;
    fPoints = ( fPoints & ( ~ ( 3 <<  8 ) ) )  |  ( nPoints <<  8 );
  }

  else if ( iDet == kSTT ) {
    if      ( nPoints < 0 ) nPoints = 0;
    else if ( nPoints > 3 ) nPoints = 3;
    fPoints = ( fPoints & ( ~ ( 3 << 10 ) ) )  |  ( nPoints << 10 );
  }

  else if ( iDet == kFTOF ) {
    if      ( nPoints < 0 ) nPoints = 0;
    else if ( nPoints > 3 ) nPoints = 3;
    fPoints = ( fPoints & ( ~ ( 3 << 12 ) ) )  |  ( nPoints << 12 );
  }

  else if ( iDet == kTOF ) {
    if      ( nPoints < 0 ) nPoints = 0;
    else if ( nPoints > 3 ) nPoints = 3;
    fPoints = ( fPoints & ( ~ ( 3 << 14 ) ) )  |  ( nPoints << 14 );
  }

  else if ( iDet == kGEM ) {
    if      ( nPoints < 0 ) nPoints = 0;
    else if ( nPoints > 3 ) nPoints = 3;
    fPoints = ( fPoints & ( ~ ( 3 << 16 ) ) )  |  ( nPoints << 16 );
  }
  else if ( iDet == kDSK ) {
    if      ( nPoints < 0 ) nPoints = 0;
    else if ( nPoints > 3 ) nPoints = 3;
    fPoints = ( fPoints & ( ~ ( 3 << 18 ) ) )  |  ( nPoints << 18 );
  }

  else if ( iDet == kHYP ) {
    if      ( nPoints < 0 ) nPoints = 0;
    else if ( nPoints > 3 ) nPoints = 3;
    fPoints = ( fPoints & ( ~ ( 3 << 20 ) ) )  |  ( nPoints << 20 );
  }
  else if ( iDet == kRPC ) {
    if      ( nPoints < 0 ) nPoints = 0;
    else if ( nPoints > 3 ) nPoints = 3;
    fPoints = ( fPoints & ( ~ ( 3 << 22 ) ) )  |  ( nPoints << 22 );
  }

  else if ( iDet == kLUMI ) {
    if      ( nPoints < 0 ) nPoints = 0;
    else if ( nPoints > 3 ) nPoints = 3;
    fPoints = ( fPoints & ( ~ ( 3 << 24 ) ) )  |  ( nPoints << 24 );
  }
	
  else if ( iDet == kHYPG ) {
    if      ( nPoints < 0 ) nPoints = 0;
    else if ( nPoints > 3 ) nPoints = 3;
    fPoints = ( fPoints & ( ~ ( 3 << 26 ) ) )  |  ( nPoints << 26 );
  }

  else if ( iDet == kFTS ) {
    if      ( nPoints < 0 ) nPoints = 0;
    else if ( nPoints > 3 ) nPoints = 3;
    fPoints = ( fPoints & ( ~ ( 3 << 28 ) ) )  |  ( nPoints << 28 );
  }
	
	
	
  else cout << "-E- PndMCTrack::SetNPoints: Unknown detector ID "
	    << iDet << endl;

}
//


ClassImp(PndMCTrack)
