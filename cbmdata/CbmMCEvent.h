/** CbmMCEvent.h
 *@author V.Friese <v.friese@gsi.de>
 ** Data class (level MC) containing information about the input event.
 ** 15.05.2008 change the event time to ns (M. Al-Turany)
 ** 11.05.2009 New CBM class derived from FairMCEventHeader
 **/


#ifndef CBMMCEVENT_H
#define CBMMCEVENT_H 1


#include "TNamed.h"
#include "TVector3.h"


class CbmMCEvent : public TNamed
{

 public:

  /** Default constructor **/
  CbmMCEvent();


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
  CbmMCEvent(UInt_t runId, Int_t iEvent, 
	     Double_t x, Double_t y, Double_t z, Double_t t, 
	     Double_t b, Double_t phi, Int_t nPrim);


  /** Standard constructor with run identifier **/
  CbmMCEvent(UInt_t runId);


  /** Destructor **/
  virtual ~CbmMCEvent();


  /** Accessors **/
  UInt_t GetRunID()   const { return fRunId; }     // run identifier
  Int_t  GetEventID() const { return fEventId; }   // event identifier
  Double_t GetX()     const { return fX; }         // vertex x [cm]
  Double_t GetY()     const { return fY; }         // vertex y [cm]
  Double_t GetZ()     const { return fZ; }         // vertex z [cm]
  Double_t GetT()     const { return fT; }         // event time [ns]
  Double_t GetB()     const { return fB; }         // impact parameter [fm]
  Double_t GetPhi()   const { return fPhi; }       // event plane angle [rad]
  Int_t GetNPrim()    const { return fNPrim; }     // number of input tracks
  Bool_t IsSet()      const { return fIsSet; }     // Flag
  void GetVertex(TVector3& vertex) { vertex.SetXYZ(fX, fY, fZ); }
  

  /** Modifiers **/
  void SetEventID(Int_t eventId) { fEventId = eventId; }
  void SetTime(Double_t t)       { fT = t; }
  void SetB(Double_t b)          { fB = b; }
  void SetPhi(Double_t phi)      { fPhi = phi; }
  void SetNPrim(Int_t nPrim)     { fNPrim = nPrim; }
  void MarkSet(Bool_t isSet)     { fIsSet = isSet; }
  void SetVertex(Double_t x, Double_t y, Double_t z);
  void SetVertex(const TVector3& vertex);


  /** Reset all members **/
  void Reset();



 private:

  UInt_t     fRunId;       //  Run identifier
  UInt_t     fEventId;     //  Event identifier
  Double32_t fX;           //  Primary vertex x [cm]
  Double32_t fY;           //  Primary vertex y [cm]
  Double32_t fZ;           //  Primary vertex z [cm]
  Double32_t fT;           //  Event time [s]
  Double32_t fB;           //  Impact parameter [fm] (if relevant)
  Double32_t fPhi;         //  Event plane angle [rad] (if relevant)
  Int_t      fNPrim;       //  Number of input tracks
  Bool_t     fIsSet;       //  Flag whether variables are filled


  ClassDef(CbmMCEvent,1);

};


inline void CbmMCEvent::SetVertex(Double_t x, Double_t y, 
					Double_t z) {
  fX = x;
  fY = y;
  fZ = z;
}


inline void CbmMCEvent::SetVertex(const TVector3& vertex) {
  fX = vertex.X();
  fY = vertex.Y();
  fZ = vertex.Z();
}


#endif
