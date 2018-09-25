/**
* \file CbmRichReconstruction.h
*
* \brief Main class for running event reconstruction in the RICH detector.
*
* \author Semen Lebedev
* \date 2012
**/

#ifndef CBM_RICH_RECONSTRUCTION
#define CBM_RICH_RECONSTRUCTION

#include "FairTask.h"

#include <string>

class TClonesArray;
class CbmRichRingFinder;
class CbmRichRingFitterBase;
class CbmRichTrackExtrapolationBase;
class CbmRichProjectionProducerBase;
class CbmRichRingTrackAssignBase;

using std::string;

/**
* \class CbmRichReconstruction
*
* \brief Main class for running event reconstruction in the RICH detector.
*
* \author Semen Lebedev
* \date 2012
**/
class CbmRichReconstruction : public FairTask
{
public:
   /**
    * \brief Default constructor.
    */
   CbmRichReconstruction();

   /**
    * \brief Destructor.
    */
   virtual ~CbmRichReconstruction();

   /**
    * \brief Inherited from FairTask.
    */
   void SetParContainers();

   /**
    * \brief Inherited from FairTask.
    */
   virtual InitStatus Init();

   /**
    * \brief Inherited from FairTask.
    */
   virtual void Exec(
         Option_t* opt);

   /**
    * \brief Inherited from FairTask.
    */
   virtual void Finish();

   void SetRunExtrapolation(bool b){fRunExtrapolation = b;}
   void SetRunProjection(bool b){fRunProjection = b;}
   void SetRunFinder(bool b){fRunFinder = b;}
   void SetRunFitter(bool b){fRunFitter = b;}
   void SetRunTrackAssign(bool b){fRunTrackAssign = b;}

   void SetExtrapolationName(const string& n){fExtrapolationName = n;}
   void SetProjectionName(const string& n){fProjectionName = n;}
   void SetFinderName(const string& n){fFinderName = n;}
   void SetFitterName(const string& n){fFitterName = n;}
   void SetTrackAssignName(const string& n){fTrackAssignName = n;}

   /**
    * \brief Set Z coordinate where STS tracks will be extrapolated.
    * \param[in] z Z coordinate.
    */
   void SetZTrackExtrapolation(Double_t z){fZTrackExtrapolation = z;}

   /**
    * \brief Set minimum number of STS hits.
    * \param[in] minNofStsHits minimum number of STS hits.
    */
   void SetMinNofStsHits(Int_t minNofStsHits){fMinNofStsHits = minNofStsHits;}

private:
   TClonesArray* fRichHits;
   TClonesArray* fRichRings;
   TClonesArray* fRichProjections;
   TClonesArray* fRichTrackParamZ;
   TClonesArray* fGlobalTracks;

   CbmRichRingFinder* fRingFinder; // pointer to ring finder algorithm
   CbmRichRingFitterBase* fRingFitter; // pointer to ring fitting algorithm
   CbmRichTrackExtrapolationBase* fTrackExtrapolation; // pointer to track extrapolation algorithm
   CbmRichProjectionProducerBase* fProjectionProducer; // pointer to projection producer
   CbmRichRingTrackAssignBase* fRingTrackAssign; // pointer to track assignment algorithm

   // What do you wan to run.
   bool fRunExtrapolation;
   bool fRunProjection;
   bool fRunFinder;
   bool fRunFitter;
   bool fRunTrackAssign;

   // Algorithm names for each step of reconstruction.
   string fExtrapolationName; // name of extrapolation algorithm
   string fProjectionName; // name of track projection algorithm
   string fFinderName; // name of ring finder algorithm
   string fFitterName; // name of ring fitter algorithm
   string fTrackAssignName; // name of track-ring matching algorithm

   Double_t fZTrackExtrapolation; // Z coordinate to which one wants to extrapolate STS tracks
   Int_t fMinNofStsHits; // minimum number of Sts hits for extrapolation to RICH detector

   /**
    * \brief
    */
   void InitExtrapolation();

   /**
    * \brief
    */
   void InitProjection();

   /**
    * \brief
    */
   void InitFinder();

   /**
    * \brief
    */
   void InitFitter();

   /**
    * \brief
    */
   void InitTrackAssign();

   /**
    * \brief
    */
   void RunExtrapolation();

   /**
    * \brief
    */
   void RunProjection();

   /**
    * \brief
    */
   void RunFinder();

   /**
    * \brief
    */
   void RunFitter();

   /**
    * \brief
    */
   void RunTrackAssign();

   /**
    * \brief Copy constructor.
    */
   CbmRichReconstruction(const CbmRichReconstruction&);

   /**
    * \brief Assignment operator.
    */
   CbmRichReconstruction& operator=(const CbmRichReconstruction&);

   ClassDef(CbmRichReconstruction,1);
};

#endif
