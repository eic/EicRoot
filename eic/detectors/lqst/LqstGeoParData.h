//
// RMP (rpetti@bnl.gov), 09-10-2014
//
// Low Q^2 tagging detector specific data definitions
//

#ifndef _LQSTGEOPARDATA_
#define _LQSTGEOPARDATA_


#include <EicGeoParData.h>
//#include <EndcapGeoParData.h>
#include <../calorimetry/CalorimeterGeoParData.h>
#include "TGeoVolume.h"
#include "TNamed.h"

#define _AIR_      ("air")
#define _DIAMOND_  ("diamond")

//! Class for low Q^2 tagging detector specific data definitions
/***/

class LqstCell;
//class CalorimeterGeoParData;

class LqstGeoParData: public EicGeoParData
//class LqstGeoParData: public CalorimeterGeoParData
{

  friend class LqstCell;

 public:
  
  //! constructor
  /***/
 LqstGeoParData(const char *detName = 0, int version = -1, int subVersion = 0):
  //  CalorimeterGeoParData(detName, version, subVersion),
  EicGeoParData(detName, version, subVersion),
    nGroups(1),
    nCellsX(1),
    nCellsY(1) ,
    cellFaceSize(0),
    cellFaceLength(0),
    ecalTowerLength(0.),
    beamLineOffset(0.),
    xOffset(0.),
    totalAngle(0.),
    totalLength(0.),
    // tcounter(0),
    mcell(0),
    holder(0),
    lqs(0)  {};


  //! destructor
  /***/
  virtual ~LqstGeoParData() {};

  void InitializeDetector(LqstGeoParData *det, int nlayers, int nX, int nY, double size, double width, double zoffset, double xoffset, double angle);
  //  LqstGeoParData *constructDetector();
  void constructTrackerLayers();
  void constructEcalLayers();
  void constructDetector();


  // some access functions
  const double get_numCellsX()      { return nCellsX; };
  const double get_numCellsY()      { return nCellsY; };
  const double get_totalNumCells()  { return nCellsX*nCellsY; };

  const double get_detectorLength() { return nCellsX*cellFaceSize; };
  const double get_detectorHeight() { return nCellsY*cellFaceSize; };
  const double get_detectorWidth()  { return cellFaceLength; };  // I know its confusing, but cellFaceLength is depth ( width) of the detector

  const double get_cellFaceSize()   { return cellFaceSize; };
  const double get_cellFaceLength() { return cellFaceLength; };
 
  const double get_nGroups()        { return nGroups; };
  const double get_ecalTowerLength() {return ecalTowerLength;};


  void set_numCellsX(int nX)  { nCellsX = nX; };
  void set_numCellsY(int nY)  { nCellsY = nY; };

  void set_cellFaceSize(double size)   { cellFaceSize = size; };
  void set_cellFaceLength(double length) { cellFaceLength = length; };

  void set_beamLineOffset(double offset) { beamLineOffset = offset; };
  void set_xOffset(double offset)        { xOffset = offset; };
  void set_angle(double angle)           { totalAngle = angle; };

  void set_nGroups(double nlayers)       { nGroups = nlayers; };

  void set_ecalTowerLength(double length) {ecalTowerLength = length;};

 private:
  int nCellsX;
  int nCellsY;
  int nGroups;

  double cellFaceSize;
  double cellFaceLength;

  double ecalTowerLength;

  double xOffset;
  double beamLineOffset;
  double totalAngle;

  double totalLength;

  //  int tcounter;

  LqstCell *mcell;
  
  TGeoBBox *holder;
  LqstGeoParData *lqs;

  //  CalorimeterGeoParData *calogeom;
  
  ClassDef(LqstGeoParData,1);

};


// class defining a single sensor object
class LqstCell: public TNamed
{

  friend class LqstGeoParData;

 public:
  LqstCell() {};
  
 LqstCell(const char *name):
  TNamed(name, 0),
    //    cellFaceSize(cellSize),
    //cellFaceLength(cellLength),
    posX(0.),
    posY(0.),
    posZ(0.),
    angle(0.),
    mCell(0)  {};

  virtual ~LqstCell() {};

  // some access functions
  //  const double get_cellFaceSize()     { return cellFaceSize; };
  //const double get_cellFaceLength()   { return cellFaceLength; };
  //  const char*  get_name()             { return name; }

  void set_localX(double x)     { posX = x; };
  void set_localY(double y)     { posY = y; };
  void set_localZ(double z)     { posZ = z; };
  void set_localAngle(double a) { angle = a; }; 

  TGeoVolume *createCell(LqstGeoParData *detector, TString name);
  TGeoVolume *createCell(LqstGeoParData *detector, TString name, double length);


 private:
  //  const double cellFaceSize;  // assume square sensor for now
  //const double cellFaceLength;

  double posX, posY, posZ;
  double angle;   // in degrees

  TGeoVolume *mCell;
  

  ClassDef(LqstCell,1);

};


#endif
