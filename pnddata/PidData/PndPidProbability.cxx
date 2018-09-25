//////////////////////////////////////////////////////////////////////////
//                                                                      //
// PndPidProbability	                                                  //
//                                                                      //
// Definition of the Panda pid probabilities .	                        //
//                                                                      //
// Author: Ralf Kliemt, Dresden/Turin/Bonn, 01.09.09                    //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "PndPidProbability.h"


PndPidProbability::PndPidProbability()		
{
  fElectronPdf = 1.;
  fMuonPdf = 1.;
  fPionPdf = 1.;
  fKaonPdf = 1.;
  fProtonPdf = 1.;  
  fIndex = -1;
}

PndPidProbability::PndPidProbability(Double_t e, Double_t mu, Double_t pi, Double_t k, Double_t p, Int_t idx)
{
  fElectronPdf = e;
  fMuonPdf = mu;
  fPionPdf = pi;
  fKaonPdf = k;
  fProtonPdf = p;  
  fIndex = idx;
}

PndPidProbability::~PndPidProbability()
{
}

PndPidProbability PndPidProbability::operator*(const PndPidProbability& a)
{ 
  return PndPidProbability(
                           a.GetElectronPdf() * fElectronPdf, 
                           a.GetMuonPdf()     * fMuonPdf,
                           a.GetPionPdf()     * fPionPdf,
                           a.GetKaonPdf()     * fKaonPdf,
                           a.GetProtonPdf()   * fProtonPdf,
                           a.GetIndex()  );
}
PndPidProbability& PndPidProbability::operator*=(const PndPidProbability& a)
{ 
  fElectronPdf *= a.GetElectronPdf();
  fMuonPdf     *= a.GetMuonPdf();    
  fPionPdf     *= a.GetPionPdf();     
  fKaonPdf     *= a.GetKaonPdf();  
  fProtonPdf   *= a.GetProtonPdf();   
  return *this;
}

void PndPidProbability::Print()
{
  std::cout <<"PndPidProbability pdf's:   "
  <<"p(e)="<<fElectronPdf<<"  "
  <<"p(mu)="<<fMuonPdf<<"  "
  <<"p(pi)="<<fPionPdf<<"  "
  <<"p(K)="<<fKaonPdf<<"  "
  <<"p(P)="<<fProtonPdf<<"  "
  <<"index="<<fIndex
  <<std::endl;
}

void PndPidProbability::Reset()
{
  fElectronPdf = 1.;
  fMuonPdf = 1.;
  fPionPdf = 1.;
  fKaonPdf = 1.;
  fProtonPdf = 1.;  
  fIndex = -1;
}

void PndPidProbability::NormalizeTo(Double_t N)
{
  Double_t scalefactor = GetSumProb();
  if (scalefactor == 0) return;
  scalefactor = N / scalefactor;
  fElectronPdf*=scalefactor;
  fMuonPdf*=scalefactor;
  fPionPdf*=scalefactor;
  fKaonPdf*=scalefactor;
  fProtonPdf*=scalefactor;
  return;
}


ClassImp(PndPidProbability)

