//
// AYK (ayk@bnl.gov), 2013/06/13
//
//  EIC detector name service class; just for convenience ...;
//

#include "TString.h"
#include "TObject.h"

class EicGeoParData;
class EicDetName;

#ifndef _EIC_DETECTOR_
#define _EIC_DETECTOR_

enum SteppingType {qSteppingTypeUndefined = 0, qOneStepOneHit, qMergeStepsInOneHit, qSteppingTypeDefault};

class EicDetector : public TObject {
public:
  /// Default constructor
  ///
 EicDetector(const char *detector_name = 0, char *geometry_name = 0, SteppingType stType = qSteppingTypeUndefined): 
  mDetectorName(0), gptr(0), vptr(0), mAllVolumesSensitiveFlag(false),fListOfGeantSensitives(0), fStType(stType),
    fMotherVolumeName(""), fRootMaterialImportFlag(false) {
    
    if (detector_name) mDetectorName = new EicDetName(detector_name);
    if (geometry_name) SetGeometryFileName(geometry_name);
  };

  /// Main constructor
  ///
  /// @param name detector name 
  //EicDetName(const char *name);
  
  /// Destructor
  ///
  virtual ~EicDetector() {};

  void Initialize( void );
  int DeclareGeantSensitiveVolume(const char *name, SteppingType stType = qSteppingTypeDefault);
  int DeclareGeantSensitiveVolumePrefix(const char *name, SteppingType stType = qSteppingTypeDefault);
  void ConstructGeometry( void);
  ULong64_t GetNodeMultiIndex( void );
  void FinishRun( void );

  bool CheckIfSensitive(std::string name);

  EicDetName *GetDetectorName() { return mDetectorName; };

  void SetGeometryFileName(const TString &gname)       { mGeoName = gname; };
  const TString &GetGeometryFileName( void )     const { return mGeoName ;}

  void ConstructRootGeometry( void );
  void SetMotherVolume(TString volName) {fMotherVolumeName=volName;}

  void SetDefaultMatrixName(TGeoMatrix* matrix);
  void AssignMediumAtImport(TGeoVolume* v);  
  void ImportRootMaterials() { fRootMaterialImportFlag = kTRUE; };

  void ExpandNode(TGeoNode* Node);
  void AddSensitiveVolume(TGeoVolume* v);

 private:
  TString mGeoName;

  // Keep track of volumes already served in ExpandNode(); can greatly 
  // reduce start-up time for certain geometry types;
  std::set<TGeoVolume*> fGeoVolumeLut;

  SteppingType fStType;   //!

  Bool_t fRootMaterialImportFlag; 

  /*! Detector name frame; */
  EicDetName *mDetectorName; //!

  /*! Geometry mapping table; */
  EicGeoParData *gptr;    //!
  TGeoVolume *vptr;       //!

  TString fMotherVolumeName; //!

  /*! ROOT volume path upon entry; */
  TString fPathUponEntry; //!

  Bool_t mAllVolumesSensitiveFlag;
  EicNamePatternHub<SteppingType> *fListOfGeantSensitives;

  ClassDef(EicDetector,1);
};

#endif
