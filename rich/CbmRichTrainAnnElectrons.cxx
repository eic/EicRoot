/**
 * \file CbmRichTrainAnnelectrons.cxx
 *
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2011
 **/

#include "CbmRichTrainAnnElectrons.h"

#include "FairTrackParam.h"
#include "CbmGlobalTrack.h"
#include "CbmRichRing.h"
#include "CbmTrackMatch.h"
#include "CbmTrackMatch.h"
#include "FairRootManager.h"
#include "FairMCPoint.h"
#include "CbmMCTrack.h"
#include "CbmDrawHist.h"

#include "TString.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TClonesArray.h"
#include "TMultiLayerPerceptron.h"

#include <boost/assign/list_of.hpp>

#include <iostream>
#include <vector>
#include <cmath>
#include <string>

class CbmStsTrack;

using std::cout;
using std::endl;
using std::vector;
using std::fabs;
using std::string;
using boost::assign::list_of;

CbmRichTrainAnnElectrons::CbmRichTrainAnnElectrons():
   fEventNum(0),

   fRichHits(NULL),
   fRichRings(NULL),
   fRichPoints(NULL),
   fMCTracks(NULL),
   fRichRingMatches(NULL),
   fRichProj(NULL),
   fStsTrackMatches(NULL),
   fGlobalTracks(NULL),
   fStsTracks(NULL),

   fMinNofHitsInRichRing(7),
   fQuota(0.6),
   fMaxNofTrainSamples(5000),

   fNofPiLikeEl(0),
   fNofElLikePi(0),
   fAnnCut(-0.5),

   fhAnnOutput(),
   fhCumProb(),

   fRElIdParams(),

   fhAaxis(),
   fhBaxis(),
  // fhAaxisCor(),
  // fhBaxisCor(),
   fhDistTrueMatch(),
   fhDistMisMatch(),
   fhNofHits(),
   fhChi2(),
   fhRadPos(),
   fhAaxisVsMom(),
   fhBaxisVsMom(),
   fhPhiVsRadAng(),

   fHists()
{
   fhAaxis.resize(2);
   fhBaxis.resize(2);
 //  fhAaxisCor.resize(2);
 //  fhBaxisCor.resize(2);
   fhDistTrueMatch.resize(2);
   fhDistMisMatch.resize(2);
   fhNofHits.resize(2);
   fhChi2.resize(2);
   fhRadPos.resize(2);
   fhAaxisVsMom.resize(2);
   fhBaxisVsMom.resize(2);
   fhPhiVsRadAng.resize(2);
   fhAnnOutput.resize(2);
   fhCumProb.resize(2);
   fRElIdParams.resize(2);
   string ss;
   for (int i = 0; i < 2; i++){
      if (i == 0) ss = "Electron";
      if (i == 1) ss = "Pion";
      // difference between electrons and pions
      fhAaxis[i] = new TH1D(string("fhAaxis"+ss).c_str(), "fhAaxis;A axis [cm];Counter", 50, 0., 8.);
      fHists.push_back(fhAaxis[i]);
      fhBaxis[i] = new TH1D(string("fhBAxis"+ss).c_str(), "fhBAxis;B axis [cm];Counter", 50, 0., 8.);
      fHists.push_back(fhBaxis[i]);
     // fhAaxisCor[i] = new TH1D(string("fhAaxisCor"+ss).c_str(), "fhAaxisCor;A axis [cm];Counter", 30,3,8);
     // fHists.push_back(fhAaxisCor[i]);
     // fhBaxisCor[i] = new TH1D(string("fhBAxisCor"+ss).c_str(), "fhBAxisCor;B axis [cm];Counter", 30,3,8);
     // fHists.push_back(fhBaxisCor[i]);
      fhDistTrueMatch[i] = new TH1D(string("fhDistTrueMatch"+ss).c_str(), "fhDistTrueMatch;Ring-track distance [cm];Counter", 50, 0., 5.);
      fHists.push_back(fhDistTrueMatch[i]);
      fhDistMisMatch[i] = new TH1D(string("fhDistMisMatch"+ss).c_str(), "fhDistMisMatch;Ring-track distance [cm];Counter", 50, 0., 5.);
      fHists.push_back(fhDistMisMatch[i]);
      fhNofHits[i] = new TH1D(string("fhNofHits"+ss).c_str(), "fhNofHits;Number of hits;Counter", 50, 0, 50);
      fHists.push_back(fhNofHits[i]);
      fhChi2[i] = new TH1D(string("fhChi2"+ss).c_str(), "fhChi2;#Chi^{2};Counter", 100, 0., 1.);
      fHists.push_back(fhChi2[i]);
      fhRadPos[i] = new TH1D(string("fhRadPos"+ss).c_str(), "fhRadPos;Radial position [cm];Counter", 150, 0., 150.);
      fHists.push_back(fhRadPos[i]);
      fhAaxisVsMom[i] = new TH2D(string("fhAaxisVsMom"+ss).c_str(), "fhAaxisVsMom;P [GeV/c];A axis [cm]",30, 0, 15, 50, 0, 10);
      fHists.push_back(fhAaxisVsMom[i]);
      fhBaxisVsMom[i] = new TH2D(string("fhBAxisVsMom"+ss).c_str(), "fhBAxisVsMom;P [GeV/c];B axis [cm]",30, 0, 15, 50, 0, 10);
      fHists.push_back(fhBaxisVsMom[i]);
      fhPhiVsRadAng[i] = new TH2D(string("fhPhiVsRadAng"+ss).c_str(), "fhPhiVsRadAng;Phi [rad];Radial angle [rad]", 50, -2. ,2.,50, 0. , 6.3);
      fHists.push_back(fhPhiVsRadAng[i]);
      // ANN outputs
      fhAnnOutput[i] = new TH1D(string("fhAnnOutput"+ss).c_str(),"ANN output;ANN output;Counter",100, -1.2, 1.2);
      fHists.push_back(fhAnnOutput[i]);
      fhCumProb[i] = new TH1D(string("fhCumProb"+ss).c_str(),"ANN output;ANN output;Cumulative probability",100, -1.2, 1.2);
      fHists.push_back(fhCumProb[i]);
   }
}

