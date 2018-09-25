//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  Generic calorimeter mapping class;
//
//  Well, there are too many calorimeters in EIC setup :-); this sort of 
//  justifies creation of several class layers which share common methods
//  and data; most of the class data variables are for information purposes;
//
//  -> at some point will have to write down exact list of methods/variables
//     which should not be modified for backwards compatibility!;
//

#include <EicGeoParData.h>

#ifndef _CALORIMETER_GEO_PAR_DATA_
#define _CALORIMETER_GEO_PAR_DATA_

class CalorimeterGeoParData: public EicGeoParData
{
 private:
  void ResetVars() {
    mType = CalorimeterGeoParData::Dummy;
    mCellFaceSizeX = mCellFaceSizeY = mCellLength = mInterCellGap = 0.0;
  };

 public:
  // Well, can imagine different calorimeter types may have certain specific
  // features which need to be distinguished later; does not cost much to describe 
  // calorimeter type in geometry;
  enum EicCalorimeterType {Dummy, Crystal, Fiber, Sandwich};

 CalorimeterGeoParData(const char *detName = 0, int version = -1, int subVersion = 0): 
  EicGeoParData(detName, version, subVersion) { ResetVars(); };

  EicCalorimeterType mType;     // calorimeter type (PWO, fiber, etc)

  // Elementary unit (BEMC crystal; fiber calorimeter tower; ...);
  Double_t mCellFaceSizeX;      // crystal(tower) front face (square) dimension
  Double_t mCellFaceSizeY;      // crystal(tower) front face (square) dimension
  Double_t mCellLength;         // crystal(tower) length
  Double_t mInterCellGap;       // either air (FEMC) or carbon fiber alveole gap between cells

  ClassDef(CalorimeterGeoParData,7);
};

#endif
