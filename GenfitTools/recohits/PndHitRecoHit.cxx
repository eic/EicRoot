//-----------------------------------------------------------
// File and Version Information:
// $Id$
//
// Description:
//      Implementation of class PndHitRecoHit
//      see PndHitRecoHit.h for details
//
// Environment:
//      Software developed for the PANDA Detector at FAIR.
//
// Author List:
//      Sebastian Neubert    TUM            (original author)
//      Ralf Kliemt          Uni Bonn       (application to FairHits)
//
//-----------------------------------------------------------

// C/C++ Headers ----------------------
// root Headers ----------------------
#include "TMatrixT.h"
#include "TMath.h"
// Collaborating Class Headers --------
#include "LSLTrackRep.h"
#include "GeaneTrackRep.h"
// This Class' Header ------------------
#include "PndHitRecoHit.h"

// Class Member definitions -----------

ClassImp(PndHitRecoHit);


PndHitRecoHit::~PndHitRecoHit()
{
}

PndHitRecoHit::PndHitRecoHit()
: GFRecoHitIfc<GFSpacepointHitPolicy>(fNparHitRep)
{
}


PndHitRecoHit::PndHitRecoHit(FairMCPoint* point)
: GFRecoHitIfc<GFSpacepointHitPolicy>(fNparHitRep)
{
  //std::cout<<" -I- PndHitRecoHit::PndHitRecoHit(FairMCPoint*) called."<<std::endl;

  fHitCoord[0][0] =  point->GetX();
  fHitCoord[1][0] =  point->GetY();
  fHitCoord[2][0] =  point->GetZ();
  
  // fixed errors on the monte carlo points
  // we set the covariances to (500 mum)^2 by hand.
  Double_t sigmasq=0.05*0.05; //cm //TODO: cm is rigt?
  fHitCov[0][0] = sigmasq;
  fHitCov[1][1] = sigmasq;
  fHitCov[2][2] = sigmasq;
  
}

PndHitRecoHit::PndHitRecoHit(FairHit* hit)
: GFRecoHitIfc<GFSpacepointHitPolicy>(fNparHitRep)
{
  //  std::cout<<" -I- PndHitRecoHit::PndHitRecoHit(PndSdsHit*) called."<<std::endl;
   
  fHitCoord[0][0] = hit->GetX();
  fHitCoord[1][0] = hit->GetY();
  fHitCoord[2][0] = hit->GetZ();
  
  fHitCov[0][0] = hit->GetDx() * hit->GetDx();
  fHitCov[1][1] = hit->GetDy() * hit->GetDy();
  fHitCov[2][2] = hit->GetDz() * hit->GetDz();

}
//============================================================================



TMatrixT<double>
PndHitRecoHit::getHMatrix(const GFAbsTrackRep* stateVector)
{
  
  // !! TODO I copied this from the DemoRecoHit - check validity!!!
  if (dynamic_cast<const GeaneTrackRep*>(stateVector) != NULL) {
    // Uses TrackParP (q/p,v',w',v,w)
    // coordinates are defined by detplane!
    TMatrixT<double> HMatrix(fNparHitRep,5);
    
    HMatrix[0][0] = 0.;
    HMatrix[0][1] = 0.;
    HMatrix[0][2] = 0.;
    HMatrix[0][3] = 1.;
    HMatrix[0][4] = 0.;
    
    HMatrix[1][0] = 0.;
    HMatrix[1][1] = 0.;
    HMatrix[1][2] = 0.;
    HMatrix[1][3] = 0.;
    HMatrix[1][4] = 1.;
    return HMatrix;
  }
  else if (dynamic_cast<const LSLTrackRep*>(stateVector) != NULL) {
    // LSLTrackRep (x,y,x',y',q/p) recohits are (Xloc,Yloc,0.)
    // The virtual detector plane in LSL is perpendicular to Zlab
    TMatrixT<double> HMatrix(fNparHitRep,5);
    HMatrix[0][0] = 1.;
    HMatrix[0][1] = 0.;
    HMatrix[0][2] = 0.;
    HMatrix[0][3] = 0.;
    HMatrix[0][4] = 0.;
    
    HMatrix[1][0] = 0.;
    HMatrix[1][1] = 1.;
    HMatrix[1][2] = 0.;
    HMatrix[1][3] = 0.;
    HMatrix[1][4] = 0.;
    return HMatrix;
  }
  else {
    std::cerr << "PndHitRecoHit can only handle state"
    << " vectors of type LSLTrackRep or GeaneTrackRep -> abort"
    << std::endl;
    throw;
  }
  
}

Double_t
PndHitRecoHit::residualScalar(GFAbsTrackRep* stateVector,
                              const TMatrixT<Double_t>& state)
{
  std::cerr<<"PndHitRecoHit::residualScalar() called. Throw exception."<<std::endl;
  throw;
}