CbmRichTrainAnnElectrons::~CbmRichTrainAnnElectrons()
{

}

InitStatus CbmRichTrainAnnElectrons::Init()
{
	cout << "InitStatus CbmRichTrainAnnElectrons::Init()"<<endl;

	FairRootManager* ioman = FairRootManager::Instance();
	if (NULL == ioman) { Fatal("CbmRichTrainAnnElectrons::Init","RootManager not instantised!");}

	fRichHits = (TClonesArray*) ioman->GetObject("RichHit");
	if ( NULL == fRichHits) { Fatal("CbmRichTrainAnnElectrons::Init","No RichHit array!");}

	fRichRings = (TClonesArray*) ioman->GetObject("RichRing");
	if ( NULL == fRichRings) { Fatal("CbmRichTrainAnnElectrons::Init","No RichRing array!");}

	fRichPoints = (TClonesArray*) ioman->GetObject("RichPoint");
	if ( NULL == fRichPoints) { Fatal("CbmRichTrainAnnElectrons::Init","No RichPoint array!");}

	fMCTracks = (TClonesArray*) ioman->GetObject("MCTrack");
	if ( NULL == fMCTracks) { Fatal("CbmRichTrainAnnElectrons::Init","No MCTrack array!");}

	fRichRingMatches = (TClonesArray*) ioman->GetObject("RichRingMatch");
	if ( NULL == fRichRingMatches) { Fatal("CbmRichTrainAnnElectrons::Init","No RichRingMatch array!");}

	fRichProj = (TClonesArray*) ioman->GetObject("RichProjection");
	if ( NULL == fRichProj) { Fatal("CbmRichTrainAnnElectrons::Init","No RichProjection array!");}

	fStsTrackMatches = (TClonesArray*) ioman->GetObject("StsTrackMatch");
	if ( NULL == fStsTrackMatches) { Fatal("CbmRichTrainAnnElectrons::Init","No track match array!");}

	fGlobalTracks = (TClonesArray*) ioman->GetObject("GlobalTrack");
	if ( NULL == fGlobalTracks) { Fatal("CbmRichTrainAnnElectrons::Init","No global track array!");}

	fStsTracks = (TClonesArray*) ioman->GetObject("StsTrack");
	if ( NULL == fStsTracks) { Fatal("CbmRichTrainAnnElectrons::Init","No  STSTrack array!");}

	return kSUCCESS;
}

