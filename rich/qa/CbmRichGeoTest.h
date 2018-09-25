/**
* \file CbmRichGeoTest.h
*
* \brief RICH geometry checking and testing.
*
* \author Semen Lebedev <s.lebedev@gsi.de>
* \date 2011
**/

#ifndef CBMRICHGEOTEST
#define CBMRICHGEOTEST

#include "FairTask.h"
class TH1;
class TH2;
class TH1D;
class TH2D;
class TH3D;
class TClonesArray;
class CbmRichRingFitterCOP;
class CbmRichRingFitterEllipseTau;
class CbmRichRing;
class CbmRichRingLight;
class CbmGeoRichPar;
class TCanvas;

#include <vector>

using namespace std;

/**
 * \class CbmRichGeoTest
 *
 * \brief RICH geometry checking and testing.
 *
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2011
 **/
class CbmRichGeoTest : public FairTask
{

public:
   /**
    * \brief Standard constructor.
    */
   CbmRichGeoTest();

   /**
    * \brief Standard destructor.
    */
   virtual ~CbmRichGeoTest();

   /**
    * \brief SetParContainers.
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
		   Option_t* option);

   /**
    * \brief Inherited from FairTask.
    */
   virtual void Finish();

   /**
    * \brief Creates summary report for different simulations. This is just an
    * interface. All functionality is implemented in CbmRichGeoTestStudyReport class.
    * \param[in] title Report title.
    * \param[in] resultDirectories Paths to directories with results for each simulation.
    * \param[in] studyNames Name of each study.
    * \param[in] outputDir Path to the output directory.
    */
   void CreateStudyReport(
         const string& title,
         const vector<string>& resultDirectories,
         const vector<string>& studyNames,
         const string& outputDir);

   /**
    * \brief Set output directory where you want to write results (figures and json).
    * \param[in] dir Path to the output directory.
    */
   void SetOutputDir(const string& dir) {fOutputDir = dir;}

   /**
    * \brief Set detector type ("standard" or "prototype").
    * \param[in] type Detector type.
    */
   void SetRichDetectorType(const string& type) {fRichDetectorType = type;}

private:

   /**
    * \brief Initialize histograms.
    */
   void InitHistograms();

   /**
    * \brief Temporary function for summary plots drawing.
    */
   void DrawSummaryPlotsTemp();


   /**
    * \brief Fill MC histogram for detector acceptance calculation.
    */
   void FillMcHist();

   /**
    * \brief Loop over all rings in array and fill ring parameters histograms.
    */
	void RingParameters();
	
   /**
    * \brief Fit ring using ellipse fitter and fill histograms.
    * \param[in] histIndex Fitting type index, 0 - hit fitting, 1 - MC points fitting.
    * \param[in] ring Pointer to CbmRichRing to be fitted and filled in histograms.
    * \param[in] momentum MC momentum of particle produced ring.
    */
	void FitAndFillHistEllipse(
	      Int_t histIndex,
	      CbmRichRingLight* ring,
	      Double_t momentum);

   /**
    * \brief Fit ring using circle fitter and fill histograms.
    * \param[in] histIndex Fitting type index, 0 - hit fitting, 1 - MC points fitting.
    * \param[in] ring Pointer to CbmRichRingLight to be fitted and filled in histograms.
    * \param[in] momentum MC momentum of particle produced ring.
    */
   void FitAndFillHistCircle(
         Int_t histIndex,
         CbmRichRingLight* ring,
         Double_t momentum);

   /**
    * \brief Calculate difference between ellipse parameters
    *  for two fitting using hits and MC points for fit and fill
    *  corresponding histograms.
    * \param[in] ring Ring fitted using hits.
    * \param[in] ringMc Ring fitted using MC points
    */
   void FillMcVsHitFitEllipse(
         CbmRichRingLight* ring,
         CbmRichRingLight* ringMc);

