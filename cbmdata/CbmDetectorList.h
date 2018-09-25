/** @file CbmDetectorList.h
 ** @author V.Friese  <V.Friese@Gsi.De>
 ** @date 12.06.2007
 ** @brief Defines unique identifiers (enum) for all CBM detector systems
 **/



#ifndef CBMDETECTORLIST_H
#define CBMDETECTORLIST_H 1
 

/**  DetectorID enumerator  **/
enum DetectorId {kREF,      // Reference plane  
		 kMVD,      // Micro-Vertex Detector
		 kSTS,      // Silicon Tracking System
		 kRICH,     // Ring-Imaging Cherenkov Detector
		 kMUCH,     // Muon detetcion system
		 kTRD,      // Transition Radiation Detector
		 kTOF,      // Time-of-flight Detector
		 kECAL,     // EM-Calorimeter
		 kPSD,      // Projectile spectator detector
		 kSTT,      // Straw Tube Tracker (obsolete)
		 kTutDet,   // Dummy for tutorials
		 kNOFDETS}; // Number of elements (e.g. for loops)


/**   Data type enumerator  **/
enum DataType {kUnknown, 
	       kMCTrack,
	       kStsPoint, 
	       kStsDigi, 
	       kStsCluster, 
	       kStsHit};



#include "TObject.h"



/** @class CbmDetectorList
 ** @author V.Friese  <V.Friese@Gsi.De>
 ** @date 29.04.2010
 **
 ** @brief Provides some utility functions for DetectorId
 **/
class CbmDetectorList : public TObject
{

 public:

  /**   Constructor   **/
  CbmDetectorList();


  /**   Destructor  **/
  virtual ~CbmDetectorList() { }


  /**   Get system name
   *@param det   System identifier (type DetectorId)
   *@param name  (return) System name (lower case) 
   **/
  static void GetSystemName(DetectorId det, TString& name);
  static void GetSystemName(Int_t det, TString& name);


  /** Get system name in capitals
   ** @param det   System identifier (type DetectorId)
   ** @param name  (return) System name (lower case) 
   **/
  static void GetSystemNameCaps(DetectorId det, TString& name);
  static void GetSystemNameCaps(Int_t det, TString& name);



  ClassDef(CbmDetectorList, 1);

};


#endif
