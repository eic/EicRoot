/**
 * \file CbmRichGeoTest.cxx
 *
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2011
 **/

#include "CbmRichGeoTest.h"
#include "CbmRichGeoTestStudyReport.h"
#include "FairRootManager.h"
#include "CbmRichHit.h"
#include "CbmRichRing.h"
#include "CbmTrackMatch.h"
#include "CbmMCTrack.h"
#include "CbmRichPoint.h"
#include "CbmRichRingFitterCOP.h"
#include "CbmRichRingFitterEllipseTau.h"
#include "CbmGeoRichPar.h"
#include "FairGeoTransform.h"
#include "FairGeoNode.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "CbmRichHitProducer.h"
#include "CbmDrawHist.h"
#include "std/utils/CbmLitUtils.h"
#include "CbmRichConverter.h"
#include "CbmReport.h"
#include "CbmStudyReport.h"

#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TCanvas.h"
#include "TEllipse.h"
#include "TClonesArray.h"
#include "TMath.h"
#include "TPad.h"
#include "TLatex.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TF1.h"
#include "TLegend.h"

#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <string>

#include <boost/assign/list_of.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace std;
using boost::assign::list_of;
using boost::property_tree::ptree;

CbmRichGeoTest::CbmRichGeoTest():
   FairTask("RichGeoTestQa"),

   fRichDetectorType("standard"),

   fOutputDir(""),

   fRichHits(NULL),
   fRichRings(NULL),
   fRichPoints(NULL),
   fMCTracks(NULL),
   fRichRingMatches(NULL),

   fDetZOrig(0.),
   fTheta(0.),
   fPhi(0.),
   fSensNodes(NULL),
   fPassNodes(NULL),
   fPar(NULL),

   fCopFit(NULL),
   fTauFit(NULL),
   fCanvas(),
   fEventNum(0),
   fMinNofHits(0),

   fhHitsXY(NULL),
   fhPointsXY(NULL),
   fhNofPhotonsPerHit(NULL),

   fhNofHits(),
   fhAaxisVsMom(),
   fhBaxisVsMom(),
   fhBoverA(),
   fhXcYcEllipse(),
   fhChi2EllipseVsMom(),
   fhXcYcCircle(),
   fhRadiusVsMom(),
   fhChi2CircleVsMom(),
   fhDRVsMom(),

   fhRadiusVsNofHits(NULL),
   fhAaxisVsNofHits(NULL),
   fhBaxisVsNofHits(NULL),
   fhDiffAaxis(NULL),
   fhDiffBaxis(NULL),
   fhDiffXcEllipse(NULL),
   fhDiffYcEllipse(NULL),
   fhDiffXcCircle(NULL),
   fhDiffYcCircle(NULL),
   fhDiffRadius(NULL),

   fhDiffXhit(NULL),
   fhDiffYhit(NULL),

   fMinAaxis(0.),
   fMaxAaxis(0.),
   fMinBaxis(0.),
   fMaxBaxis(0.),
   fMinRadius(0.),
   fMaxRadius(0.),

   fhNofHitsAll(NULL),
   fhNofHitsCircleFit(NULL),
   fhNofHitsEllipseFit(NULL),
   fhNofHitsCircleFitEff(NULL),
   fhNofHitsEllipseFitEff(NULL),

   fh_mc_mom_el(NULL),
   fh_mc_pty_el(NULL),
   fh_acc_mom_el(NULL),
   fh_acc_pty_el(NULL),

   fh_mc_mom_pi(NULL),
   fh_mc_pty_pi(NULL),
   fh_acc_mom_pi(NULL),
   fh_acc_pty_pi(NULL),

   fhNofHitsXYZ(NULL),
   fhNofPointsXYZ(NULL),
   fhBoverAXYZ(NULL),
   fhBaxisXYZ(NULL),
   fhAaxisXYZ(NULL),
   fhRadiusXYZ(NULL),
   fhdRXYZ(NULL),

   fHists(),

   fNofDrawnRings(0)

{
	fEventNum = 0;
	fNofDrawnRings = 0;
	fMinNofHits = 7;

   fMinAaxis = 3.;
   fMaxAaxis = 7.;
   fMinBaxis = 3.;
   fMaxBaxis = 7.;
   fMinRadius = 3.;
   fMaxRadius = 7.;

}

CbmRichGeoTest::~CbmRichGeoTest()
{

}

void CbmRichGeoTest::SetParContainers()
{
  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb=ana->GetRuntimeDb();
  fPar=(CbmGeoRichPar*)(rtdb->getContainer("CbmGeoRichPar"));
}

InitStatus CbmRichGeoTest::Init()
{
   cout << "CbmRichGeoTest::Init"<<endl;
   FairRootManager* ioman = FairRootManager::Instance();
   if (NULL == ioman) { Fatal("CbmRichGeoTest::Init","RootManager not instantised!"); }

   if (fRichDetectorType == "standard") {
      fSensNodes = fPar->GetGeoSensitiveNodes();
      fPassNodes = fPar->GetGeoPassiveNodes();

      // get detector position:
      FairGeoNode *det= dynamic_cast<FairGeoNode*> (fSensNodes->FindObject("rich1d#1"));
      if (NULL == det) cout << " -I no RICH Geo Node  found !!!!!  " << endl;

      FairGeoTransform* detTr=det->getLabTransform(); // detector position in labsystem
      FairGeoVector detPosLab=detTr->getTranslation(); // ... in cm
      FairGeoTransform detCen=det->getCenterPosition(); // center in Detector system
      FairGeoVector detPosCen=detCen.getTranslation();

      fDetZOrig = detPosLab.Z() + detPosCen.Z(); // z coordinate of photodetector (Labsystem, cm)
      FairGeoRotation fdetR=detTr->getRotMatrix();
      fTheta = TMath::ASin(fdetR(7)); // tilting angle around x-axis
      fPhi = -1.*TMath::ASin(fdetR(2)); // tilting angle around y-axis
   }

   fRichHits = (TClonesArray*) ioman->GetObject("RichHit");
   if ( NULL == fRichHits) { Fatal("CbmRichGeoTest::Init","No RichHit array!"); }

	fRichRings = (TClonesArray*) ioman->GetObject("RichRing");
	if ( NULL == fRichRings) { Fatal("CbmRichGeoTest::Init","No RichRing array!"); }

	fRichPoints = (TClonesArray*) ioman->GetObject("RichPoint");
	if ( NULL == fRichPoints) { Fatal("CbmRichGeoTest::Init","No RichPoint array!"); }

	fMCTracks = (TClonesArray*) ioman->GetObject("MCTrack");
	if ( NULL == fMCTracks) { Fatal("CbmRichGeoTest::Init","No MCTrack array!"); }

	fRichRingMatches = (TClonesArray*) ioman->GetObject("RichRingMatch");
	if ( NULL == fRichRingMatches) { Fatal("CbmRichGeoTest::Init","No RichRingMatch array!"); }

   fCopFit = new CbmRichRingFitterCOP();
   fTauFit = new CbmRichRingFitterEllipseTau();

   InitHistograms();

	return kSUCCESS;
}

void CbmRichGeoTest::Exec(
      Option_t* option)
{
	fEventNum++;
	cout << "CbmRichGeoTest, event No. " <<  fEventNum << endl;
	FillMcHist();
	RingParameters();
	HitsAndPoints();
}

