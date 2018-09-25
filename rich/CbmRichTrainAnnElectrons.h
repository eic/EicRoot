/**
 * \file CbmRichTrainAnnElectrons.h
 *
 * \brief Train ANN for electron identification in RICH.
 *
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2011
 **/

#ifndef CbmRichTrainAnnElectrons_H
#define CbmRichTrainAnnElectrons_H

#include "FairTask.h"
#include "TH2D.h"

//class TH1;
//class TH1D;
//class TH2D;
class TClonesArray;

#include <vector>

using std::vector;

/**
 * \class RingElectronParam
 *
 * \brief Input Parameters for ANN for electron identification in RICH.
 *
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2011
 **/
class RingElectronParam
{
public:
   Double_t fAaxis;
   Double_t fBaxis;
   Double_t fPhi;
   Double_t fRadAngle;
   Double_t fChi2;
   Double_t fRadPos;
   Double_t fNofHits;
   Double_t fDistance;
   Double_t fMomentum;
};


/**
 * \class CbmRichTrainAnnElectrons
 *
 * \brief Train ANN for electron identification in RICH.
 *
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2011
 **/
class CbmRichTrainAnnElectrons: public FairTask{

public:

   /**
    * \brief Default constructor
    */
   CbmRichTrainAnnElectrons();

   /**
    * \brief Destructor
    */
	virtual ~CbmRichTrainAnnElectrons();

   /**
    * \brief Inherited from FairTask
    */
	virtual InitStatus Init();

   /**
    * \brief Inherited from FairTask
    */
	virtual void Exec(
	      Option_t* option);

   /**
    * \brief Inherited from FairTask
    */
	virtual void FinishTask();

private:
   Int_t fEventNum;
   TClonesArray* fRichHits;
   TClonesArray* fRichRings;
   TClonesArray* fRichPoints;
   TClonesArray* fMCTracks;
   TClonesArray* fRichRingMatches;
   TClonesArray* fRichProj;
   TClonesArray* fStsTrackMatches;
   TClonesArray* fGlobalTracks;
   TClonesArray* fStsTracks;

   Int_t fMinNofHitsInRichRing;
   Double_t fQuota;
   Int_t fMaxNofTrainSamples; // maximum number of train samples for ANN

   Int_t fNofPiLikeEl;
   Int_t fNofElLikePi;
   Double_t fAnnCut;
   // ANN outputs
   // [0] = electrons; [1] = pions
   vector<TH1D*> fhAnnOutput;
   vector<TH1D*> fhCumProb;

   // Data for ANN input
   // [0] = electrons, [1] = pions
   vector<vector<RingElectronParam> > fRElIdParams;

   // difference between electrons and pions
   // [0] = is electron; [1] = is pion
   vector<TH1D*> fhAaxis; //major half axis
   vector<TH1D*> fhBaxis; //minor half axis
  // vector<TH1D*> fhAaxisCor; // major half axis after correction
  // vector<TH1D*> fhBaxisCor; //minor half axis after correction
   vector<TH1D*> fhDistTrueMatch; // distance between ring center and track projection for true matches
   vector<TH1D*> fhDistMisMatch; // distance between ring center and track projection  for wrong matches
   vector<TH1D*> fhNofHits; // number of hits in ring
   vector<TH1D*> fhChi2; // chi2 of the fit
   vector<TH1D*> fhRadPos; // radial position of a ring onto the photodetector plane
   vector<TH2D*> fhAaxisVsMom; // major half axis vs. momentum
   vector<TH2D*> fhBaxisVsMom; // minor half axis vs. momentum
   vector<TH2D*> fhPhiVsRadAng; // ellipse rotation angle vs. radial angle

   vector<TH1*> fHists; // Store pointer for all histograms

   /**
    * \brief Fill input parameters for ANN in array and histograms.
    */
   void DiffElandPi();

   /**
    * \brief Train and test ANN.
    */
   void TrainAndTestAnn();

   /**
    * \brief Draw results.
    */
   void Draw();

   /**
    * \brief Copy constructor.
    */
   CbmRichTrainAnnElectrons(const CbmRichTrainAnnElectrons&);

   /**
    * \brief Assignment operator.
    */
   CbmRichTrainAnnElectrons& operator=(const CbmRichTrainAnnElectrons&);

   ClassDef(CbmRichTrainAnnElectrons,1)
};

#endif