   /**
    * \brief Calculate difference between circle parameters
    *  for two fittings using hits and MC points for fit and fill
    *  corresponding histograms.
    * \param[in] ring Ring fitted using hits.
    * \param[in] ringMc Ring fitted using MC points
    */
   void FillMcVsHitFitCircle(
         CbmRichRingLight* ring,
         CbmRichRingLight* ringMc);

   /**
    * \brief Calculate residuals between hits and MC points and fill histograms.
    */
   void HitsAndPoints();

   /**
    * \brief Create histogram: RICH detector acceptance vs.
    * minimum required number of hits in ring
    */
	TH1D* CreateAccVsMinNofHitsHist();

   /**
    *  \brief Draw histograms.
    */
	void DrawHist();

	/**
	 * \brief Draw ring in separate TCanvas.
	 * \param[in] ring Ring with RICH hits.
	 * \param[in] ringPoint Ring with MC RICH points.
	 */
	void DrawRing(
	      CbmRichRingLight* ringHit,
	      CbmRichRingLight* ringPoint);
	/**
	 * \brief Creates two histogram. First is mean value vs. x axis,
	 * errors represents RMS. Second is RMS value vs. x axis.
	 * \param[in] hist Input 2D histogram.
	 * \param[out] meanHist Histogram of mean values.
	 * \param[out] rmsHist Histogram of RMS values.
	 */
	void CreateH2MeanRms(
	      TH2D* hist,
	      TH1D** meanHist,
	      TH1D** rmsHist);

	void DrawH1andFit(
	      TH1D* hist);

	void DrawH2MeanRms(
	      TH2D* hist,
	      const string& canvasName);

	void FitH1OneOverX(
	      TH1D* hist,
	      double xMinFit,
	      double xMaxFit,
	      double xMin,
	      double xMax,
	      const string& canvasName);

//	/**
//	 * \brief Create property tree an store different statistics there.
//	 */
//	void CreatePTree();

	/**
	 * \brief Calculate efficiency.
	 * \param[in] histRec
	 * \param[in] histAcc
	 */
	string CalcEfficiency(
	   TH1* histRec,
	   TH1* histAcc);

	/**
	 * \brief Divide two histograms and create third one.
	 */
	TH1D* DivideH1(
	   TH1D* h1,
	   TH1D* h2,
	   const string& name,
	   const string& title,
	   const string& axisX,
	   const string& axisY);

	TH2D* DivideH2(
	   TH1D* h1,
	   TH1D* h2,
	   const string& name,
	   const string& title,
	   const string& axisX,
	   const string& axisY,
	   const string& axisZ);

	TCanvas* CreateCanvas(
	      const string& name,
	      const string& title,
	      int width,
	      int height);

	void DrawH3(
	      TH3D* h,
	      const string& cName,
	      const string& zAxisTitle,
	      double zMin,
	      double zMax,
	      bool doFit = false);

	void SaveCanvasToImage();

   /**
    * \brief Copy constructor.
    */
   CbmRichGeoTest(const CbmRichGeoTest&);

   /**
    * \brief Assignment operator.
    */
   CbmRichGeoTest& operator=(const CbmRichGeoTest&);

   string fRichDetectorType; // "standard" or prototype

   string fOutputDir; // output dir for results

	TClonesArray* fRichHits;
	TClonesArray* fRichRings;
	TClonesArray* fRichPoints; 
	TClonesArray* fMCTracks;
	TClonesArray* fRichRingMatches; 
	
   Double_t fDetZOrig; // X-coordinate of photodetector (original from parameter file)
   Double_t fTheta; // angle by which photodetector was tilted around x-axis
   Double_t fPhi; // angle by which photodetector was tilted around y-axis
   TObjArray* fSensNodes;
   TObjArray* fPassNodes;
   CbmGeoRichPar* fPar;

   // rings will be fitted on a fly
	CbmRichRingFitterCOP* fCopFit;
	CbmRichRingFitterEllipseTau* fTauFit;

	vector<TCanvas*> fCanvas;