void CbmRichGeoTest::InitHistograms()
{
   double xMin = -110.;
   double xMax = 110.;
   int nBinsX = 28;
   double yMin = -200;
   double yMax = 200.;
   int nBinsY = 40;
   if (fRichDetectorType == "prototype"){
      xMin = -10.;
      xMax = 10.;
      nBinsX = 50;
      yMin = 25.;
      yMax = 45.;
      nBinsY = 50;
   }

   fhHitsXY = new TH2D("fhHitsXY", "fhHitsXY;X [cm];Y [cm];Counter", nBinsX, xMin, xMax, nBinsY, yMin, yMax);
   fHists.push_back(fhHitsXY);
   fhPointsXY = new TH2D("fhPointsXY", "fhPointsXY;X [cm];Y [cm];Counter", nBinsX, xMin, xMax, nBinsY, yMin, yMax);
   fHists.push_back(fhPointsXY);

   fhNofHits.resize(2);
   fhBoverA.resize(2);
   fhXcYcEllipse.resize(2);
   fhXcYcCircle.resize(2);
   fhBaxisVsMom.resize(2);
   fhAaxisVsMom.resize(2);
   fhRadiusVsMom.resize(2);
   fhChi2EllipseVsMom.resize(2);
   fhChi2CircleVsMom.resize(2);
   fhDRVsMom.resize(2);
   for (Int_t i = 0; i < 2; i++){
      stringstream ss;
      if (i == 0) ss << "_hits";
      if (i == 1) ss << "_points";
      string t = ss.str();
      fhNofHits[i] = new TH1D(("fhNofHits"+t).c_str(), ("fhNofHits"+t+";Nof hits in ring;Yield").c_str(), 300, -.5, 299.5);
      fHists.push_back(fhNofHits[i]);
      // ellipse fitting parameters
      fhBoverA[i] = new TH1D(("fhBoverA"+t).c_str(), ("fhBoverA"+t+";B/A;Yield").c_str(), 50, 0, 1);
      fHists.push_back(fhBoverA[i]);
      fhXcYcEllipse[i] = new TH2D(("fhXcYcEllipse"+t).c_str(), ("fhXcYcEllipse"+t+";x [cm];y [cm];Yield").c_str(), nBinsX, xMin, xMax, nBinsY, yMin, yMax);
      fHists.push_back(fhXcYcEllipse[i]);
      fhBaxisVsMom[i] = new TH2D(("fhBaxisVsMom"+t).c_str(), ("fhBaxisVsMom"+t+";p [GeV/c];B axis [cm];Yield").c_str(), 100, 0., 10, 200, 0., 10.);
      fHists.push_back(fhBaxisVsMom[i]);
      fhAaxisVsMom[i] = new TH2D(("fhAaxisVsMom"+t).c_str(), ("fhAaxisVsMom"+t+";p [GeV/c];A axis [cm];Yield").c_str(), 100, 0., 10, 200, 0., 10.);
      fHists.push_back(fhAaxisVsMom[i]);
      fhChi2EllipseVsMom[i] = new TH2D(("fhChi2EllipseVsMom"+t).c_str(), ("fhChi2EllipseVsMom"+t+";p [GeV/c];#Chi^{2};Yield").c_str(), 100, 0., 10., 50, 0., 0.5);
      fHists.push_back(fhChi2EllipseVsMom[i]);
      // circle fitting parameters
      fhXcYcCircle[i] = new TH2D(("fhXcYcCircle"+t).c_str(), ("fhXcYcCircle"+t+";x [cm];y [cm];Yield").c_str(), nBinsX, xMin, xMax, nBinsY, yMin, yMax);
      fHists.push_back(fhXcYcCircle[i]);
      fhRadiusVsMom[i] = new TH2D(("fhRadiusVsMom"+t).c_str(), ("fhRadiusVsMom"+t+";p [GeV/c];Radius [cm];Yield").c_str(), 100, 0., 10, 200, 0., 10.);
      fHists.push_back(fhRadiusVsMom[i]);
      fhChi2CircleVsMom[i] = new TH2D(("fhChi2CircleVsMom"+t).c_str(), ("fhChi2CircleVsMom"+t+";p [GeV/c];#Chi^{2};Yield").c_str(), 100, 0., 10., 50, 0., .5);
      fHists.push_back(fhChi2CircleVsMom[i]);
      fhDRVsMom[i] = new TH2D(("fhDRVsMom"+t).c_str(), ("fhDRVsMom"+t+";p [GeV/c];dR [cm];Yield").c_str(), 100, 0, 10, 100, -1., 1.);
      fHists.push_back(fhDRVsMom[i]);
   }

   fhNofPhotonsPerHit = new TH1D("fhNofPhotonsPerHit", "fhNofPhotonsPerHit;Number of photons per hit;Yield", 10, -0.5, 9.5);
   fHists.push_back(fhNofPhotonsPerHit);
   // Difference between Mc Points and Hits fitting.
   fhDiffAaxis = new TH2D("fhDiffAaxis", "fhDiffAaxis;Nof hits in ring;A_{point}-A_{hit} [cm];Yield", 40, 0., 40., 100, -1.5, 1.5);
   fHists.push_back(fhDiffAaxis);
   fhDiffBaxis = new TH2D("fhDiffBaxis", "fhDiffBaxis;Nof hits in ring;B_{point}-B_{hit} [cm];Yield", 40, 0., 40., 100, -1.5, 1.5);
   fHists.push_back(fhDiffBaxis);
   fhDiffXcEllipse = new TH2D("fhDiffXcEllipse", "fhDiffXcEllipse;Nof hits in ring;Xc_{point}-Xc_{hit} [cm];Yield", 40, 0., 40., 100, -1.5, 1.5);
   fHists.push_back(fhDiffXcEllipse);
   fhDiffYcEllipse = new TH2D("fhDiffYcEllipse", "fhDiffYcEllipse;Nof hits in ring;Yc_{point}-Yc_{hit} [cm];Yield", 40, 0., 40., 100, -1.5, 1.5);
   fHists.push_back(fhDiffYcEllipse);
   fhDiffXcCircle = new TH2D("fhDiffXcCircle", "fhDiffXcCircle;Nof hits in ring;Xc_{point}-Xc_{hit} [cm];Yield", 40, 0., 40., 100, -1.5, 1.5);
   fHists.push_back(fhDiffXcCircle);
   fhDiffYcCircle = new TH2D("fhDiffYcCircle", "fhDiffYcCircle;Nof hits in ring;Yc_{point}-Yc_{hit} [cm];Yield", 40, 0., 40., 100, -1.5, 1.5);
   fHists.push_back(fhDiffYcCircle);
   fhDiffRadius = new TH2D("fhDiffRadius", "fhDiffRadius;Nof hits in ring;Radius_{point}-Radius_{hit} [cm];Yield", 40, 0., 40., 100, -1.5, 1.5);
   fHists.push_back(fhDiffRadius);

   // R, A, B distribution for different number of hits from 0 to 40.
   fhRadiusVsNofHits = new TH2D("fhRadiusVsNofHits", "fhRadiusVsNofHits;Nof hits in ring;Radius [cm];Yield", 40, 0., 40., 100, 0., 10.);
   fhAaxisVsNofHits = new TH2D("fhAaxisVsNofHits", "fhAaxisVsNofHits;Nof hits in ring;A axis [cm];Yield", 40, 0., 40., 100, 0., 10.);
   fhBaxisVsNofHits = new TH2D("fhBaxisVsNofHits", "fhBaxisVsNofHits;Nof hits in ring;B axis [cm];Yield", 40, 0., 40., 100, 0., 10.);

   // Hits and points.
   fhDiffXhit = new TH1D("fhDiffXhit", "fhDiffXhit;Y_{point}-Y_{hit} [cm];Yield", 200, -1., 1.);
   fHists.push_back(fhDiffXhit);
   fhDiffYhit = new TH1D("fhDiffYhit", "fhDiffYhit;Y_{point}-Y_{hit} [cm];Yield", 200, -1., 1.);
   fHists.push_back(fhDiffYhit);

   // Fitting efficiency.
   fhNofHitsAll = new TH1D("fhNofHitsAll", "fhNofHitsAll;Nof hits in ring;Yield", 50, 0., 50.);
   fHists.push_back(fhNofHitsAll);
   fhNofHitsCircleFit = new TH1D("fhNofHitsCircleFit", "fhNofHitsCircleFit;Nof hits in ring;Yield", 50, 0., 50.);
   fHists.push_back(fhNofHitsCircleFit);
   fhNofHitsEllipseFit = new TH1D("fhNofHitsEllipseFit", "fhNofHitsEllipseFit;Nof hits in ring;Yield", 50, 0., 50.);
   fHists.push_back(fhNofHitsEllipseFit);
   fhNofHitsCircleFitEff = new TH1D("fhNofHitsCircleFitEff", "fhNofHitsCircleFitEff;Nof hits in ring;Efficiency [%]", 50, 0., 50.);
   fHists.push_back(fhNofHitsCircleFitEff);
   fhNofHitsEllipseFitEff = new TH1D("fhNofHitsEllipseFitEff", "fhNofHitsEllipseFitEff;Nof hits in ring;Efficiency [%]", 50, 0., 50.);
   fHists.push_back(fhNofHitsEllipseFitEff);

   // Detector acceptance efficiency vs. (pt,y) and p
   fh_mc_mom_el = new TH1D("fh_mc_mom_el", "fh_mc_mom_el;p [GeV/c];Yield", 24, 0., 12.);
   fHists.push_back(fh_mc_mom_el);
   fh_mc_pty_el = new TH2D("fh_mc_pty_el", "fh_mc_pty_el;Rapidity;P_{t} [GeV/c];Yield", 25, 0., 4., 20, 0., 3.);
   fHists.push_back(fh_mc_pty_el);
   fh_acc_mom_el = new TH1D("fh_acc_mom_el", "fh_acc_mom_el;p [GeV/c];Yield", 24, 0., 12.);
   fHists.push_back(fh_acc_mom_el);
   fh_acc_pty_el = new TH2D("fh_acc_pty_el", "fh_acc_pty_el;Rapidity;P_{t} [GeV/c];Yield",25, 0., 4., 20, 0., 3.);
   fHists.push_back(fh_acc_pty_el);

   fh_mc_mom_pi = new TH1D("fh_mc_mom_pi", "fh_mc_mom_pi;p [GeV/c];Yield", 24, 0., 12.);
   fHists.push_back(fh_mc_mom_pi);
   fh_mc_pty_pi = new TH2D("fh_mc_pty_pi", "fh_mc_pty_pi;Rapidity;P_{t} [GeV/c];Yield", 25, 0., 4., 20, 0., 3.);
   fHists.push_back(fh_mc_pty_pi);
   fh_acc_mom_pi = new TH1D("fh_acc_mom_pi", "fh_acc_mom_pi;p [GeV/c];Yield", 24, 0., 12.);
   fHists.push_back(fh_acc_mom_pi);
   fh_acc_pty_pi = new TH2D("fh_acc_pty_pi", "fh_acc_pty_pi;Rapidity;P_{t} [GeV/c];Yield", 25, 0., 4., 20, 0., 3.);
   fHists.push_back(fh_acc_pty_pi);

   // Numbers in dependence on XY position onto the photodetector.
   fhNofHitsXYZ = new TH3D("fhNofHitsXY", "fhNofHitsXY;X [cm];Y [cm];Nof hits in ring", nBinsX, xMin, xMax, nBinsY, yMin, yMax, 50, 0., 50);
   fHists.push_back(fhNofHitsXYZ);
   fhNofPointsXYZ = new TH3D("fhNofPointsXYZ", "fhNofPointsXYZ;X [cm];Y [cm];Nof points in ring", nBinsX, xMin, xMax, nBinsY, yMin, yMax, 50, 100., 300.);
   fHists.push_back(fhNofPointsXYZ);
   fhBoverAXYZ = new TH3D("fhBoverAXYZ", "fhBoverAXYZ;X [cm];Y [cm];B/A", nBinsX, xMin, xMax, nBinsY, yMin, yMax, 50, 0., 1.);
   fHists.push_back(fhBoverAXYZ);
   fhBaxisXYZ = new TH3D("fhBaxisXYZ", "fhBaxisXYZ;X [cm];Y [cm];B axis [cm]", nBinsX, xMin, xMax, nBinsY, yMin, yMax, 80, 3., 7.);
   fHists.push_back(fhBaxisXYZ);
   fhAaxisXYZ = new TH3D("fhAaxisXYZ", "fhAaxisXYZ;X [cm];Y [cm];A axis [cm]", nBinsX, xMin, xMax, nBinsY, yMin, yMax, 80, 3., 7.);
   fHists.push_back(fhAaxisXYZ);
   fhRadiusXYZ = new TH3D("fhRadiusXYZ", "fhRadiusXYZ;X [cm];Y [cm];Radius [cm]", nBinsX, xMin, xMax, nBinsY, yMin, yMax, 80, 3., 7.);
   fHists.push_back(fhRadiusXYZ);
   fhdRXYZ = new TH3D("fhdRXYZ", "fhdRXYZ;X [cm];Y [cm];dR [cm]", nBinsX, xMin, xMax, nBinsY, yMin, yMax, 100, -1., 1.);
   fHists.push_back(fhdRXYZ);
   DrawSummaryPlotsTemp();
}

