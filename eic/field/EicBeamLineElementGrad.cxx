// RMP (rpetti@bnl.gov), 2016/03/02
//
//  EIC IR Beam line element magnetic field gradient handler;
//

//
// CHECK: XY-orientation (well, and Z-sign as well); A[][] -> transpose or not?;
//

#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <iostream>
#include <fstream>
#ifdef __APPLE__
#include <unistd.h>
#include <libgen.h>
#endif

#include <TGeoBBox.h>
#include <TGeoCone.h>
#include <TGeoVolume.h>
#include <TMath.h>
#include <TVector3.h>

#include <EicLibrary.h>
#include <EicBeamLineElementGrad.h>

// =======================================================================================

//
// FIXME: check against the latest EicHtcTask codes and modify accordingly;
//

int EicBeamLineElementGrad::Initialize() 
{
  //std::cout << "In EicBeamLineElementGrad::Initialize()" << std::endl;

  
  mAngle = mAngle*1.e-3*TMath::RadToDeg();
  TGeoRotation *rw = new TGeoRotation();
  rw->RotateY(mAngle);
  // Convert [m] -> [cm];
  mTransformation = new TGeoCombiTrans(100.0 * mCenterX, 100.0 * mCenterY, 100.0 * mCenterZ, rw);
 

  //return EicMagneticFieldGrad::Initialize();
  return EicMagneticFieldMap::Initialize();
} // EicBeamLineElementGrad::Initialize()

const char *EicBeamLineElementGrad::BasenameWrapper(const char *fname) const {
  // basename() can screw up this buffer, fine;
  char buffer[1024];
  snprintf(buffer, 1024-1, "%s", fname);

  return basename(buffer);
} // EicBeamLineElementGrad::BasenameWrapper()

void EicBeamLineElementGrad::SetFieldScale(const double fieldScaler)
{
  mScale = fieldScaler;
}


// ---------------------------------------------------------------------------------------

int EicBeamLineElementGrad::GetFieldValue(const double xx[], double B[]) const 
{
  // FIXME: do thia lambda stuff better later (Fermi shape, bore-dependent, etc);
  double xl[3], lambda = 10.0;

  // NB: it looks like this->Contains() call will also have to perform mTransformation->MasterToLocal()
  // before doing anything else; so no gain to call it first; THINK: the only thing I can do to speed 
  // processing up is to call mShape->Contains() after mTransformation->MasterToLocal() here; hmm?;
  mTransformation->MasterToLocal(xx, xl);

  {
    TVector3 xlv(xl[0], xl[1], xl[2]), BLV;

    // Make sure we are in the region of the aperture where the field exists;
    // if not, make the field 0.
    double zl = xl[2], r = sqrt(xl[0]*xl[0] + xl[1]*xl[1]), dzMax = 100*mLength/2. + lambda;//)*100.0;
    // FIXME: consider conical bore case, please;
    double rMax = mBoreZout*100.0;

    //if(xl[2] < (mLength/2.)*100. && xl[2] > (-mLength/2.)*100. && r < mBoreZout*100.) {
    if(fabs(zl) < dzMax && r < rMax) {
      double cff = 1.0;
      // Mimic triangular fall off in the [-lambda .. lambda] range around mLength/2;
      if (fabs(zl) > 100.*mLength/2. - lambda) cff = (fabs(dzMax) - fabs(zl))/(2*lambda);
      //printf("%f\n", cff);

      if(mGradient == 0.) { // indicates a dipole
	BLV.SetX(0.);
	BLV.SetY(cff * mB * 10.0 * mScale);   // 10.0: convert from T to kG
	BLV.SetZ(0.);
      }	else {
	BLV.SetX(cff * xl[1]*mGradient*10./100.*mScale);   // multiply by 10 for T->kG conversion
	BLV.SetY(cff * xl[0]*mGradient*10./100.*mScale);   // divide by 100 to convert cm -> m
	BLV.SetZ(0.);
      } //if
    } else {
      for(unsigned iq=0; iq<3; iq++)
	B[iq] = 0.0;

      return -1;
    } //if

    {
      double BL[3] = {BLV[0], BLV[1], BLV[2]};
      mTransformation->LocalToMasterVect(BL, B);
    }
  }

  return 0;
} // EicBeamLineElementGrad::GetFieldValue()

// ---------------------------------------------------------------------------------------

#include <EicMediaHub.h>

// Make it configurable parameter later; for now just add 100mm to both width and height;
#define _EXTRA_BORE_WIDTH_ (100.0)

#define _IRON_   ("iron")
#define _VACUUM_ ("thin-air")//("vacuum")
//#define _VACUUM_ ("vacuum")