	Int_t fEventNum;
	Int_t fMinNofHits; // Min number of hits in ring for detector acceptance calculation.

	TH2D* fhHitsXY; // distribution of X and Y position of hits
	TH2D* fhPointsXY; // distribution of X and Y position of points
	TH1D* fhNofPhotonsPerHit; // Number of photons per hit

   // fitting parameters
	// [0] = hits fit, [1] = MC points fit
	vector<TH1D*> fhNofHits; // number of hits per ring
	// for ellipse
	vector<TH2D*> fhAaxisVsMom; // major axis (A) vs. MC momentum
	vector<TH2D*> fhBaxisVsMom; // minor axis (B) vs. MC momentum
	vector<TH1D*> fhBoverA; // B/A distribution
	vector<TH2D*> fhXcYcEllipse; // (Xc, Yc) of ellipse center
   vector<TH2D*> fhChi2EllipseVsMom; // Chi2
   // for circle
	vector<TH2D*> fhXcYcCircle; // (Xc, Yc) of circle center
	vector<TH2D*> fhRadiusVsMom; // circle radius vs. MC momentum
   vector<TH2D*> fhChi2CircleVsMom; // chi2
   vector<TH2D*> fhDRVsMom; // dR

   // R, A, B distribution for different number of hits from 0 to 40
   TH2D* fhRadiusVsNofHits;
   TH2D* fhAaxisVsNofHits;
   TH2D* fhBaxisVsNofHits;

	// Difference between MC Points and Hits fit
   // for ellipse fitting parameters
   TH2D* fhDiffAaxis; // major axis (A)
   TH2D* fhDiffBaxis; // minor axis (B)
   TH2D* fhDiffXcEllipse; // Xc of ellipse center
   TH2D* fhDiffYcEllipse; // Yc of ellipse center
   // for circle fitting parameters
   TH2D* fhDiffXcCircle; // Xc of circle center
   TH2D* fhDiffYcCircle; // Xc of circle center
   TH2D* fhDiffRadius; // circle radius

   // Hits and points
   TH1D* fhDiffXhit;
   TH1D* fhDiffYhit;

   // fitting efficiency
   Double_t fMinAaxis;
   Double_t fMaxAaxis;
   Double_t fMinBaxis;
   Double_t fMaxBaxis;
   Double_t fMinRadius;
   Double_t fMaxRadius;
   TH1D* fhNofHitsAll; // distribution of the number of hits in ring for all
   TH1D* fhNofHitsCircleFit; // distribution of the number of hits in ring
                             // for good fitted rings using circle  fitting
   TH1D* fhNofHitsEllipseFit; // distribution of the number of hits in ring
                              // for good fitted rings using ellipse  fitting
   TH1D* fhNofHitsCircleFitEff;
   TH1D* fhNofHitsEllipseFitEff;

   // Detector acceptance vs (pt,y) and p for e+/- and pi+/-
   TH1D* fh_mc_mom_el;
   TH2D* fh_mc_pty_el;
   TH1D* fh_acc_mom_el;
   TH2D* fh_acc_pty_el;

   TH1D* fh_mc_mom_pi;
   TH2D* fh_mc_pty_pi;
   TH1D* fh_acc_mom_pi;
   TH2D* fh_acc_pty_pi;


   // numbers in dependence on XY position onto the photodetector
   TH3D* fhNofHitsXYZ; // number of hits
   TH3D* fhNofPointsXYZ; // number of points
   TH3D* fhBoverAXYZ; // B/A ratio
   TH3D* fhBaxisXYZ; // B axis
   TH3D* fhAaxisXYZ; // A axis
   TH3D* fhRadiusXYZ; // Radius
   TH3D* fhdRXYZ; // dR

	vector<TH1*> fHists; // store all TH1 pointers of the histogram

	Int_t fNofDrawnRings; // store number of drawn rings

	ClassDef(CbmRichGeoTest,1)
};

#endif