void CbmRichTrainAnnElectrons::Exec(
      Option_t* option)
{
   cout << endl <<"-I- CbmRichTrainAnnElectrons, event " << fEventNum << endl;
   DiffElandPi();
	fEventNum++;
   cout <<"Nof Electrons = " << fRElIdParams[0].size() << endl;
   cout <<"Nof Pions = " << fRElIdParams[1].size() << endl;
}

void CbmRichTrainAnnElectrons::DiffElandPi()
{
	Int_t nGlTracks = fGlobalTracks->GetEntriesFast();

	for (Int_t iTrack=0; iTrack < nGlTracks; iTrack++) {
      CbmGlobalTrack* gTrack = (CbmGlobalTrack*)fGlobalTracks->At(iTrack);
      if (NULL == gTrack) continue;
      Int_t stsIndex = gTrack->GetStsTrackIndex();
      if (stsIndex == -1) continue;
      CbmStsTrack* stsTrack = (CbmStsTrack*)fStsTracks->At(stsIndex);
      if (NULL == stsTrack) continue;
      CbmTrackMatch* stsTrackMatch = (CbmTrackMatch*)fStsTrackMatches->At(stsIndex);
      if (NULL == stsTrackMatch) continue;
      Int_t mcIdSts = stsTrackMatch->GetMCTrackId();

      Int_t richIndex = gTrack->GetRichRingIndex();
      if (richIndex == -1) continue;
      CbmRichRing* ring = (CbmRichRing*)fRichRings->At(richIndex);
      if (NULL == ring) continue;
      CbmTrackMatch* richRingMatch = (CbmTrackMatch*)fRichRingMatches->At(richIndex);
      if (NULL == richRingMatch) continue;
      Int_t mcIdRich = richRingMatch->GetMCTrackId();

      CbmMCTrack* track = (CbmMCTrack*) fMCTracks->At(mcIdSts);
      if (NULL == track) continue;
      Int_t pdg = TMath::Abs(track->GetPdgCode());
      Int_t motherId = track->GetMotherId();
      Double_t momentum = track->GetP();

      Double_t axisACor = ring->GetAaxisCor();
      Double_t axisBCor= ring->GetBaxisCor();

      Int_t lFoundHits = richRingMatch->GetNofTrueHits() + richRingMatch->GetNofWrongHits()
            + richRingMatch->GetNofFakeHits();
      Double_t lPercTrue = 0;
      if (lFoundHits >= 3){
         lPercTrue = (Double_t)richRingMatch->GetNofTrueHits() / (Double_t)lFoundHits;
      }
      Bool_t isTrueFound = (lPercTrue > fQuota);

      RingElectronParam p;
      p.fAaxis = ring->GetAaxis();
      p.fBaxis = ring->GetBaxis();
      p.fPhi = ring->GetPhi();
      p.fRadAngle = ring->GetRadialAngle();
      p.fChi2 = ring->GetChi2()/ring->GetNDF();
      p.fRadPos = ring->GetRadialPosition();
      p.fNofHits = ring->GetNofHits();
      p.fDistance = ring->GetDistance();
      p.fMomentum = momentum;

      // electrons
      if (pdg == 11 && motherId == -1 && isTrueFound &&
            mcIdSts == mcIdRich && mcIdRich != -1){
         fhAaxis[0]->Fill(p.fAaxis);
         fhBaxis[0]->Fill(p.fBaxis);
        // fhAaxisCor[0]->Fill(axisACor);
        // fhBaxisCor[0]->Fill(axisBCor);
         fhDistTrueMatch[0]->Fill(p.fDistance);
         fhNofHits[0]->Fill(p.fNofHits);
         fhChi2[0]->Fill(p.fChi2);
         fhRadPos[0]->Fill(p.fRadPos);
         fhAaxisVsMom[0]->Fill(momentum, p.fAaxis);
         fhBaxisVsMom[0]->Fill(momentum, p.fBaxis);
         fhPhiVsRadAng[0]->Fill(p.fPhi, p.fRadAngle);
         fRElIdParams[0].push_back(p);
      }

      if (pdg == 11 && motherId == -1 && isTrueFound &&
            mcIdSts != mcIdRich && mcIdRich != -1){
         fhDistMisMatch[0]->Fill(p.fDistance);
      }


      // pions
      if ( pdg == 211 &&  mcIdRich != -1){
         fhAaxis[1]->Fill(p.fAaxis);
         fhBaxis[1]->Fill(p.fBaxis);
        // fhAaxisCor[1]->Fill(axisACor);
        // fhBaxisCor[1]->Fill(axisBCor);
         if (mcIdRich == mcIdSts) {
            fhDistTrueMatch[1]->Fill(p.fDistance);
            fhAaxisVsMom[1]->Fill(momentum, p.fAaxis);
            fhBaxisVsMom[1]->Fill(momentum, p.fBaxis);
         } else {
            fhDistMisMatch[1]->Fill(p.fDistance);
         }
         fhNofHits[1]->Fill(p.fNofHits);
         fhChi2[1]->Fill(p.fChi2);
         fhRadPos[1]->Fill(p.fRadPos);
         fhPhiVsRadAng[1]->Fill(p.fPhi, p.fRadAngle);

         fRElIdParams[1].push_back(p);
      }
	}// global tracks
}