void CbmRichGeoTest::DrawSummaryPlotsTemp()
{
   TCanvas *c1 = CreateCanvas("chi2_mean_fit_vs_bfield_scale", "chi2_mean_fit_vs_bfield_scale", 600, 600);
   Double_t x[6]={0.2, 0.5, 0.7, 1., 1.5, 2.0};
   Double_t y1[6]={0.023, 0.031, 0.034, 0.039, 0.039, 0.045};
   TGraph* gr1 = new TGraph(6, x, y1);
   gr1->GetXaxis()->SetTitle("Field scales");
   gr1->GetYaxis()->SetTitle("a_{fit}^{#Chi^{2}.mean}");
   TF1 *f1 = new TF1("f1", "[0]*x+[1]", 0., 2.1);
   f1->SetLineColor(kBlack);
   gr1->GetXaxis()->SetRangeUser(0.1, 2.1);
   DrawGraph(gr1, kLinear, kLinear, "AP");
   gr1->SetMarkerColor(4);
   gr1->SetMarkerSize(2.5);
   gr1->SetMarkerStyle(21);
   gr1->Fit(f1, "RQ");
   gr1->GetXaxis()->SetRangeUser(0.1, 2.1);
   f1->SetLineColor(kBlack);
   double p0 = f1->GetParameter(0);
   double p1 = f1->GetParameter(1);
   stringstream ss;
   ss.precision(3);
   ss << "y="<<lit::NumberToString(p0, 1)<<"*x+"<<lit::NumberToString(p1, 1);
   TLegend* leg = new TLegend(0.15, 0.9, 0.85, 0.99);
   leg->AddEntry(new TH2D(), ss.str().c_str(), "");
   leg->SetFillColor(kWhite);
   leg->SetFillStyle(0);
   leg->SetBorderSize(0);
   leg->Draw();


   TCanvas *c2 = CreateCanvas("dr_rms_fit_vs_bfield_scale", "dr_rms_fit_vs_bfield_scale", 600, 600);
   Double_t x2[6]={0.2, 0.5, 0.7, 1., 1.5, 2.0};
   Double_t y2[6]={0.027, 0.04, 0.05, 0.057, 0.074, 0.08};
   TGraph* gr2 = new TGraph(6, x2, y2);
   gr2->GetXaxis()->SetTitle("Field scales");
   gr2->GetYaxis()->SetTitle("a_{fit}^{dR.RMS}");
   TF1 *f2 = new TF1("f2", "[0]*x+[1]", 0., 2.1);
   f2->SetLineColor(kBlack);
   DrawGraph(gr2, kLinear, kLinear, "AP");
   gr2->SetMarkerColor(4);
   gr2->SetMarkerSize(2.5);
   gr2->SetMarkerStyle(21);
   gr2->Fit(f2, "RQ");
   gr2->GetXaxis()->SetRange(0., 2.1);
   f2->SetLineColor(kBlack);
   double p10 = f2->GetParameter(0);
   double p11 = f2->GetParameter(1);
   stringstream ss2;
   ss2 << "y="<<lit::NumberToString(p10, 1)<<"*x+"<<lit::NumberToString(p11, 1);
   TLegend* leg2 = new TLegend(0.15, 0.9, 0.85, 0.99);
   leg2->AddEntry(new TH2D(), ss2.str().c_str(), "");
   leg2->SetFillColor(kWhite);
   leg2->SetFillStyle(0);
   leg2->SetBorderSize(0);
   leg2->Draw();
}

