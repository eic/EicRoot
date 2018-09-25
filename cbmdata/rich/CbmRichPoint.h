// -------------------------------------------------------------------------
// -----                     CbmRichPoint header file                  -----
// -----               Created 28/04/04  by B. Polichtchouk            -----
// -------------------------------------------------------------------------


/**  CbmRichPoint.h
 *@author B. Polichtchouk
 **
 ** Interception of MC track with an RICH photodetector.
 **/


#ifndef CBMRICHPOINT_H
#define CBMRICHPOINT_H 1


#include "FairMCPoint.h"

class TVector3;

class CbmRichPoint : public FairMCPoint 
{

 public:

  /** Default constructor **/
  CbmRichPoint();


  /** Constructor with arguments
   *@param trackID  Index of MCTrack
   *@param detID    Detector ID
   *@param pos      Coordinates at entrance to active volume [cm]
   *@param mom      Momentum of track at entrance [GeV]
   *@param tof      Time since event start [ns]
   *@param length   Track length since creation [cm]
   *@param eLoss    Energy deposit [GeV]
   **/
  CbmRichPoint(Int_t trackID, Int_t pdg, Int_t detID, TVector3 pos, TVector3 mom, 
	       Double_t tof, Double_t length, Double_t eLoss);


  /** Copy constructor **/
  CbmRichPoint(const CbmRichPoint& point) { *this = point; };


  /** Destructor **/
  virtual ~CbmRichPoint();


  /** Output to screen **/
  virtual void Print(const Option_t* opt) const;

  // Somehow fail to propagate this through MC tracks (they are not stored and
  // besides this trackID is always equal -2); so it's a hack for now;
  Int_t fPDG;

  ClassDef(CbmRichPoint,2)

};


#endif
