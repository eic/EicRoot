// -------------------------------------------------------------------------
// -----                       CbmVertex source file                   -----
// -----                  Created 28/11/05  by V. Friese               -----
// -------------------------------------------------------------------------
#include "CbmVertex.h"

#include <iostream>

using std::cout;
using std::endl;


// -----   Default constructor   -------------------------------------------
CbmVertex::CbmVertex() 
  : TNamed("Vertex", "Global"),
    fX(0.),
    fY(0.),
    fZ(0.),
    fChi2(0.),
    fNDF(0),
    fNTracks(0),
    fCovMatrix()
{
  for(Int_t i=0; i<6; i++) fCovMatrix[i] = 0;
}
// -------------------------------------------------------------------------



// -----   Constructor with name and title   -------------------------------
CbmVertex::CbmVertex(const char* name, const char* title) 
  : TNamed(name, title),
    fX(0.),
    fY(0.),
    fZ(0.),
    fChi2(0.),
    fNDF(0),
    fNTracks(0),
    fCovMatrix()
{
  for(Int_t i=0; i<6; i++) fCovMatrix[i] = 0;
}
// -------------------------------------------------------------------------
  


// -----   Constructor with all parameters   -------------------------------
CbmVertex::CbmVertex(const char* name, const char* title,
		     Double_t x, Double_t y, Double_t z, Double_t chi2,
		     Int_t ndf, Int_t nTracks, 
		     const TMatrixFSym& covMat) 
  : TNamed(name, title),
    fX(x),
    fY(y),
    fZ(z),
    fChi2(chi2),
    fNDF(ndf),
    fNTracks(nTracks),
    fCovMatrix()
{
  /*
  fTitle   = title;
  fX       = x;
  fY       = y;
  fZ       = z;
  fChi2    = chi2;
  fNDF     = ndf;
  fNTracks = nTracks;
  */
  Int_t index = 0;
  for (Int_t i=0; i<3; i++) {
    for (Int_t j=i; j<3; j++) fCovMatrix[index++] = covMat[i][j];
  }
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmVertex::~CbmVertex() { }
// -------------------------------------------------------------------------



// -----   Public method Print   -------------------------------------------
void CbmVertex::Print() {
  Double_t chi2ndf;
  if (fNDF) chi2ndf = fChi2 / Double_t(fNDF);
  else chi2ndf = 0.;	       
  cout << "Vertex coord. (" << fX << "," << fY << "," << fZ << ") cm, "
       << "chi2/ndf = " << chi2ndf << ", " << fNTracks
       << " tracks used" << endl;
}
// -------------------------------------------------------------------------



// -----   Accessor to covariance matrix    --------------------------------
void CbmVertex::CovMatrix(TMatrixFSym& covMat) const {
  Int_t index = 0;
  for (int i=0; i<3; i++) {
    for (int j=i; j<3; j++) {
      covMat[i][j] = fCovMatrix[index];
      covMat[j][i] = fCovMatrix[index];
      index++;
    }
  }
}
// -------------------------------------------------------------------------



// -----   Accessor to covariance matrix elements   ------------------------
Double_t CbmVertex::GetCovariance(Int_t i, Int_t j) const {
  TMatrixFSym* mat = new TMatrixFSym(3);
  CovMatrix(*mat);
  Double_t element = (*mat)[i][j];
  delete mat;
  return element;
}
// -------------------------------------------------------------------------



// -----   Public method SetVertex   ---------------------------------------
void CbmVertex::SetVertex(Double_t x, Double_t y, Double_t z, Double_t chi2,
			  Int_t ndf, Int_t nTracks, 
			  const TMatrixFSym& covMat) {
  fX       = x;
  fY       = y;
  fZ       = z;
  fChi2    = chi2;
  fNDF     = ndf;
  fNTracks = nTracks;
  Int_t index = 0;
  for (Int_t i=0; i<3; i++) {
    for (Int_t j=i; j<3; j++) fCovMatrix[index++] = covMat[i][j];
  }
}
// -------------------------------------------------------------------------



// -----   Public method Reset   -------------------------------------------
void CbmVertex::Reset() {
  fX = fY = fZ = fChi2 = 0.;
  fNDF = fNTracks = 0;
  for(Int_t i=0; i<6; i++) fCovMatrix[i] = 0;
}  
// -------------------------------------------------------------------------



ClassImp(CbmVertex)