void CbmRichGeoTest::FillMcHist()
{
   for (Int_t i = 0; i < fMCTracks->GetEntriesFast(); i++){
      CbmMCTrack* mcTrack = (CbmMCTrack*)fMCTracks->At(i);
      if (!mcTrack) continue;
      Int_t motherId = mcTrack->GetMotherId();
      Int_t pdg = TMath::Abs(mcTrack->GetPdgCode());

      if (pdg == 11 && motherId == -1){
         fh_mc_mom_el->Fill(mcTrack->GetP());
         fh_mc_pty_el->Fill(mcTrack->GetRapidity(), mcTrack->GetPt());
      }

      if (pdg == 211 && motherId == -1){
         fh_mc_mom_pi->Fill(mcTrack->GetP());
         fh_mc_pty_pi->Fill(mcTrack->GetRapidity(), mcTrack->GetPt());
      }
   }
}

void CbmRichGeoTest::RingParameters()
{
	Int_t nofRings = fRichRings->GetEntriesFast();
	for (Int_t iR = 0; iR < nofRings; iR++){
		CbmRichRing *ring = (CbmRichRing*) fRichRings->At(iR);
		if (NULL == ring) continue;
		CbmTrackMatch* ringMatch = (CbmTrackMatch*) fRichRingMatches->At(iR);
		if (NULL == ringMatch) continue;

		Int_t mcTrackId = ringMatch->GetMCTrackId();
		if (mcTrackId < 0) continue;
		CbmMCTrack* mcTrack = (CbmMCTrack*)fMCTracks->At(mcTrackId);
		if (!mcTrack) continue;
		Int_t motherId = mcTrack->GetMotherId();
		Int_t pdg = TMath::Abs(mcTrack->GetPdgCode());
		Double_t momentum = mcTrack->GetP();
      Double_t pt = mcTrack->GetPt();
      Double_t rapidity = mcTrack->GetRapidity();


		if (ring->GetNofHits() >= fMinNofHits){
	      if (pdg == 11 && motherId == -1){
	         fh_acc_mom_el->Fill(momentum);
	         fh_acc_pty_el->Fill(rapidity, pt);
	      }
	      if (pdg == 211 && motherId == -1){
            fh_acc_mom_pi->Fill(momentum);
            fh_acc_pty_pi->Fill(rapidity, pt);
	      }
		}

		if (pdg != 11 || motherId != -1) continue; // only primary electrons

		CbmRichRingLight ringPoint;
		int nofRichPoints = fRichPoints->GetEntriesFast();
		for (int iPoint = 0; iPoint < nofRichPoints; iPoint++){
		   CbmRichPoint* richPoint = (CbmRichPoint*) fRichPoints->At(iPoint);
		   if (NULL == richPoint) continue;
		   Int_t trackId = richPoint->GetTrackID();
		   if (trackId < 0) continue;
	      CbmMCTrack* mcTrackRich = (CbmMCTrack*)fMCTracks->At(trackId);
	      if (NULL == mcTrackRich) continue;
	      int motherIdRich = mcTrackRich->GetMotherId();
		   if (motherIdRich == mcTrackId){
		      TVector3 posPoint;
		      richPoint->Position(posPoint);
		      TVector3 detPoint;
		      //cout << "phi= " << fPhi << " fTheta = " << fTheta << " fDetZOrig = " << fDetZOrig << endl;
		      CbmRichHitProducer::TiltPoint(&posPoint, &detPoint, fPhi, fTheta, fDetZOrig);
		      CbmRichHitLight hit(detPoint.X(), detPoint.Y());
		      ringPoint.AddHit(hit);
		   }
		}
	   fhNofHitsAll->Fill(ring->GetNofHits());

	   CbmRichRingLight ringHit;
	   CbmRichConverter::CopyHitsToRingLight(ring, &ringHit);

      FitAndFillHistCircle(0, &ringHit, momentum); //hits
      FitAndFillHistCircle(1, &ringPoint, momentum); // points
      FillMcVsHitFitCircle(&ringHit, &ringPoint);

      double r = ringHit.GetRadius();
      double xc = ringHit.GetCenterX();
      double yc = ringHit.GetCenterY();

      if (ringHit.GetRadius() > fMinRadius && ringHit.GetRadius() < fMaxRadius){
         fhNofHitsCircleFit->Fill(ringHit.GetNofHits());
      }
	   if (fNofDrawnRings < 10 && ringHit.GetNofHits() <= 500){
	      DrawRing(&ringHit, &ringPoint);
	   }

		FitAndFillHistEllipse(0, &ringHit, momentum);// hits
		FitAndFillHistEllipse(1, &ringPoint, momentum); // points
		FillMcVsHitFitEllipse(&ringHit, &ringPoint);

	   if (ringHit.GetAaxis() > fMinAaxis && ringHit.GetAaxis() < fMaxAaxis
	       &&  ringHit.GetBaxis() > fMinBaxis && ringHit.GetAaxis() < fMaxBaxis ){
	      fhNofHitsEllipseFit->Fill(ringHit.GetNofHits());

         double np = ringPoint.GetNofHits();
         double a = ringHit.GetAaxis();
         double b = ringHit.GetBaxis();
         double nh = ring->GetNofHits();

         fhNofHitsXYZ->Fill(xc, yc, nh);
         fhNofPointsXYZ->Fill(xc, yc, np);
         fhBoverAXYZ->Fill(xc, yc, b/a);
         fhBaxisXYZ->Fill(xc, yc, b);
         fhAaxisXYZ->Fill(xc, yc, a);
         fhRadiusXYZ->Fill(xc, yc, r);

         for (int iH = 0; iH < ringHit.GetNofHits(); iH++){
            double xh = ringHit.GetHit(iH).fX;
            double yh = ringHit.GetHit(iH).fY;
            double dr = r - sqrt((xc - xh)*(xc - xh) + (yc - yh)*(yc - yh));
            fhdRXYZ->Fill(xc, yc, dr);
         }
	   }
	}// iR
}

void CbmRichGeoTest::FitAndFillHistEllipse(
      int histIndex,
      CbmRichRingLight* ring,
      double momentum)
{
   fTauFit->DoFit(ring);
   double axisA = ring->GetAaxis();
   double axisB = ring->GetBaxis();
   double xcEllipse = ring->GetCenterX();
   double ycEllipse = ring->GetCenterY();
   int nofHitsRing = ring->GetNofHits();
   if (axisA > fMinAaxis && axisA < fMaxAaxis &&  axisB > fMinBaxis && axisB < fMaxBaxis ){
      fhBoverA[histIndex]->Fill(axisB/axisA);
      fhXcYcEllipse[histIndex]->Fill(xcEllipse, ycEllipse);
   }
   fhNofHits[histIndex]->Fill(nofHitsRing);
   fhBaxisVsMom[histIndex]->Fill(momentum, axisB);
   fhAaxisVsMom[histIndex]->Fill(momentum, axisA);
   fhChi2EllipseVsMom[histIndex]->Fill(momentum, ring->GetChi2()/ring->GetNofHits());
   if (histIndex == 0){ // only hit fit
      fhAaxisVsNofHits->Fill(nofHitsRing, axisA);
      fhBaxisVsNofHits->Fill(nofHitsRing, axisB);
   }
}

void CbmRichGeoTest::FitAndFillHistCircle(
      int histIndex,
      CbmRichRingLight* ring,
      double momentum)
{
   fCopFit->DoFit(ring);
   double radius = ring->GetRadius();
   double xcCircle = ring->GetCenterX();
   double ycCircle = ring->GetCenterY();
   int nofHitsRing = ring->GetNofHits();
   fhXcYcCircle[histIndex]->Fill(xcCircle, ycCircle);
   fhRadiusVsMom[histIndex]->Fill(momentum, radius);
   fhChi2CircleVsMom[histIndex]->Fill(momentum, ring->GetChi2()/ring->GetNofHits());

   for (int iH = 0; iH < ring->GetNofHits(); iH++){
      double xh = ring->GetHit(iH).fX;
      double yh = ring->GetHit(iH).fY;
      double dr = radius - sqrt((xcCircle - xh)*(xcCircle - xh) + (ycCircle - yh)*(ycCircle - yh));
      fhDRVsMom[histIndex]->Fill(momentum, dr);

      //if (histIndex == 0) {
      //   fhdRXYZ->Fill(xcCircle, ycCircle, dr);
      //}
   }

   if (histIndex == 0){ // only hit fit
      fhRadiusVsNofHits->Fill(nofHitsRing, radius);
   }
}

