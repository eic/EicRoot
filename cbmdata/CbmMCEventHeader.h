/** CbmMCEventHeader.h
 *@author F.Uhlig <f.uhlig@gsi.de>
 ** Data class (level MC) containing information about the input event.
 ** 04.11.2010 New CBM class derived from FairMCEventHeader
 **/


#ifndef CBMMCEVENTHEADER_H
#define CBMMCEVENTHEADER_H 1


#include "FairMCEventHeader.h"

class CbmMCEventHeader : public FairMCEventHeader
{

 public:

  /** Default constructor **/
  CbmMCEventHeader();


  /** Constructor with all members
   **
   *@param runId    run identifier
   *@param iEvent   event identifier
   *@param x,y,z    vertex oordinates [cm]
   *@param t        event time [ns]
   *@param b        impact parameter [fm] (if relevant)
   *@param phi      event plane angle [rad]
   *@param nPrim    number of input tracks
   **/
  CbmMCEventHeader(UInt_t runId, Int_t iEvent, 
	     Double_t x, Double_t y, Double_t z, Double_t t, 
	     Double_t b, Double_t phi, Int_t nPrim);


  /** Standard constructor with run identifier **/
  CbmMCEventHeader(UInt_t runId);


  /** Destructor **/
  virtual ~CbmMCEventHeader();


  /** Accessors **/
  Double_t GetPhi()   const { return fPhi; }       // event plane angle [rad]
  

  /** Modifiers **/
  void SetPhi(Double_t phi)      { fPhi = phi; }

  /** Reset all members **/
  virtual void Reset();

  /** Register the class as data branch to the poutput */
  virtual void Register();

  ClassDef(CbmMCEventHeader,1);
 
 private:

  Double32_t fPhi;         //  Event plane angle [rad] (if relevant)


};

#endif
