/** CbmPropagator.h
 **
 ** Interface for track propagation algorithms.
 **/ 

#ifndef CBMPROPAGATOR_H_
#define CBMPROPAGATOR_H_

#include "TObject.h"
#include "TMatrixD.h"

#include "CbmStatusCode.h"

#include "Rtypes.h"

#include <vector>

class FairTrackParam;

//class TMatrixD;

class CbmPropagator : public TObject {
public:
   CbmPropagator();
   virtual ~CbmPropagator();
   
   /** Pure virtual function for track parameters propagation
    ** @param parIn            input track parameter
    ** @param parOut           output track parameter
    ** @param zOut             z position to propagate to
    ** @param pdg              PDG code of the particle 
    **/
   virtual StatusCode Propagate( 
		   const FairTrackParam *parIn,
           FairTrackParam *parOut,
           Double_t zOut,
           Int_t pdg) = 0;
    
   /** Pure virtual function for track parameters propagation
    ** @param par              input/output track parameter
    ** @param zOut             z position to propagate to
    ** @param pdg              PDG code of the particle 
    **/
   virtual StatusCode Propagate( 
		   FairTrackParam *par, 
           Double_t zOut,
           Int_t pdg) = 0;
   
   /** Pure virtual function to access the transport matrix
    ** @param F      output 5x5 transport matrix as a vector with 25 elements
    **/
   virtual void TransportMatrix(
		   std::vector<Double_t>& F) = 0;
   
   /** Pure virtual function to access the transport matrix
    ** @param F      output 5x5 transport matrix as a TMatrixD class 
    **/
   virtual void TransportMatrix(
		   TMatrixD& F) = 0;
      
   ClassDef(CbmPropagator,1);
}; 

#endif /*CBMPROPAGATOR_H_*/