void CbmRichGeoTest::FillMcVsHitFitEllipse(
      CbmRichRingLight* ring,
      CbmRichRingLight* ringMc)
{
   fhDiffAaxis->Fill(ring->GetNofHits(), ringMc->GetAaxis() - ring->GetAaxis());
   fhDiffBaxis->Fill(ring->GetNofHits(),ringMc->GetBaxis() - ring->GetBaxis());
   fhDiffXcEllipse->Fill(ring->GetNofHits(),ringMc->GetCenterX() - ring->GetCenterX());
   fhDiffYcEllipse->Fill(ring->GetNofHits(),ringMc->GetCenterY() - ring->GetCenterY());
}

void CbmRichGeoTest::FillMcVsHitFitCircle(
      CbmRichRingLight* ring,
      CbmRichRingLight* ringMc)
{
   fhDiffXcCircle->Fill(ring->GetNofHits(),ringMc->GetCenterX() - ring->GetCenterX());
   fhDiffYcCircle->Fill(ring->GetNofHits(),ringMc->GetCenterY() - ring->GetCenterY());
   fhDiffRadius->Fill(ring->GetNofHits(),ringMc->GetRadius() - ring->GetRadius());
}

void CbmRichGeoTest::HitsAndPoints()
{
   Int_t nofHits = fRichHits->GetEntriesFast();
   for (Int_t iH = 0; iH < nofHits; iH++){
      CbmRichHit *hit = (CbmRichHit*) fRichHits->At(iH);
      if ( hit == NULL ) continue;
      Int_t pointInd =  hit->GetRefId();
      if (pointInd < 0) continue;
      CbmRichPoint *point = (CbmRichPoint*) fRichPoints->At(pointInd);
      if ( point == NULL ) continue;

      fhNofPhotonsPerHit->Fill(hit->GetNPhotons());

      TVector3 inPos(point->GetX(), point->GetY(), point->GetZ());
      TVector3 outPos;
      CbmRichHitProducer::TiltPoint(&inPos, &outPos, fPhi, fTheta, fDetZOrig);
      fhHitsXY->Fill(hit->GetX(), hit->GetY());
      fhDiffXhit->Fill(hit->GetX() - outPos.X());
      fhDiffYhit->Fill(hit->GetY() - outPos.Y());
   }

   Int_t nofPoints = fRichPoints->GetEntriesFast();
   for (Int_t iP = 0; iP < nofPoints; iP++){
      CbmRichPoint *point = (CbmRichPoint*) fRichPoints->At(iP);
      if ( point == NULL ) continue;
      TVector3 inPos(point->GetX(), point->GetY(), point->GetZ());
      TVector3 outPos;
      CbmRichHitProducer::TiltPoint(&inPos, &outPos, fPhi, fTheta, fDetZOrig);
      fhPointsXY->Fill(outPos.X(), outPos.Y());
   }
}

void CbmRichGeoTest::DrawRing(
      CbmRichRingLight* ringHit,
      CbmRichRingLight* ringPoint)
{
   stringstream ss;
   ss << "Event" << fNofDrawnRings;
   fNofDrawnRings++;
   TCanvas *c = CreateCanvas(ss.str().c_str(), ss.str().c_str(), 500, 500);
   c->SetGrid(true, true);
   TH2D* pad = new TH2D("pad", ";X [cm];Y [cm]", 1, -15., 15., 1, -15., 15);
   pad->SetStats(false);
   pad->Draw();

   // find min and max x and y positions of the hits
   // in order to shift drawing
   double xmin = 99999., xmax = -99999., ymin = 99999., ymax = -99999.;
   for (int i = 0; i < ringHit->GetNofHits(); i++){
      double hitX = ringHit->GetHit(i).fX;
      double hitY = ringHit->GetHit(i).fY;
      if (xmin > hitX) xmin = hitX;
      if (xmax < hitX) xmax = hitX;
      if (ymin > hitY) ymin = hitY;
      if (ymax < hitY) ymax = hitY;
   }
   double xCur = (xmin + xmax) / 2.;
   double yCur = (ymin + ymax) / 2.;

   //Draw circle and center
   TEllipse* circle = new TEllipse(ringHit->GetCenterX() - xCur,
         ringHit->GetCenterY() - yCur, ringHit->GetRadius());
   circle->SetFillStyle(0);
   circle->SetLineWidth(3);
   circle->Draw();
   TEllipse* center = new TEllipse(ringHit->GetCenterX() - xCur, ringHit->GetCenterY() - yCur, .5);
   center->Draw();

   // Draw hits
   for (int i = 0; i < ringHit->GetNofHits(); i++){
      TEllipse* hitDr = new TEllipse(ringHit->GetHit(i).fX - xCur, ringHit->GetHit(i).fY - yCur, .5);
      hitDr->SetFillColor(kRed);
      hitDr->Draw();
   }

   // Draw MC Points
   for (int i = 0; i < ringPoint->GetNofHits(); i++){
      TEllipse* pointDr = new TEllipse(ringPoint->GetHit(i).fX - xCur, ringPoint->GetHit(i).fY - yCur, 0.15);
      pointDr->SetFillColor(kBlue);
      pointDr->Draw();
   }

   //Draw information
   stringstream ss2;
   ss2 << "(r, n)=(" << setprecision(3) << ringHit->GetRadius() << ", " << ringHit->GetNofHits()<<")";
   TLatex* latex = new TLatex(-8., 8., ss2.str().c_str());
   latex->Draw();
}

TH1D* CbmRichGeoTest::CreateAccVsMinNofHitsHist()
{
   Int_t nofMc = (Int_t)fh_mc_mom_el->GetEntries();
   TH1D* hist = (TH1D*)fhNofHits[0]->Clone("fhAccVsMinNofHitsHist");
   hist->GetXaxis()->SetTitle("Required min nof hits in ring");
   hist->GetYaxis()->SetTitle("Detector acceptance [%]");
   Double_t sum = 0.;
   for (int i = hist->GetNbinsX(); i > 1 ; i--){
      sum += fhNofHits[0]->GetBinContent(i);
      hist->SetBinContent(i, 100. * sum / nofMc);
   }
   return hist;
}

void CbmRichGeoTest::CreateH2MeanRms(
      TH2D* hist,
      TH1D** meanHist,
      TH1D** rmsHist)
{
   *meanHist = (TH1D*)hist->ProjectionX( (string(hist->GetName()) + "_mean").c_str() )->Clone();
   (*meanHist)->GetYaxis()->SetTitle( ("Mean and RMS. " + string(hist->GetYaxis()->GetTitle()) ).c_str());
   *rmsHist = (TH1D*)hist->ProjectionX((string(hist->GetName()) + "_rms").c_str() )->Clone();
   (*rmsHist)->GetYaxis()->SetTitle( ("RMS. "+ string(hist->GetYaxis()->GetTitle()) ).c_str());
   for (Int_t i = 1; i <= hist->GetXaxis()->GetNbins(); i++){
      stringstream ss;
      ss << string(hist->GetName()) << "_py" << i;
      TH1D* pr = hist->ProjectionY(ss.str().c_str(), i, i);
      if (*meanHist == NULL || pr == NULL) continue;
      (*meanHist)->SetBinContent(i, pr->GetMean());
      (*meanHist)->SetBinError(i, pr->GetRMS());
      (*rmsHist)->SetBinContent(i, pr->GetRMS());
   }
}