#include <TGeoManager.h>
#include <TGeoCompositeShape.h>

int EicBeamLineElementGrad::ConstructGeometry() 
{
  // FIXME: obviously need an Instance() member there;
  EicMediaHub *mediaHub = new EicMediaHub();

  mediaHub->Init();

  // Ok, basically I need to create something matching yoke volume and put it in a proper 
  // place in the geometry tree;
  char yokeIronName[128], yokeVacuumName[128];
  
  //snprintf(yokeIronName,   128-1, "%s-Yoke",   GetDetectorName().Data());
  //snprintf(yokeVacuumName, 128-1, "%s-Vacuum", GetDetectorName().Data());
  snprintf(yokeIronName,   128-1, "%sYoke",   GetDetectorName().Data());
  snprintf(yokeVacuumName, 128-1, "%sVacuum", GetDetectorName().Data());
  
  //double origin[3] = {0.0, 0.0, 100.0 * mLength/2};
  double origin[3] = {0.0, 0.0, 0.0};
  TGeoShape *yoke;
  double xyHalfSize = mDiaOut ? 100.0 * mDiaOut/2 : 100.0 * mBoreZout + 0.1 * _EXTRA_BORE_WIDTH_; 
  if (mB) 
    yoke = new TGeoBBox(yokeIronName, 
			xyHalfSize, xyHalfSize,
			//100.0 * mBoreZout + 0.1 * _EXTRA_BORE_WIDTH_,
			//100.0 * mBoreZout + 0.1 * _EXTRA_BORE_WIDTH_,
			100.0 * mLength/2);//, origin);
  else
    yoke = new TGeoTube(yokeIronName, 
			0.0, 
			xyHalfSize,
			//100.0 * mBoreZout + 0.1 * _EXTRA_BORE_WIDTH_,
			100.0 * mLength/2);
    
#if 1//_LATER_
  assert(mBoreZin);
  if (mBoreZin) {
    TGeoShape *vacuum;
    if (mBoreZin == mBoreZout)
      // Use simplier cylindrical shape in this case;
      vacuum = new TGeoTube(yokeVacuumName, 
			    0.0,
			    100.0 * mBoreZin,
			    100.0 * mLength/2 + 0.1);
    else
      // Otherwise define a conical bore;
      vacuum = new TGeoCone(yokeVacuumName, 
			    100.0 * mLength/2 + 0.1,
			    0.0,
			    100.0 * mBoreZin,
			    0.0,
			    100.0 * mBoreZout);
    char cmd[1024], yokeCompName[128];
    snprintf(cmd, 1024-1, "%s-%s", yokeIronName, yokeVacuumName);
    snprintf(yokeCompName, 128-1, "%sComp", GetDetectorName().Data());

    TGeoCompositeShape *comp = new TGeoCompositeShape(yokeCompName/*GetDetectorName().Data()*/, cmd);
    mYoke = new TGeoVolume(yokeCompName/*GetDetectorName().Data()*/, comp, mediaHub->GetMedium(_IRON_));
    
    mYoke->SetFillColor(GetYokeColor());
    // FIXME: this has no effect -> check!;
    mYoke->SetLineColor(GetYokeColor());
    mYoke->RegisterYourself();
  } //if
#else
  mYoke = new TGeoVolume(yokeIronName, yoke, mediaHub->GetMedium(_IRON_));
  mYoke->SetFillColor(GetYokeColor());
  // FIXME: this has no effect -> check!;
  mYoke->SetLineColor(GetYokeColor());
  mYoke->RegisterYourself();

  if (mBoreZin) {
    TGeoShape *vacuum;
    if (mBoreZin == mBoreZout)
      // Use simplier cylindrical shape in this case;
      vacuum = new TGeoTube(yokeVacuumName, 
			    0.0,
			    100.0 * mBoreZin,
			    100.0 * mLength/2);
    else
      // Otherwise define a conical bore;
      vacuum = new TGeoCone(yokeVacuumName, 
			    100.0 * mLength/2,
			    0.0,
			    100.0 * mBoreZin,
			    0.0,
			    100.0 * mBoreZout);
    
    TGeoVolume *vvacuum = new TGeoVolume(yokeVacuumName, vacuum, mediaHub->GetMedium(_VACUUM_));

    vvacuum->RegisterYourself();

    mYoke->AddNode(vvacuum, 0, new TGeoTranslation(origin[0], origin[1], origin[2]));  
  } //if
#endif  

  gGeoManager->GetTopVolume()->AddNode(mYoke, 0, mTransformation);
  
  return 0;
  
} // EicBeamLineElementGrad::ConstructGeometry() 

// =======================================================================================

ClassImp(EicBeamLineElementGrad)