void CbmRichTrainAnnElectrons::TrainAndTestAnn()
{
   TTree *simu = new TTree ("MonteCarlo","MontecarloData");
   Double_t x[9];
   Double_t xOut;

   simu->Branch("x0", &x[0],"x0/D");
   simu->Branch("x1", &x[1],"x1/D");
   simu->Branch("x2", &x[2],"x2/D");
   simu->Branch("x3", &x[3],"x3/D");
   simu->Branch("x4", &x[4],"x4/D");
   simu->Branch("x5", &x[5],"x5/D");
   simu->Branch("x6", &x[6],"x6/D");
   simu->Branch("x7", &x[7],"x7/D");
   simu->Branch("x8", &x[8],"x8/D");
   simu->Branch("xOut", &xOut,"xOut/D");

   for (int j = 0; j < 2; j++){
      for (int i = 0; i < fRElIdParams[j].size(); i++){
         x[0] = fRElIdParams[j][i].fAaxis / 10.;
         x[1] = fRElIdParams[j][i].fBaxis / 10.;
         x[2] = (fRElIdParams[j][i].fPhi + 1.57) / 3.14;
         x[3] = fRElIdParams[j][i].fRadAngle / 6.28;
         x[4] = fRElIdParams[j][i].fChi2 / 1.2;
         x[5] = fRElIdParams[j][i].fRadPos / 110.;
         x[6] = fRElIdParams[j][i].fNofHits / 45.;
         x[7] = fRElIdParams[j][i].fDistance / 5.;
         x[8] = fRElIdParams[j][i].fMomentum / 15.;

         for (int k = 0; k < 9; k++){
            if (x[k] < 0.0) x[k] = 0.0;
            if (x[k] > 1.0) x[k] = 1.0;
         }

         if (j == 0) xOut = 1.;
         if (j == 1) xOut = -1.;
         simu->Fill();
         if (i >= fMaxNofTrainSamples) break;
      }
   }

   TMultiLayerPerceptron network("x0,x1,x2,x3,x4,x5,x6,x7,x8:18:xOut",simu,"Entry$+1");
   //network.LoadWeights("");
   network.Train(300,"text,update=10");
   network.DumpWeights("rich_elid_ann_weights.txt");
   //network.Export();

   Double_t params[9];

   fNofPiLikeEl = 0;
   fNofElLikePi = 0;

   for (int j = 0; j < 2; j++){
      for (int i = 0; i < fRElIdParams[j].size(); i++){
         params[0] = fRElIdParams[j][i].fAaxis / 10.;
         params[1] = fRElIdParams[j][i].fBaxis / 10.;
         params[2] = (fRElIdParams[j][i].fPhi + 1.57) / 3.14;
         params[3] = fRElIdParams[j][i].fRadAngle / 6.28;
         params[4] = fRElIdParams[j][i].fChi2 / 1.2;
         params[5] = fRElIdParams[j][i].fRadPos / 110.;
         params[6] = fRElIdParams[j][i].fNofHits / 45.;
         params[7] = fRElIdParams[j][i].fDistance / 5.;
         params[8] = fRElIdParams[j][i].fMomentum / 15.;

         for (int k = 0; k < 9; k++){
            if (params[k] < 0.0) params[k] = 0.0;
            if (params[k] > 1.0) params[k] = 1.0;
         }

        Double_t netEval = network.Evaluate(0,params);

        //if (netEval > maxEval) netEval = maxEval - 0.01;
       // if (netEval < minEval) netEval = minEval + 0.01;

        fhAnnOutput[j]->Fill(netEval);
        if (netEval >= fAnnCut && j == 1) fNofPiLikeEl++;
        if (netEval < fAnnCut && j == 0) fNofElLikePi++;
     }
   }
}