void CbmRichGeoTest::DrawH2MeanRms(
      TH2D* hist,
      const string& canvasName)
{
   TH1D* mean, *rms;
   CreateH2MeanRms(hist, &mean, &rms);
  // TCanvas *c = CreateCanvas(canvasName.c_str(), canvasName.c_str(), 900, 900);
 /*  c->Divide(2, 2);
   c->cd(1);
   DrawH2(hist);
   c->cd(2);
   TH1D* py = hist->ProjectionY( (string(hist->GetName())+ "_py" ).c_str() );
   py->Scale(1./py->Integral());
   py->GetYaxis()->SetTitle("Yield");
   DrawH1(py);
   string txt1 = lit::NumberToString<Double_t>(py->GetMean(), 2) + " / "
         + lit::NumberToString<Double_t>(py->GetRMS(), 2);
   TLatex text;
   text.SetTextAlign(21);
   text.SetTextSize(0.05);
   text.DrawTextNDC(0.5, 0.92, txt1.c_str());
   gPad->SetLogy(true);
   c->cd(3);
   DrawH1(mean);
   c->cd(4);
   DrawH1(rms);*/

   TCanvas *c1 = CreateCanvas((canvasName+"_py").c_str(), (canvasName+"_py").c_str(), 800, 800);
   TH1D* py = hist->ProjectionY( (string(hist->GetName())+ "_py" ).c_str() );
   py->Scale(1./py->Integral());
   py->GetYaxis()->SetTitle("Yield");
   DrawH1(py);
   string txt1 = lit::NumberToString<Double_t>(py->GetMean(), 2) + " / " + lit::NumberToString<Double_t>(py->GetRMS(), 2);
   TLatex text;
   text.SetTextAlign(21);
   text.SetTextSize(0.05);
   text.DrawTextNDC(0.5, 0.92, txt1.c_str());
   gPad->SetLogy(true);

   TCanvas *c2 = CreateCanvas((canvasName+"_mean").c_str(), (canvasName+"_mean").c_str(), 800, 800);
   DrawH1(mean);

   TCanvas *c3 = CreateCanvas((canvasName+"_rms").c_str(), (canvasName+"_rms").c_str(), 800, 800);
   DrawH1(rms);
}

void CbmRichGeoTest::DrawH1andFit(
      TH1D* hist)
{
  // TH1D* py = hist->ProjectionY( (string(hist->GetName())+ "_py" ).c_str() );
   hist->GetYaxis()->SetTitle("Yield");
   DrawH1(hist);
   hist->Scale(1./ hist->Integral());
   hist->GetXaxis()->SetRangeUser(2., 8.);
   hist->Fit("gaus", "Q");
   TF1* func = hist->GetFunction("gaus");
   if (func == NULL) return;
   func->SetLineColor(kBlack);
   double m = func->GetParameter(1);
   double s = func->GetParameter(2);
   string txt1 = lit::NumberToString<double>(m, 2) + " / " + lit::NumberToString<double>(s, 2);
   TLatex text;
   text.SetTextAlign(21);
   text.SetTextSize(0.06);
   text.DrawTextNDC(0.5, 0.92, txt1.c_str());
  // gPad->SetLogy(true);
}

void CbmRichGeoTest::FitH1OneOverX(
      TH1D* hist,
      double xMinFit,
      double xMaxFit,
      double xMin,
      double xMax,
      const string& canvasName)
{
   for (int i = 1; i < hist->GetNbinsX() - 1;i++){
      double c0 = hist->GetBinContent(i);
      double c1 = hist->GetBinContent(i+1);
      if (c0 > 0 && c0 <= c1){
         xMinFit = hist->GetBinCenter(i+1);
      }
      if (c0 > c1) {
         break;
      }
   }
   TCanvas *cDrRmsFit = CreateCanvas(canvasName.c_str(), canvasName.c_str(), 600, 600);
   TF1 *f1 = new TF1("f1", "[0]/x+[1]", xMinFit, xMaxFit);
   f1->SetLineColor(kBlack);
   DrawH1(hist);
   hist->Fit(f1, "RQ");
   hist->GetXaxis()->SetRangeUser(xMin, xMax);
   f1->SetLineColor(kBlack);

   double p0 = f1->GetParameter(0);
   double p1 = f1->GetParameter(1);
   stringstream ss;
   ss.precision(3);
   ss << "y="<<lit::NumberToString(p0, 1)<<"/P+"<<lit::NumberToString(p1, 1);
   TLegend* leg = new TLegend(0.15, 0.9, 0.85, 0.99);
   leg->AddEntry(new TH2D(), ss.str().c_str(), "");
   leg->SetFillColor(kWhite);
   leg->SetFillStyle(0);
   leg->SetBorderSize(0);
   leg->Draw();
}

