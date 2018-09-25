//
// AYK (ayk@bnl.gov), 2014/03/11
//
//  EicRoot media interface to TGeo, related FairRoot stuff & Co;
//

#include <map>

#include <TObject.h>
#include <TString.h>

#include <TGeoMedium.h>

#include <FairGeoLoader.h>
#include <FairGeoInterface.h>
#include <FairGeoMedia.h>
#include <FairGeoBuilder.h>

#if _LATER_
#include <EicStlKey.h>
#endif

#ifndef _EIC_MEDIA_HUB_
#define _EIC_MEDIA_HUB_

#if _LATER_
typedef std::map<const EicStlKey*, const TGeoMedium*, bool(*)(const EicStlKey*, const EicStlKey*)> mEntry;
#endif

class EicMediaHub: public TObject {
  // Do a better access later;
  friend class EicStlFile;

 public:
  EicMediaHub(char *mediaName);
 EicMediaHub(): geoLoad(0), geoFace(0), Media(0), geobuild(0), fSingleMedium(0)
#if _LATER_
    , fMediaMap(0) 
#endif
    {};
  ~EicMediaHub() {};

  /// \brief Media creation/access method
  ///
  /// \note Need to unify with EicGeoParDataHelper code at some point 
  TGeoMedium *GetMedium(const char *medium);

  /*! A single medium pointer to be used (if not 0) for all volumes of this STL file */
  TGeoMedium *fSingleMedium; //!

  /*! Either a single medium or media transcript file */
  TString fMediaName; //!

  void Init();

  /// Media map import routine
  ///
  /// \note 3-d parameter to EicCadFile(const char *, char *, char *) constructor can 
  /// be name of media remapping file rather than a single media name; this routine is 
  /// supposed to read in and parse this file;
  int importMediaMapFile(TString &mediaMapFileName);

 private:
  /*! Duplicate of the FairRoot geo loader singleton pointer */ 
  FairGeoLoader* geoLoad; //!
  /*! FairRoot geo interface pointer */
  FairGeoInterface *geoFace; //!
  //*! FairRoot geo interface media pointer */
  FairGeoMedia *Media; //!
  /*! FairRoot geo loader media builder pointer */
  FairGeoBuilder *geobuild; //!

#if _LATER_
  mEntry *fMediaMap;
#endif

  ClassDef(EicMediaHub,2) 
};

#endif
