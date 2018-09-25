#ifndef PNDPIDPROBABILITY_H
#define PNDPIDPROBABILITY_H
//////////////////////////////////////////////////////////////////////////
//                                                                      //
// PndPidProbability	                                                  //
//                                                                      //
// Definition of the Panda pid probabilities .	                        //
//                                                                      //
// Author: Ralf Kliemt, Dresden/Turin/Bonn, 01.09.09                    //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <iostream>
#include "TObject.h"

class PndPidProbability : public TObject 
{

 public:

  PndPidProbability();
  PndPidProbability(Double_t e, Double_t mu, Double_t pi, Double_t k, Double_t p, Int_t idx = -1);
  ~PndPidProbability();

  Double_t		GetElectronPdf() const { return fElectronPdf; }
  Double_t		GetMuonPdf()     const { return fMuonPdf; }
  Double_t		GetPionPdf()     const { return fPionPdf; } 
  Double_t		GetKaonPdf()     const { return fKaonPdf; }
  Double_t		GetProtonPdf()   const { return fProtonPdf; }
  Int_t                 GetIndex()       const { return fIndex;}

  Double_t   GetElectronPidProb(PndPidProbability* flux = NULL) const { if(flux==NULL) flux = new PndPidProbability(0.2,0.2,0.2,0.2,0.2); return fElectronPdf * flux->GetElectronPdf() / GetSumProb(flux); }
  Double_t   GetMuonPidProb    (PndPidProbability* flux = NULL) const { if(flux==NULL) flux = new PndPidProbability(0.2,0.2,0.2,0.2,0.2); return fMuonPdf     * flux->GetMuonPdf() / GetSumProb(flux); }
  Double_t   GetPionPidProb    (PndPidProbability* flux = NULL) const { if(flux==NULL) flux = new PndPidProbability(0.2,0.2,0.2,0.2,0.2); return fPionPdf     * flux->GetPionPdf() / GetSumProb(flux); } 
  Double_t   GetKaonPidProb    (PndPidProbability* flux = NULL) const { if(flux==NULL) flux = new PndPidProbability(0.2,0.2,0.2,0.2,0.2); return fKaonPdf     * flux->GetKaonPdf() / GetSumProb(flux); }
  Double_t   GetProtonPidProb  (PndPidProbability* flux = NULL) const { if(flux==NULL) flux = new PndPidProbability(0.2,0.2,0.2,0.2,0.2); return fProtonPdf   * flux->GetProtonPdf() / GetSumProb(flux); }

  Double_t   GetSumProb        (PndPidProbability* flux = NULL) const 
  { 
    if(flux==NULL) flux = new PndPidProbability(0.2,0.2,0.2,0.2,0.2);
    return 
      fElectronPdf * flux->GetElectronPdf() + 
      fMuonPdf     * flux->GetMuonPdf()     +
      fPionPdf     * flux->GetPionPdf()     +
      fKaonPdf     * flux->GetKaonPdf()     +
      fProtonPdf   * flux->GetProtonPdf(); 
  }
  
  void NormalizeTo(Double_t N=1.);
  
  void	SetElectronPdf(Double_t val) { fElectronPdf= (Double_t) val; }
  void	SetMuonPdf(Double_t val)     { fMuonPdf=     (Double_t) val; }
  void	SetPionPdf(Double_t val)     { fPionPdf=     (Double_t) val; } 
  void	SetKaonPdf(Double_t val)     { fKaonPdf=     (Double_t) val; }
  void	SetProtonPdf(Double_t val)   { fProtonPdf=   (Double_t) val; }
  void  SetIndex(Int_t idx)          { fIndex = idx; }

  PndPidProbability& operator*=(const PndPidProbability& a);
  PndPidProbability operator*(const PndPidProbability& a);
  
  void Print();
  void Reset();
 protected:
  
  Double_t		fElectronPdf; // e  Probability density function
  Double_t		fMuonPdf;     // mu Probability density function
  Double_t		fPionPdf;     // pi Probability density function
  Double_t		fKaonPdf;     // k  Probability density function
  Double_t		fProtonPdf;   // p  Probability density function
  Int_t       fIndex;       // Candidate Index
  ClassDef(PndPidProbability,3) // 
    
    };



#endif                                           


