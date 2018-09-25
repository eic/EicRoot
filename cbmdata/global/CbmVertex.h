// -------------------------------------------------------------------------
// -----                      CbmVertex header file                    -----
// -----                  Created 28/11/05  by V. Friese               -----
// -------------------------------------------------------------------------


/** CbmVertex.h
 *@author V.Friese <v.friese@gsi.de>
 **
 ** Data class for a vertex in CBM.
 ** Data level: RECO
 **/


#ifndef CBMVERTEX_H
#define CBMVERTEX_H 1


#include "TMatrixFSym.h"
#include "TNamed.h"
#include "TVector3.h"


class CbmVertex : public TNamed
{

 public:

  /** Default constructor  **/
  CbmVertex();


  /** Constructor with name and title **/
  CbmVertex(const char* name, const char* title);


  /** Constructor with all member variables 
   *@param name      Name of object
   *@param title     Title of object
   *@param x         x coordinate [cm]
   *@param y         y coordinate [cm]
   *@param z         z coordinate [cm]
   *@param chi2      chi square of vertex fit
   *@param ndf       Number of degrees of freedom of vertex fit
   *@param nTracks   Number of tracks used for vertex fit
   *@param covMat    Covariance Matrix (symmetric, 3x3)
   **/
  CbmVertex(const char* name, const char* title, 
	    Double_t x, Double_t y, Double_t z, Double_t chi2,
	    Int_t ndf, Int_t nTracks, const TMatrixFSym& covMat);


  /** Destructor **/
  virtual ~CbmVertex();


  /** Ouput to screen **/
  void Print();


  /** Accessors **/
  Double_t GetX()    const { return fX;       };  // x position [cm]
  Double_t GetY()    const { return fY;       };  // y position [cm]
  Double_t GetZ()    const { return fZ;       };  // z posiiton [cm]
  Double_t GetChi2() const { return fChi2;    };  // chi2
  Int_t GetNDF()     const { return fNDF;     };  // nof degrees of freedom
  Int_t GetNTracks() const { return fNTracks; };  // nof tracks used
  void Position(TVector3& pos) const { pos.SetXYZ(fX,fY,fZ); };
  void CovMatrix(TMatrixFSym& covMat) const;
  Double_t GetCovariance(Int_t i, Int_t j) const;


  /** Set the member variables
   *@param x         x coordinate [cm]
   *@param y         y coordinate [cm]
   *@param z         z coordinate [cm]
   *@param chi2      chi square of vertex fit
   *@param ndf       Number of degrees of freedom of vertex fit
   *@param nTracks   Number of tracks used for vertex fit
   *@param covMat    Covariance Matrix (symmetric, 3x3)
   **/
  void SetVertex(Double_t x, Double_t y, Double_t z, Double_t chi2,
		 Int_t ndf, Int_t nTracks, const TMatrixFSym& covMat);


  /** Reset the member variables **/
  void Reset();
		    

 private:

  /** Position coordinates  [cm] **/
  Double32_t fX, fY, fZ;

  /** Chi2 of vertex fit **/
  Double32_t fChi2;

  /** Number of degrees of freedom of vertex fit **/
  Int_t fNDF;

  /** Number of tracks used for the vertex fit **/
  Int_t fNTracks;

  /** Covariance matrix for x, y, and z stored in an array. The
   ** sequence is a[0,0], a[0,1], a[0,2], a[1,1], a[1,2], a[2,2]
   **/
  Double32_t fCovMatrix[6];


  ClassDef(CbmVertex,1);

};


#endif