void CbmRichGeoTest::DrawHist()
{
   SetDefaultDrawStyle();

   TCanvas *cHitsXY = CreateCanvas("richgeo_hits_xy", "richgeo_hits_xy", 800, 800);
   DrawH2(fhHitsXY);

   TCanvas *cPointsXY = CreateCanvas("richgeo_points_xy", "richgeo_points_xy", 800, 800);
   DrawH2(fhPointsXY);

   for (int i = 0; i < 2; i++){
      stringstream ss;
      if (i == 0) ss << "richgeo_hits_";
      if (i == 1) ss << "richgeo_points_";
      TCanvas *cEllipseBoa = CreateCanvas((ss.str()+"ellipse_boa").c_str(), (ss.str()+"ellipse_boa").c_str(), 800, 800);
      fhBoverA[i]->Scale(1./fhBoverA[i]->Integral());
      DrawH1(fhBoverA[i]);
      cout << (ss.str()+"ellipse_boa_mean") << fhBoverA[i]->GetMean() << endl;
      gPad->SetLogy(true);

      TCanvas *cEllipseXcYc = CreateCanvas((ss.str()+"ellipse_xc_yc").c_str(), (ss.str()+"ellipse_xc_yc").c_str(), 800, 800);
      DrawH2(fhXcYcEllipse[i]);

      DrawH2MeanRms(fhChi2EllipseVsMom[i],  ss.str() + "chi2_ellipse_vs_mom");
      DrawH2MeanRms(fhAaxisVsMom[i], ss.str() + "a_vs_mom");
      DrawH2MeanRms(fhBaxisVsMom[i], ss.str() + "b_vs_mom");

      TCanvas *cCircle = CreateCanvas((ss.str()+"circle").c_str(), (ss.str()+"circle").c_str(), 800, 400);
      cCircle->Divide(2,1);
      cCircle->cd(1);
      DrawH1(fhNofHits[i]);
      cout << "Number of hits/points = "  << fhNofHits[i]->GetMean() << endl;
      gPad->SetLogy(true);
      cCircle->cd(2);
      DrawH2(fhXcYcCircle[i]);

      DrawH2MeanRms(fhChi2CircleVsMom[i], ss.str() + "chi2_circle_vs_mom");
      DrawH2MeanRms(fhRadiusVsMom[i], ss.str() + "r_vs_mom");
      DrawH2MeanRms(fhDRVsMom[i], ss.str() + "dr_vs_mom");

      TH1D* meanDr, *rmsDr;
      CreateH2MeanRms(fhDRVsMom[i], &meanDr, &rmsDr);
      FitH1OneOverX(rmsDr, 0.1, 2.9, 0.0, 3.0, ss.str() + "dr_rms_vs_mom_fit");

      TH1D* meanChi2, *rmsChi2;
      CreateH2MeanRms(fhChi2CircleVsMom[i], &meanChi2, &rmsChi2);
      FitH1OneOverX(meanChi2, 0.1, 2.9, 0.0, 3.0, ss.str() + "chi2_circle_mean_vs_mom_fit");
   }

   TCanvas* cNofPhotons = CreateCanvas("richgeo_nof_photons_per_hit", "richgeo_nof_photons_per_hit", 800, 800);
   fhNofPhotonsPerHit->Scale(1./fhNofPhotonsPerHit->Integral());
   DrawH1(fhNofPhotonsPerHit);

   TCanvas *cDiff2DEllipse = CreateCanvas("richgeo_diff2d_ellipse", "richgeo_diff2d_ellipse", 800, 800);
   cDiff2DEllipse->Divide(2,2);
   cDiff2DEllipse->cd(1);
   DrawH2(fhDiffAaxis);
   cDiff2DEllipse->cd(2);
   DrawH2(fhDiffBaxis);
   cDiff2DEllipse->cd(3);
   DrawH2(fhDiffXcEllipse);
   cDiff2DEllipse->cd(4);
   DrawH2(fhDiffYcEllipse);

   TCanvas *cDiff2DCircle = CreateCanvas("richgeo_diff2d_circle", "richgeo_diff2d_circle", 1200, 400);
   cDiff2DCircle->Divide(3,1);
   cDiff2DCircle->cd(1);
   DrawH2(fhDiffXcCircle);
   cDiff2DCircle->cd(2);
   DrawH2(fhDiffYcCircle);
   cDiff2DCircle->cd(3);
   DrawH2(fhDiffRadius);

   TCanvas *cDiff1DEllipse = CreateCanvas("richgeo_diff1d_ellipse", "richgeo_diff1d_ellipse", 800, 800);
   cDiff1DEllipse->Divide(2,2);
   cDiff1DEllipse->cd(1);
   DrawH1(fhDiffAaxis->ProjectionY());
   gPad->SetLogy(true);
   cDiff1DEllipse->cd(2);
   DrawH1(fhDiffBaxis->ProjectionY());
   gPad->SetLogy(true);
   cDiff1DEllipse->cd(3);
   DrawH1(fhDiffXcEllipse->ProjectionY());
   gPad->SetLogy(true);
   cDiff1DEllipse->cd(4);
   DrawH1(fhDiffYcEllipse->ProjectionY());
   gPad->SetLogy(true);

   TCanvas *cDiff1DCircle = CreateCanvas("richgeo_diff1d_circle", "richgeo_diff1d_circle", 1200, 400);
   cDiff1DCircle->Divide(3,1);
   cDiff1DCircle->cd(1);
   DrawH1(fhDiffXcCircle->ProjectionY());
   gPad->SetLogy(true);
   cDiff1DCircle->cd(2);
   DrawH1(fhDiffYcCircle->ProjectionY());
   gPad->SetLogy(true);
   cDiff1DCircle->cd(3);
   DrawH1(fhDiffRadius->ProjectionY());
   gPad->SetLogy(true);

   TCanvas *cHits = CreateCanvas("richgeo_hits", "richgeo_hits", 1200, 600);
   cHits->Divide(2,1);
   cHits->cd(1);
   DrawH1(fhDiffXhit);
   cHits->cd(2);
   DrawH1(fhDiffYhit);

   TCanvas *cFitEff = CreateCanvas("richgeo_fit_eff", "richgeo_fit_eff", 1200, 400);
   cFitEff->Divide(3,1);
   cFitEff->cd(1);
   DrawH1( list_of((TH1D*)fhNofHitsAll->Clone())((TH1D*)fhNofHitsCircleFit->Clone())((TH1D*)fhNofHitsEllipseFit->Clone()),
         list_of("All")("Circle fit")("Ellipse fit"), kLinear, kLog, true, 0.7, 0.7, 0.99, 0.99);
   fhNofHitsCircleFitEff = DivideH1(fhNofHitsCircleFit, fhNofHitsAll, "", "", "Nof hits in ring", "Efficiency");
   fhNofHitsEllipseFitEff = DivideH1(fhNofHitsEllipseFit, fhNofHitsAll, "", "", "Nof hits in ring", "Efficiency");
   cFitEff->cd(2);
   DrawH1(fhNofHitsCircleFitEff);
   TLatex* circleFitEffTxt = new TLatex(15, 0.5, CalcEfficiency(fhNofHitsCircleFit, fhNofHitsAll).c_str());
   circleFitEffTxt->Draw();
   cFitEff->cd(3);
   DrawH1(fhNofHitsEllipseFitEff);
   TLatex* ellipseFitEffTxt = new TLatex(15, 0.5, CalcEfficiency(fhNofHitsEllipseFit, fhNofHitsAll).c_str());
   ellipseFitEffTxt->Draw();

   TCanvas *cAccEff = CreateCanvas("richgeo_acc_eff_el", "richgeo_acc_eff_el", 1200, 800);
   cAccEff->Divide(3,2);
   cAccEff->cd(1);
   DrawH1(list_of(fh_mc_mom_el)(fh_acc_mom_el), list_of("MC")("ACC"), kLinear, kLog, true, 0.8, 0.8, 0.99, 0.99);
   cAccEff->cd(2);
   DrawH2(fh_mc_pty_el);
   cAccEff->cd(3);
   DrawH2(fh_acc_pty_el);


   TH1D* pxEff = DivideH1((TH1D*)fh_acc_mom_el->Clone(), (TH1D*)fh_mc_mom_el->Clone(), "pxEff", "", "p [GeV/c]", "Geometrical acceptance [%]");
   TH2D* pyzEff = DivideH2((TH1D*)fh_acc_pty_el->Clone(), (TH1D*)fh_mc_pty_el->Clone(), "pyzEff", "", "Rapidity", "P_{t} [GeV/c]", "Geometrical acceptance [%]");
   //cAccEff->cd(4);
   TCanvas *cAccEff_mom = CreateCanvas("richgeo_acc_eff_el_mom", "richgeo_acc_eff_el_mom", 800, 800);
   DrawH1(pxEff);
   //TLatex* pxEffTxt = new TLatex(3, 0.5, CalcEfficiency(fh_acc_mom_el, fh_mc_mom_el).c_str());
   //pxEffTxt->Draw();
   TCanvas *cAccEff_pty = CreateCanvas("richgeo_acc_eff_el_pty", "richgeo_acc_eff_el_pty", 800, 800);
   //cAccEff->cd(5);
   DrawH2(pyzEff);

   TCanvas *cAccEffPi = CreateCanvas("richgeo_acc_eff_pi", "richgeo_acc_eff_pi", 1200, 800);
   cAccEffPi->Divide(3,2);
   cAccEffPi->cd(1);
   DrawH1(list_of(fh_mc_mom_pi)(fh_acc_mom_pi), list_of("MC")("ACC"), kLinear, kLog, true, 0.8, 0.8, 0.99, 0.99);
   cAccEffPi->cd(2);
   DrawH2(fh_mc_pty_pi);
   cAccEffPi->cd(3);
   DrawH2(fh_acc_pty_pi);
   TH1D* pxPiEff = DivideH1((TH1D*)fh_acc_mom_pi->Clone(), (TH1D*)fh_mc_mom_pi->Clone(), "pxPiEff", "", "p [GeV/c]", "Geometrical acceptance [%]");
   TH2D* pyzPiEff = DivideH2((TH1D*)fh_acc_pty_pi->Clone(), (TH1D*)fh_mc_pty_pi->Clone(), "pyzPiEff", "", "Rapidity", "P_{t} [GeV/c]", "Geometrical acceptance [%]");
  // cAccEffPi->cd(4);
   TCanvas *cAccEffPi_mom = CreateCanvas("richgeo_acc_eff_pi_mom", "richgeo_acc_eff_pi_mom", 800, 800);
   DrawH1(pxPiEff);
  // TLatex* pxEffTxt = new TLatex(3, 0.5, CalcEfficiency(fh_acc_mom_pi, fh_mc_mom_pi).c_str());
 //  pxEffTxt->Draw();
   //cAccEffPi->cd(5);
   TCanvas *cAccEffPi_pty = CreateCanvas("richgeo_acc_eff_pi_pty", "richgeo_acc_eff_pi_pty", 800, 800);
   DrawH2(pyzPiEff);
   pyzPiEff->GetZaxis()->SetRangeUser(0, 100);

   TCanvas *cAccEffElPi_mom = CreateCanvas("richgeo_acc_eff_el_pi_mom", "richgeo_acc_eff_el_pi_mom", 800, 800);
   DrawH1(list_of(pxEff)(pxPiEff), list_of("e^{#pm}")("#pi^{#pm}"), kLinear, kLinear, true, 0.7, 0.5, 0.87, 0.7);


   TCanvas *cAccEffZoom = CreateCanvas("richgeo_acc_eff_el_zoom", "richgeo_acc_eff_el_zoom", 1000, 500);
   cAccEffZoom->Divide(2,1);
   cAccEffZoom->cd(1);
   TH1D* fh_mc_mom_clone = (TH1D*)fh_mc_mom_el->Clone();
   TH1D* fh_acc_mom_clone = (TH1D*)fh_acc_mom_el->Clone();
   fh_mc_mom_clone->GetXaxis()->SetRangeUser(0., 3.);
   fh_acc_mom_clone->GetXaxis()->SetRangeUser(0., 3.);
   fh_mc_mom_clone->SetMinimum(0.);
   DrawH1(list_of(fh_mc_mom_clone)(fh_acc_mom_clone), list_of("MC")("ACC"), kLinear, kLog, true, 0.8, 0.8, 0.99, 0.99);
   gPad->SetLogy(false);
   cAccEffZoom->cd(2);
   TH1D* px_eff_clone = (TH1D*) pxEff->Clone();
   px_eff_clone->GetXaxis()->SetRangeUser(0., 3.);
   px_eff_clone->SetMinimum(0.);
   DrawH1(px_eff_clone);

   // Draw number vs position onto the photodetector plane
   DrawH3(fhNofHitsXYZ, "richgeo_numbers_vs_xy_hits", "Number of hits", 10, 30);
   DrawH3(fhNofPointsXYZ, "richgeo_numbers_vs_xy_points", "Number of points", 100., 300.);
   DrawH3(fhBoverAXYZ, "richgeo_numbers_vs_xy_boa", "B/A", 0.75, 1.0);
   DrawH3(fhBaxisXYZ, "richgeo_numbers_vs_xy_b", "B [cm]", 4., 5.);
   DrawH3(fhAaxisXYZ, "richgeo_numbers_vs_xy_a", "A [cm]", 4.4, 5.7);
   DrawH3(fhRadiusXYZ, "richgeo_numbers_vs_xy_r", "Radius [cm]", 4.2, 5.2);
   DrawH3(fhdRXYZ, "richgeo_numbers_vs_xy_dr", "dR [cm]", -.1, .1);

   TCanvas *cAccVsMinNofHits = CreateCanvas("richgeo_acc_vs_min_nof_hits", "richgeo_acc_vs_min_nof_hits", 600, 600);
   TH1D* h = CreateAccVsMinNofHitsHist();
   h->GetXaxis()->SetRangeUser(0., 40.0);
   DrawH1(h);

   DrawH2MeanRms(fhRadiusVsNofHits, "richgeo_hits_r_vs_nof_hits");
   DrawH2MeanRms(fhAaxisVsNofHits, "richgeo_hits_a_vs_nof_hits");
   DrawH2MeanRms(fhBaxisVsNofHits, "richgeo_hits_b_vs_nof_hits");

   TCanvas* cHitsRAB = CreateCanvas("richgeo_hits_rab", "richgeo_hits_rab", 1500, 600);
   cHitsRAB->Divide(3, 1);
   cHitsRAB->cd(1);
   DrawH1andFit(fhRadiusVsNofHits->ProjectionY( (string(fhRadiusVsNofHits->GetName())+ "_py" ).c_str() ));
   cHitsRAB->cd(2);
   DrawH1andFit(fhAaxisVsNofHits->ProjectionY( (string(fhAaxisVsNofHits->GetName())+ "_py" ).c_str() ));
   cHitsRAB->cd(3);
   DrawH1andFit(fhBaxisVsNofHits->ProjectionY( (string(fhBaxisVsNofHits->GetName())+ "_py" ).c_str() ));
}

