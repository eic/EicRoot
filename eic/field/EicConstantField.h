// AYK (ayk@bnl.gov), 2014/09/03
//
//  Constant field handler;
//

#include <TGeoBBox.h>
#include <TGeoTube.h>

#include <EicMagneticFieldMap.h>

#ifndef _EIC_CONSTANT_FIELD_
#define _EIC_CONSTANT_FIELD_

//
//  Assume, that 1) *full* box/tube dimensions are given, 2) units are [cm] and [kGs];
//

class EicConstantField: public EicMagneticFieldMap 
{
 public:
  EicConstantField(double bX = 0.0, double bY = 0.0, double bZ = 0.0) {
    mFieldLocal[0] = bX; mFieldLocal[1] = bY; mFieldLocal[2] = bZ; 

    memset(mFieldGlobal, 0x00, sizeof(mFieldGlobal));
  };
  ~EicConstantField() {};

  // Initialize() & GetFieldValue() can in principle be the same for box/tube (and 
  // other, if needed) constant fields, optionally restricted by a TGeoShape; 
  int Initialize();
  int GetFieldValue(const double xx[], double B[]) const;

 protected:
  // Obviously I only need to save local field values; the global ones are transient guys;
  Double_t mFieldLocal[3];                // constant field value in the object local coordinate system
  mutable Double_t mFieldGlobal[3];       //! constant field value in the world system

  ClassDef(EicConstantField,1)
};

class EicConstantBoxField: public EicConstantField 
{
 public:
  EicConstantBoxField() {};
  EicConstantBoxField(double xMin, double xMax, double yMin, double yMax, double zMin, double zMax,
		      double bX, double bY, double bZ): EicConstantField(bX, bY, bZ) {
    mShape = dynamic_cast<TGeoShape*>(TGeoBBox((xMax - xMin)/2, (yMax - yMin)/2, 
					       (zMax - zMin)/2).Clone());

    // Prefer not to use fOrigin[];
    mTransformation = new TGeoTranslation((xMax + xMin)/2, (yMax + yMin)/2, (zMax + zMin)/2);
  };
 EicConstantBoxField(double xSize, double ySize, double zSize,
		     double bX, double bY, double bZ, TGeoMatrix *transformation = 0):
  EicConstantField(bX, bY, bZ) {
    mShape = dynamic_cast<TGeoShape*>(TGeoBBox(xSize/2, ySize/2, zSize/2).Clone());

    mTransformation = transformation;
  };
  ~EicConstantBoxField() {};

  ClassDef(EicConstantBoxField,1)
};

class EicConstantTubeField: public EicConstantField 
{
 public:
  EicConstantTubeField() {};
  EicConstantTubeField(double rMin, double rMax, double length,
		       double bX, double bY, double bZ, TGeoMatrix *transformation = 0):
  EicConstantField(bX, bY, bZ) {
    mShape = dynamic_cast<TGeoShape*>(TGeoTube(rMin, rMax, length/2).Clone());

    mTransformation = transformation;
  };
  ~EicConstantTubeField() {};

  ClassDef(EicConstantTubeField,1)
};

#endif
