/** CbmMCEpoch.h
 *@author Volker Friese v.friese@gsi.de
 *@since 13.11.2009
 *@version 1.0
 **
 ** CBM data class representing one read-out epoch (time slice) on MC level.
 ** It manages all CBM MCPoint collections within this time slice in arrays.
 ** Within each array, the points are sorted w.r.t. time.
 **/


#ifndef CBMEPOCH_H
#define CBMEPOCH_H 1


#include <FairMCPoint.h>
//#include "sts/CbmStsPoint.h"
//#include "much/CbmMuchPoint.h"
#include "CbmDetectorList.h"


#include "TClonesArray.h"
#include "TNamed.h"

#include <vector>




class CbmMCEpoch : public TNamed
{

 public:


  /**   Default constructor  **/
  CbmMCEpoch();


  /**   Standard constructor 
   *@param startTime   Begin of epoch [ns]
   *@param epochLength Duration of epoch [ns]
   **/
  CbmMCEpoch(Double_t startTime, Double_t epochLength);


  /**   Destructor   **/
  virtual ~CbmMCEpoch();


  /**   Add one MCPoint
   **   The point will be copied to the internal array using its copy
   **   constructor. The original object is kept. The point time is 
   **   recalculated relative to the epoch start time.
   *@param det  Detector system identifier
   *@param stsPoint  Pointer to MCPoint to be added
   *@param eventId   Event identifier (negative value keeps original event Id)
   *@param eventTime MC event time
   **/
  void AddPoint(DetectorId det, FairMCPoint* point, 
		Int_t eventId = -1, Double_t eventTime = 0.);


  /**   Clear data   **/
  void Clear();


  /**   Get number of points in this epoch for a given detector 
   *@param det  Detector system identifier
   **/
  Int_t GetNofPoints(DetectorId det) const;


  /** Get array of MC points for a given detector */
  TClonesArray* GetPoints(DetectorId det) { return fPoints[det]; } 
     
     
  /**   Get an MCPoint for a given detector
   **@param det    Detector system identifier
   **@param index  Index of point in array
   **   Check for index range included.
   **/
  FairMCPoint* GetPoint(DetectorId det, Int_t index);


  /**   Get epoch start time   **/
  Double_t GetStartTime() { return fStartTime; }


  /**   Check whether epoch is empty (no MC points)   **/
  Bool_t IsEmpty();


  /**   Print info   **/
  virtual void Print(Option_t* opt ="") const;


  /**   Set epoch start time   **/
  void SetStartTime(Double_t time) { fStartTime = time; }




 private:

  Double_t fStartTime;             /** Start time of epoch [ns] **/
  Double_t fEpochLength;           /** Duration of epoch [ns] **/ 

  TClonesArray* fPoints[kTutDet];  /** Array of arrays with MCPoints **/


  /**  Create MCPoint arrays
   **  Used from the constructors.
   **/
  void CreateArrays();

  CbmMCEpoch(const CbmMCEpoch&);
  CbmMCEpoch& operator=(const CbmMCEpoch&);

  ClassDef(CbmMCEpoch,1);


};

#endif