void CbmRichGeoTest::DrawH3(
      TH3D* h,
      const string& cName,
      const string& zAxisTitle,
      double zMin,
      double zMax,
      bool doFit)
{
   int nBinsX = h->GetNbinsX();
   int nBinsY = h->GetNbinsY();
   TH2D* h2Mean = (TH2D*)h->Project3D("yx")->Clone();
   TH2D* h2Rms = (TH2D*)h->Project3D("yx")->Clone();

   TCanvas *canvas;
   if (doFit) canvas = new TCanvas ("fit_func", "fit_func");

   for (int x = 1; x <= nBinsX; x++){
      for (int y = 1; y <= nBinsY; y++){
         stringstream ss;
         ss << h->GetName() << "_z_" << x << "_" << y;
         TH1D* hz = h->ProjectionZ(ss.str().c_str(), x, x, y, y);
         double m = 0.;
         double s = 0.;
         if (doFit) {
            hz->Fit("gaus", "QO");
            TF1* func = hz->GetFunction("gaus");
            if (func != NULL) {
               m = func->GetParameter(1);
               s = func->GetParameter(2);
            }
         } else {
            m = hz->GetMean();
            s = hz->GetRMS();
         }
         h2Mean->SetBinContent(x, y, m);
         h2Rms->SetBinContent(x, y, s);
      }
   }

   TCanvas *mean = CreateCanvas(string(cName+"_mean").c_str(), string(cName+"_mean").c_str(), 800, 800);
   h2Mean->GetZaxis()->SetTitle(string("Mean."+zAxisTitle).c_str());
   h2Mean->GetZaxis()->SetRangeUser(zMin, zMax);
   DrawH2(h2Mean);
   TCanvas *rms = CreateCanvas(string(cName+"_rms").c_str(), string(cName+"_rms").c_str(), 800, 800);
   h2Rms->GetZaxis()->SetTitle(string("RMS."+zAxisTitle).c_str());
   DrawH2(h2Rms);


   TCanvas* cHitsRAB = CreateCanvas(string(cName+"_local_xy").c_str(), string(cName+"_local_xy").c_str(), 800, 800);
   int x = h->GetXaxis()->FindBin(20);
   int y = h->GetYaxis()->FindBin(120);
   cout << x << " " << y << endl;
   TH1D* hz = h->ProjectionZ((string(h->GetName())+"_local_xy").c_str(), x, x, y, y);
   //if (hz->GetEntries() > 2) {
      DrawH1andFit(hz);
   //}

   if (canvas != NULL) delete canvas;

}

void CbmRichGeoTest::CreateStudyReport(
      const string& title,
      const vector<string>& fileNames,
      const vector<string>& studyNames,
      const string& outputDir)
{
   if (outputDir != "") gSystem->mkdir(outputDir.c_str(), true);

   CbmStudyReport* report = new CbmRichGeoTestStudyReport();
   fTitle = title;
   cout << "Report can be found here: " << outputDir << endl;
   report->Create(fileNames, studyNames, outputDir);
   delete report;
}

void CbmRichGeoTest::Finish()
{
   DrawHist();
   for (Int_t i = 0; i < fHists.size(); i++){
   //   if (NULL != fHists[i]) fHists[i]->Write();
   }
   fhNofHits[0]->Write();
   fhNofHits[1]->Write();
   fhNofPhotonsPerHit->Write();
   SaveCanvasToImage();
}

string CbmRichGeoTest::CalcEfficiency(
   TH1* histRec,
   TH1* histAcc)
{
   if (histAcc->GetEntries() == 0) { return "0.0"; }
   else {
      Double_t eff = Double_t(histRec->GetEntries()) / Double_t(histAcc->GetEntries());
      stringstream ss;
      ss.precision(3);
      ss << eff;
      return ss.str();
   }
}


TH1D* CbmRichGeoTest::DivideH1(
   TH1D* h1,
   TH1D* h2,
   const string& name,
   const string& title,
   const string& axisX,
   const string& axisY)
{
   h1->Sumw2();
   h2->Sumw2();
   TH1D* h3 = new TH1D(name.c_str(), string(title +";"+axisX+";"+ axisY).c_str(),
                       h1->GetNbinsX(), h1->GetXaxis()->GetXmin(),h1->GetXaxis()->GetXmax());
   h3->Divide(h1, h2, 100., 1., "B");
   return h3;
}

TH2D* CbmRichGeoTest::DivideH2(
   TH1D* h1,
   TH1D* h2,
   const string& name,
   const string& title,
   const string& axisX,
   const string& axisY,
   const string& axisZ)
{
   h1->Sumw2();
   h2->Sumw2();
   TH2D* h3 = new TH2D(name.c_str(), string(title +";"+axisX+";"+ axisY+";"+axisZ).c_str(),
                       h1->GetNbinsX(), h1->GetXaxis()->GetXmin(),h1->GetXaxis()->GetXmax(),
                       h1->GetNbinsY(), h1->GetYaxis()->GetXmin(),h1->GetYaxis()->GetXmax());
   h3->Divide(h1, h2, 100., 1., "B");
   return h3;
}

TCanvas* CbmRichGeoTest::CreateCanvas(
      const string& name,
      const string& title,
      int width,
      int height)
{
   TCanvas* c = new TCanvas(name.c_str(), title.c_str(), width, height);
   fCanvas.push_back(c);
   return c;
}

void CbmRichGeoTest::SaveCanvasToImage()
{
   for (int i = 0; i < fCanvas.size(); i++)
   {
      lit::SaveCanvasAsImage(fCanvas[i], fOutputDir);
   }
}

ClassImp(CbmRichGeoTest)

