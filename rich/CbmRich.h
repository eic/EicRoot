/**
* \file CbmRich.h
*
* \brief Defines the active detector RICH. Constructs the geometry and creates MCPoints.
*
* \author Volker Friese
* \date 2004
**/

#ifndef CBM_RICH
#define CBM_RICH


#include "FairDetector.h"

#include "TVector3.h"

class TClonesArray;
class CbmRichRefPlanePoint;
class CbmRichPoint;
class CbmRichMirrorPoint;
class FairVolume; 
class TGeoMatrix;
class TGeoNode;
/**
* \class CbmRich
*
* \brief Defines the active detector RICH. Constructs the geometry and creates MCPoints.
*
* \author Volker Friese
* \date 2004
**/
class CbmRich : public FairDetector 
{

public:

   /**
   * \brief Default constructor.
   */
   CbmRich();

   /**
    * \brief Constructor for the GDML geometry.
    * \param[in] name Detector name.
    * \param[in] active Sensitivity flag.
    * \param[in] px Position X.
    * \param[in] py Position Y.
    * \param[in] pz Position Z from target to the center of the RICH detector.
    * \param[in] rx Rotation around X.
    * \param[in] ry Rotation around Y.
    * \param[in] rz Rotation around Z.
    */
   CbmRich(
         const char* name,
         Bool_t active,
         Double_t px=0.,
         Double_t py=0.,
         Double_t pz=0.,
         Double_t rx=0.,
         Double_t ry=0.,
         Double_t rz=0.);


   /**
    * \brief Destructor.
    */
   virtual ~CbmRich();


   /**
    * \brief Initialize detector. Stores volume IDs for RICH detector and mirror.
    */
   virtual void Initialize();


   /**
    * \brief Defines the action to be taken when a step is inside the
    * active volume. Creates CbmRichPoints and CbmRichMirrorPoints and adds
    * them to the collections.
    * \param[in] vol Pointer to the active volume.
    */
   virtual Bool_t ProcessHits(
         FairVolume* vol = 0);


   /**
    * \brief If verbosity level is set, print hit collection at the
    * end of the event and resets it afterwards.
    */
   virtual void EndOfEvent();


   /**
    * \brief Registers the hit collection in the ROOT manager.
    */
   virtual void Register();


   /**
    * \brief Return hit collection.
    */
   virtual TClonesArray* GetCollection(
         Int_t iColl) const;


   /**
    * \brief Screen output of hit collection.
    */
   virtual void Print() const;


   /**
    * \brief Clears the hit collection.
    */
   virtual void Reset();


   /**
    * \brief Copies the hit collection with a given track index offset.
    * \param[in] cl1 Origin array.
    * \param[out] cl2 Target array.
    * \param[in] offset Index offset.
    */
   virtual void CopyClones(
         TClonesArray* cl1,
         TClonesArray* cl2,
         Int_t offset);


   /**
    * \brief Construct geometry. Currently ROOT and ASCII formats are supported.
    * The concrete method for geometry construction is called according to geometry file.
    */
   virtual void ConstructGeometry();

   /**
    * \brief Construct geometry from ASCII file. Supported structure for the mirrors are added on a fly.
    */
   void ConstructAsciiGeometry();


   /**
    * \brief Construct geometry from GDML file.
    * \param[in] geoMatrix Position and rotation of the RICH detector.
    */
   void ConstructGdmlGeometry(TGeoMatrix* geoMatrix);

   /**
    * \brief Assign materials by taking description from medoa.geo and not from GDML for a certain node.
    * \param[in] node GeoNode.
    */
   void ExpandNodeForGdml(TGeoNode* node);


   /**
    * \brief Put some optical properties.
    */
   void ConstructOpGeometry();


   /** Check whether a volume is sensitive.
    ** The decision is based on the volume name. Only used in case
    ** of ROOT geometry.
    ** @since 11.06.2012
    ** @param(name)  Volume name
    ** @value        kTRUE if volume is sensitive, else kFALSE
    **/
   virtual Bool_t CheckIfSensitive(std::string name);

   void SetScale(double scale) { mScale = scale; };

private:
   Int_t fPosIndex;

   TClonesArray* fRichPoints; // MC points onto the photodetector plane
   TClonesArray* fRichRefPlanePoints; // points on the reference plane
   TClonesArray* fRichMirrorPoints; // mirror points

   // GDML geometry
   static std::map<TString, Int_t> fFixedMats; // materials for the GDML geometry
   static Bool_t fIsFirstGDML;
   TGeoRotation* fRotation; // Rotation matrix of the RICH detector
   TGeoCombiTrans* fPositionRotation;  // Full combined matrix for position and rotation of the RICH detector
   Double_t mScale;

   TGeoVolume* GetGasVolume(TGeoVolume* gdmlTop);

   /**
    * \brief Adds a RichPoint to the TClonesArray.
    */
   CbmRichPoint* AddHit(
         Int_t trackID,
	 Int_t pdg,
         Int_t detID,
         TVector3 pos,
         TVector3 mom,
         Double_t time,
         Double_t length,
         Double_t eLoss);

   /**
    * \brief Adds a RichRefPlanePoint to the TClonesArray.
    */
   CbmRichPoint* AddRefPlaneHit(
         Int_t trackID,
         Int_t detID,
         TVector3 pos,
         TVector3 mom,
         Double_t time,
         Double_t length,
         Double_t eLoss);

   /**
    * \brief Adds a RichMirrorPoint to the TClonesArray.
    */
   CbmRichPoint* AddMirrorHit(
            Int_t trackID,
            Int_t detID,
            TVector3 pos,
            TVector3 mom,
            Double_t time,
            Double_t length,
            Double_t eLoss);

   /**
    * \brief Copy constructor.
    */
   CbmRich(const CbmRich&);

   /**
    * \brief Assignment operator.
    */
   CbmRich& operator=(const CbmRich&);

   ClassDef(CbmRich,3)
};


#endif
