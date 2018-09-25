/**
* \file CbmRichUrqmdTest.h
*
* \brief RICH geometry testing in Urqmd collisions.
*
* \author Semen Lebedev <s.lebedev@gsi.de>
* \date 2012
**/

#ifndef CBM_RICH_URQMD_TEST
#define CBM_RICH_URQMD_TEST

#include "FairTask.h"
class TH1;
class TH2;
class TH1D;
class TH2D;
class TClonesArray;
class CbmRichRing;
class TCanvas;

#include <vector>
#include <map>

using namespace std;

/**
 * \class CbmRichUrqmdTest
 *
 * \brief RICH geometry testing in Urqmd collisions.
 *
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2012
 **/
class CbmRichUrqmdTest : public FairTask
{

public:
   /**
    * \brief Standard constructor.
    */
   CbmRichUrqmdTest();

   /**
    * \brief Standard destructor.
    */
   virtual ~CbmRichUrqmdTest();

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


   /**
    * \brief Set output directory where you want to write results (figures and json).
    * \param[in] dir Path to the output directory.
    */
   void SetOutputDir(const string& dir) {fOutputDir = dir;}


private:

   /**
    * \brief Initialize histograms.
    */
   void InitHistograms();

   /**
    * \brief
    */
   void FillRichRingNofHits();

   /**
    * \brief
    */
   void NofRings();

   /**
    * \brief
    */
   void NofHits();

   /**
    * \brief
    */
   void NofProjections();

   /**
    * \brief
    */
   void Vertex();


   /**
    *  \brief Draw histograms.
    */
	void DrawHist();

	TCanvas* CreateCanvas(
	      const string& name,
	      const string& title,
	      int width,
	      int height);

	void SaveCanvasToImage();

   /**
    * \brief Copy constructor.
    */
   CbmRichUrqmdTest(const CbmRichUrqmdTest&);

   /**
    * \brief Assignment operator.
    */
   CbmRichUrqmdTest& operator=(const CbmRichUrqmdTest&);


   string fOutputDir; // output dir for results

	TClonesArray* fRichHits;
	TClonesArray* fRichRings;
	TClonesArray* fRichPoints; 
	TClonesArray* fMcTracks;
	TClonesArray* fRichRingMatches; 
	TClonesArray* fRichProjections;


	vector<TCanvas*> fCanvas;

	Int_t fEventNum;
	Int_t fMinNofHits; // Min number of hits in ring for detector acceptance calculation.

   // Number of hits in the MC RICH ring
   std::map<Int_t, Int_t> fNofHitsInRingMap;

	TH1* fh_vertex_z;
	TH1* fh_nof_rings_1hit;
	TH1* fh_nof_rings_7hits;
   TH1* fh_nof_rings_prim_1hit;
   TH1* fh_nof_rings_prim_7hits;
   TH1* fh_nof_rings_target_1hit;
   TH1* fh_nof_rings_target_7hits;

   TH1* fh_secel_mom;
   TH1* fh_gamma_target_mom; // secondary electrons from gamma conversion from target region vertex < 0.5
   TH1* fh_gamma_nontarget_mom;// secondary electrons from gamma conversion NOT from target region vertex > 0.5
   TH1* fh_pi_mom;
   TH1* fh_kaon_mom;
   TH1* fh_mu_mom;

   TH1* fh_nof_hits_per_event;
   TH2D* fh_hits_xy_u;
   TH2D* fh_hits_xy_d;

   TH2D* fh_hitrate_xy_u;
   TH2D* fh_hitrate_xy_d;

   TH1* fh_nof_proj_per_event;
   TH2D* fh_proj_xy_u;
   TH2D* fh_proj_xy_d;

	vector<TH1*> fHists; // store all TH1 pointers of the histogram

	ClassDef(CbmRichUrqmdTest,1)
};

#endif

