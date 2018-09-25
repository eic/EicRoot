/**
* \file CbmRichTestHits.h
*
* \brief This class fills some histograms for a fast check of simulated data.
*
* \author Claudia Hoehne
* \date 2006
**/

#ifndef CBM_RICH_TEST_HITS
#define CBM_RICH_TEST_HITS

#include "FairTask.h"
class FairBaseParSet;
class CbmGeoRichPar;
class TClonesArray;
class TH1D;
class TH2D;

/**
* \class CbmRichTestHits
*
* \brief This class fills some histograms for a fast check of simulated data.
*
* \author Claudia Hoehne
* \date 2006
**/
class CbmRichTestHits : public FairTask
{
public:

  /**
   * \brief Default constructor.
   */
  CbmRichTestHits();

  /**
   * \brief Destructor.
   */
  virtual ~CbmRichTestHits();

  /**
   * \brief Inherited from FairTask.
   */
  virtual void SetParContainers();

  /**
   * \brief Inherited from FairTask.
   */
  virtual InitStatus Init();

  /**
   * \brief Inherited from FairTask.
   */
  virtual void Exec(
        Option_t* option);

  /**
   * \brief Inherited from FairTask.
   */
  virtual void Finish();

private:

  TClonesArray* fRichHits; // RICH hits
  TClonesArray* fRichPoints; // RICH MC points
  TClonesArray* fMcTracks; // MC Tracks

  Int_t fNEvents;

  TH2D* fh_Det1ev; // photodetector plane for 1st event
  TH2D* fh_Det1ev_zoom; // photodetector plane for 1st event, zoom in
  TH2D* fh_Detall; // photodetector plane for all events
  TH2D* fh_Detall_zoom; // photodetector plane for all events, zoom in for fine-structure
  TH2D* fh_n_vs_p; // Number of hits versus momentum

  TH1D* fh_Nhits; // Number of hits/ring

  TH1D* fh_Nall; // Number of all rings
  TH1D* fh_Nel; // Number of electron rings
  TH1D* fh_Nelprim; // Number of electron rings with STS>=6
  TH1D* fh_Npi; // Number of pion rings
  TH1D* fh_Nk; // Number of kaon rings
  TH1D* fh_Nhad; // Number of hadrons crossing the PMT plane
  TH1D* fh_Nnoise; // Number of noise hits

  // geometry parameters
  TObjArray* fSensNodes;
  CbmGeoRichPar* fPar;
  Double_t fDetZ; // Z-coordinate of photodetector

  /**
   * \brief Copy constructor.
   */
  CbmRichTestHits(const CbmRichTestHits&);

  /**
   * \brief Assignment operator.
   */
  CbmRichTestHits& operator=(const CbmRichTestHits&);

  ClassDef(CbmRichTestHits,1)

};

#endif
