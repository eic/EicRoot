/**
* \file CbmRichTestSim.h
*
* \brief This class fills some histograms for a fast check of simulated data.
*
* \author Claudia Hoehne
* \date 2006
**/

#ifndef CBM_RICH_TEST_SIM
#define CBM_RICH_TEST_SIM

#include "FairTask.h"

class CbmGeoRichPar;
class FairBaseParSet;
class TClonesArray;
class TH1D;
class TH2D;

/**
* \class CbmRichTestSim
*
* \brief This class fills some histograms for a fast check of simulated data.
*
* \author Claudia Hoehne
* \date 2006
**/
class CbmRichTestSim : public FairTask
{

public:

   /**
    * \brief Default constructor.
    */
   CbmRichTestSim();

   /**
    * \brief Destructor.
    */
   virtual ~CbmRichTestSim();

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
    * \brief Finish task.
    */
   virtual void Finish();

private:

   TClonesArray* fMCRichPointArray; // RICH MC points
   TClonesArray* fMCTrackArray;// MC Tracks

   Int_t fNEvents;

   TH2D* fh_Det1ev; // photodetector plane for 1st event
   TH2D* fh_Det1ev_zoom; // photodetector plane for 1st event, zoom in
   TH2D* fh_Detall; // photodetector plane for all events
   TH2D* fh_n_vs_p; // Number of points versus momentum
   TH2D* fh_v_el; // (y,z) position of production vertex for electrons

   TH1D* fh_Nall; // Number of all rings
   TH1D* fh_Nel; // Number of electron rings
   TH1D* fh_Nelprim; // Number of electron rings with STS>=6
   TH1D* fh_Npi; // Number of pion rings
   TH1D* fh_Nk; // Number of kaon rings
   TH1D* fh_Nhad; // Number of all hadrons crossing the PMT plane

   // RICH geometry parameters
   TObjArray* fSensNodes;
   CbmGeoRichPar* fPar;
   Double_t fDetZ; // Z-coordinate of photodetector

   /**
   * \brief Copy constructor.
   */
   CbmRichTestSim(const CbmRichTestSim&);

   /**
   * \brief Assignment operator.
   */
   CbmRichTestSim& operator=(const CbmRichTestSim&);

   ClassDef(CbmRichTestSim,1)
};

#endif
