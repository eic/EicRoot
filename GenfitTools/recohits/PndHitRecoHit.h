//-----------------------------------------------------------
// File and Version Information:
// $Id$
//
// Description:
//      3d reco hit
//
// Environment:
//      Software developed for the PANDA Detector at FAIR.
//
// Author List:
//      Sebastian Neubert    TUM            (original author)
//			Ralf Kliemt					 TUD						(application to PndSds)
//
//-----------------------------------------------------------

//! A planar (x,y) reco hit & its sensor plane
/** @author Ralf Kliemt Uni Bonn
 */
#ifndef PNDHITRECOHIT_H
#define PNDHITRECOHIT_H

// Base Class Headers ----------------
#include "GFRecoHitIfc.h"
#include "GFSpacepointHitPolicy.h"

// Collaborating Class Headers -------
#include <ostream> // remove if you do not need streaming op

// Collaborating Class Declarations --
#include "FairMCPoint.h"
#include "FairHit.h"


class PndHitRecoHit : public GFRecoHitIfc<GFSpacepointHitPolicy> {
public:

  // Constructors/Destructors ---------
  PndHitRecoHit();

  PndHitRecoHit(FairMCPoint* point);//from lab MC points
  PndHitRecoHit(FairHit* hit);//from lab cluster hits

  virtual ~PndHitRecoHit();

  virtual GFAbsRecoHit* clone(){return new PndHitRecoHit(*this);};



  // Operations ----------------------
  virtual TMatrixT<double> getHMatrix(const GFAbsTrackRep* stateVector);


  virtual Double_t residualScalar(GFAbsTrackRep* stateVector,
                                const TMatrixT<Double_t>& state);


private:

  // Private Data Members ------------
  static const Int_t fNparHitRep = 3;

  // Private Methods -----------------

//public:
  ClassDef(PndHitRecoHit,1);

};

#endif

