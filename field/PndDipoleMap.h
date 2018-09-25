
/** PndDipoleMap.h
 ** @author M.Al-Turany <m.al-turany@gsi.de>
 ** @version1.0
 **
 ** Magnetic field map on a 3-D grid with symmetries w.r.t. the x and y
 ** coordinate axes. The map is only stored in the quadrant x>0, y>0.
 ** The symmetries are:
 ** - Bx is antisymmetric in x and y
 ** - By is symmetric in x and y
 ** - Bz is symmetric in x and antisymmetric y
 **
 ** Field values are hold and returned in kG.
 **/


#ifndef PNDDIPOLEMAP_H
#define PNDDIPOLEMAP_H 1


#include "PndFieldMap.h"


class PndDipolePar;
class PndMapPar;

class PndDipoleMap : public PndFieldMap {


public:


  /** Default constructor **/
  PndDipoleMap();


  /** Standard constructor
   ** @param name       Name of field map
   ** @param fileType   R = ROOT file, A = ASCII
   **/
  PndDipoleMap(const char* mapName, const char* fileType = "R", Double_t BeamMom=15.0);


  /** Constructor from PndFieldPar **/
  PndDipoleMap(PndMapPar* fieldPar);


  /** Destructor **/
  virtual ~PndDipoleMap();


   /** Determine whether a point is inside the field map
   ** @param x,y,z              Point coordinates (global) [cm]
   ** @param ix,iy,iz (return)  Grid cell
   ** @param dx,dy,dz (return)  Distance from grid point [cm] if inside
   ** @value kTRUE if inside map, else kFALSE
   **/
  virtual Bool_t IsInside(Double_t x, Double_t y, Double_t z,
			  Int_t& ix, Int_t& iy, Int_t& iz,
			  Double_t& dx, Double_t& dy, Double_t& dz);

  static Int_t fNumberOfRegions; //!
  Int_t fRegionNo;	

  Int_t GetRegionNo(){return fRegionNo;}

  void GetBxyz(const Double_t point[3], Double_t* bField);

  void FillParContainer();	
	
	
 protected:

  // Hemispheres of a point (for temporary use)
  Double_t fHemiX, fHemiY;   //!
  Double_t fBeamMom;        //!

  ClassDef(PndDipoleMap,3)

};


#endif