void CbmRichTrainAnnElectrons::Draw()
{
   cout <<"nof electrons = " << fRElIdParams[0].size() << endl;
   cout <<"nof pions = " << fRElIdParams[1].size() << endl;
   cout <<"Pions like electrons = " << fNofPiLikeEl << ", pi supp = " << (Double_t) fRElIdParams[1].size() / fNofPiLikeEl << endl;
   cout <<"Electrons like pions = " << fNofElLikePi << ", el lost eff = " << 100.* (Double_t)fNofElLikePi / fRElIdParams[0].size()<< endl;
   cout <<"ANN cut = " << fAnnCut << endl;

   Double_t cumProbFake = 0.;
   Double_t cumProbTrue = 0.;
   Int_t nofFake = (Int_t)fhAnnOutput[1]->GetEntries();
   Int_t nofTrue = (Int_t)fhAnnOutput[0]->GetEntries();
   for (Int_t i = 1; i <= fhAnnOutput[1]->GetNbinsX(); i++){
      cumProbFake += fhAnnOutput[1]->GetBinContent(i);
      fhCumProb[1]->SetBinContent(i, (Double_t)cumProbFake/nofFake);

      cumProbTrue += fhAnnOutput[0]->GetBinContent(i);
      fhCumProb[0]->SetBinContent(i, 1. - (Double_t)cumProbTrue / nofTrue);
   }

   SetDefaultDrawStyle();
   TCanvas* c1 = new TCanvas("ann_electrons_ann_output", "ann_electrons_ann_output", 500, 500);
   DrawH1(list_of(fhAnnOutput[0])(fhAnnOutput[1]), list_of("e^{#pm}")("#pi^{#pm}"),
         kLinear, kLog, true, 0.8, 0.8, 0.99, 0.99);

   TCanvas* c2 = new TCanvas("ann_electrons_cum_prob", "ann_electrons_cum_prob", 500, 500);
   DrawH1(list_of(fhCumProb[0])(fhCumProb[1]), list_of("e^{#pm}")("#pi^{#pm}"),
         kLinear, kLinear, true, 0.8, 0.8, 0.99, 0.99);

   int c = 1;
   TCanvas* c3 = new TCanvas("ann_electrons_params_ab", "ann_electrons_params_ab", 1200, 600);
   c3->Divide(2, 1);
   c3->cd(c++);
   DrawH1(list_of(fhAaxis[0])(fhAaxis[1]), list_of("e^{#pm}")("#pi^{#pm}"), kLinear, kLog, true, 0.8, 0.8, 0.99, 0.99);
   c3->cd(c++);
   DrawH1(list_of(fhBaxis[0])(fhBaxis[1]), list_of("e^{#pm}")("#pi^{#pm}"), kLinear, kLog, true, 0.8, 0.8, 0.99, 0.99);
//   c3->cd(c++);
//   DrawH1(list_of(fhAaxisCor[0])(fhAaxisCor[1]), list_of("e^{#pm}")("#pi^{#pm}"), kLinear, kLog, true, 0.8, 0.8, 0.99, 0.99);
//   c3->cd(c++);
//   DrawH1(list_of(fhBaxisCor[0])(fhBaxisCor[1]), list_of("e^{#pm}")("#pi^{#pm}"), kLinear, kLog, true, 0.8, 0.8, 0.99, 0.99);

   c = 1;
   TCanvas* c3_2 = new TCanvas("ann_electrons_params_1", "ann_electrons_params_1", 1500, 600);
   c3_2->Divide(3, 1);
   c3_2->cd(c++);
   //fhAaxisVsMom[0]->SetLineColor(kRed);
   //fhAaxisVsMom[1]->SetLineColor(kBlue);
   DrawH2(fhBaxisVsMom[1], kLinear, kLinear, kLinear);
   //DrawH2(fhAaxisVsMom[0], kLinear, kLinear, kLinear, "same");
   c3_2->cd(c++);
   //fhBaxisVsMom[0]->SetLineColor(kRed);
   //fhBaxisVsMom[1]->SetLineColor(kBlue);
   //DrawH2(fhBaxisVsMom[1], kLinear, kLinear, kLinear);
   DrawH2(fhBaxisVsMom[0], kLinear, kLinear, kLinear);

   c3_2->cd(c++);
   DrawH1(list_of(fhDistTrueMatch[0])(fhDistMisMatch[0])(fhDistTrueMatch[1])(fhDistMisMatch[1]),
         list_of("e^{#pm} true match")("e^{#pm} mis match")("#pi^{#pm} true match")("#pi^{#pm} mis match"),
         kLinear, kLog, true, 0.7, 0.7, 0.99, 0.99);

   c = 1;
   TCanvas* c3_1 = new TCanvas("ann_electrons_params_2", "ann_electrons_params_2", 1500, 600);
   c3_1->Divide(3, 1);
   c3_1->cd(c++);
   DrawH1(list_of(fhNofHits[0])(fhNofHits[1]), list_of("e^{#pm}")("#pi^{#pm}"), kLinear, kLog, true, 0.8, 0.8, 0.99, 0.99);
   c3_1->cd(c++);
   DrawH1(list_of(fhChi2[0])(fhChi2[1]), list_of("e^{#pm}")("#pi^{#pm}"), kLinear, kLog, true, 0.8, 0.8, 0.99, 0.99);
   c3_1->cd(c++);
   DrawH1(list_of(fhRadPos[0])(fhRadPos[1]), list_of("e^{#pm}")("#pi^{#pm}"), kLinear, kLog, true, 0.8, 0.8, 0.99, 0.99);

   c = 1;
   TCanvas* c4 = new TCanvas("ann_electrons_params_2d", "ann_electrons_params_2d", 600, 900);
   c4->Divide(2, 3);
   c4->cd(c++);
   DrawH2(fhAaxisVsMom[0]);
   c4->cd(c++);
   DrawH2(fhAaxisVsMom[1]);
   c4->cd(c++);
   DrawH2(fhBaxisVsMom[0]);
   c4->cd(c++);
   DrawH2(fhBaxisVsMom[1]);
   c4->cd(c++);
   DrawH2(fhPhiVsRadAng[0]);
   c4->cd(c++);
   DrawH2(fhPhiVsRadAng[1]);

//   TCanvas* c5 = new TCanvas("ann_electrons_b_vs_mom", "ann_electrons_b_vs_mom", 600, 600);
//   fhBaxisVsMom[0]->Add(fhBaxisVsMom[1]);
//   DrawH2(fhBaxisVsMom[0]);
//
//   TCanvas* c6 = new TCanvas("ann_electrons_a_vs_mom", "ann_electrons_a_vs_mom", 600, 600);
//   fhAaxisVsMom[0]->Add(fhAaxisVsMom[1]);
//   DrawH2(fhAaxisVsMom[0]);

}

void CbmRichTrainAnnElectrons::FinishTask()
{
   TrainAndTestAnn();
   Draw();

   for (int i = 0; i < fHists.size(); i++){
      fHists[i]->Scale(1./fHists[i]->Integral());
      fHists[i]->Write();
   }
}

ClassImp(CbmRichTrainAnnElectrons)
