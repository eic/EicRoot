/**
 * \file CbmRichTrainAnnSelect.cxx
 *
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2005
 **/

#include "CbmRichTrainAnnSelect.h"

#include "CbmRichHit.h"
#include "CbmRichRing.h"
#include "CbmTrackMatch.h"
#include "CbmMCTrack.h"
#include "CbmRichRingFitterCOP.h"
#include "CbmRichRingSelectImpl.h"
#include "FairRootManager.h"
#include "CbmDrawHist.h"
#include "CbmRichConverter.h"

#include "TMultiLayerPerceptron.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TClonesArray.h"

#include <boost/assign/list_of.hpp>

#include <vector>
#include <iostream>
#include <sstream>

using std::cout;
using std::endl;
using std::vector;
using boost::assign::list_of;

CbmRichTrainAnnSelect::CbmRichTrainAnnSelect():
   fRichRings(NULL),
   fMcTracks(NULL),
   fRichRingMatches(NULL),

   fEventNumber(0),
   fQuota(0.6),
   fMaxNofTrainSamples(5000),
   fNofFakeLikeTrue(0),
   fNofTrueLikeFake(0),
   fAnnCut(-0.5),

   fhNofHits(),
   fhAngle(),
   fhNofHitsOnRing(),
   fhChi2(),
   fhRadPos(),
   fhRadius(),

   fhAnnOutput(),
   fhCumProb(),

   fRSParams(),

   fFitCOP(NULL),
   fSelectImpl(NULL),

   fHists()
{
   fhNofHits.resize(2);
   fhAngle.resize(2);
   fhNofHitsOnRing.resize(2);
   fhChi2.resize(2);
   fhRadPos.resize(2);
   fhRadius.resize(2);
   fhAnnOutput.resize(2);
   fhCumProb.resize(2);
   string ss;
   for (int i = 0; i < 2; i++){
      if (i == 0) ss = "True";
      if (i == 1) ss = "Fake";
      // Difference Fake and True rings histograms BEGIN
      fhNofHits[i] = new TH1D(string("fhNofHits"+ss).c_str(),"Number of hits in ring;Nof hits in ring;Counter",50, 0, 50);
      fHists.push_back(fhNofHits[i]);
      fhAngle[i] = new TH1D(string("fhAngle"+ss).c_str(),"Biggest angle in ring;Angle [rad];Counter",50, 0, 6.5);
      fHists.push_back(fhAngle[i]);
      fhNofHitsOnRing[i] = new TH1D(string("fhNofHitsOnRing"+ss).c_str(),"Number of hits on ring;Nof hits on ring;Counter",50, 0, 50);
      fHists.push_back(fhNofHitsOnRing[i]);
      fhChi2[i] = new TH1D(string("fhFakeChi2"+ss).c_str(),"Chi2;Chi2;Counter", 100, 0., 1.0);
      fHists.push_back(fhChi2[i]);
      fhRadPos[i] = new TH1D(string("fhRadPos"+ss).c_str(),"Radial position;Radial position [cm];Counter",150, 0, 150);
      fHists.push_back(fhRadPos[i]);
      fhRadius[i] = new TH1D(string("fhRadius"+ss).c_str(),"Radius;Radius [cm];Counter",80, 0., 9.);
      fHists.push_back(fhRadius[i]);
      // ANN outputs
      fhAnnOutput[i] = new TH1D(string("fhAnnOutput"+ss).c_str(),"ANN output;ANN output;Counter",100, -1.2, 1.2);
      fHists.push_back(fhAnnOutput[i]);
      fhCumProb[i] = new TH1D(string("fhCumProb"+ss).c_str(),"ANN output;ANN output;Cumulative probability",100, -1.2, 1.2);
      fHists.push_back(fhCumProb[i]);
   }

   fRSParams.resize(2);
}

CbmRichTrainAnnSelect::~CbmRichTrainAnnSelect()
{

}

InitStatus CbmRichTrainAnnSelect::Init()
{
   cout << "InitStatus CbmRichTrainAnnSelect::Init()" << endl;
	FairRootManager* ioman = FairRootManager::Instance();
	if (NULL == ioman) { Fatal("CbmRichRingQa::Init","CbmRootManager is not instantiated"); }

	fRichRings = (TClonesArray*) ioman->GetObject("RichRing");
	if (NULL == fRichRings) { Fatal("CbmRichTrainAnnSelect::Init","No RichRing array!"); }

	fMcTracks = (TClonesArray*) ioman->GetObject("MCTrack");
	if (NULL == fMcTracks) { Fatal("CbmRichTrainAnnSelect::Init","No MCTrack array!");}

	fRichRingMatches = (TClonesArray*) ioman->GetObject("RichRingMatch");
	if (NULL == fRichRingMatches) { Fatal("CbmRichTrainAnnSelect::Init","No RichRingMatch array!");}

	fFitCOP = new CbmRichRingFitterCOP();
   fSelectImpl = new CbmRichRingSelectImpl();
   CbmRichConverter::Init();

   return kSUCCESS;
}

void CbmRichTrainAnnSelect::Exec(
      Option_t* option)
{
	fEventNumber++;
   cout<<"CbmRichRingQa Event No. "<< fEventNumber << endl;

   SetRecFlag();
   DiffFakeTrueCircle();

   cout << "nof trues = " << fRSParams[0].size() << endl;
   cout << "nof fakes = " << fRSParams[1].size() << endl;
}

void CbmRichTrainAnnSelect::SetRecFlag()
{
   Int_t nMatches = fRichRingMatches->GetEntriesFast();
   vector<Int_t> clone;
   clone.clear();

   for (Int_t iMatches = 0; iMatches < nMatches; iMatches++){
      CbmTrackMatch* match = (CbmTrackMatch*)fRichRingMatches->At(iMatches);
      if (NULL == match) continue;

      CbmRichRing* ring = (CbmRichRing*)fRichRings->At(iMatches);
      if (NULL == ring) continue;

      Int_t lTrueHits = match->GetNofTrueHits();
      Int_t lWrongHits = match->GetNofWrongHits();
      Int_t lFakeHits = match->GetNofFakeHits();
//      Int_t lMCHits = match->GetNofMCHits();
      Int_t lFoundHits = lTrueHits + lWrongHits + lFakeHits;
      Double_t lPercTrue = 0.;
      if (lFoundHits >= 1){
         lPercTrue = (Double_t)lTrueHits / (Double_t)lFoundHits;
      }

      Int_t trackID = match->GetMCTrackId();
      if (trackID > fMcTracks->GetEntriesFast() || trackID < 0) continue;
      CbmMCTrack* track = (CbmMCTrack*) fMcTracks->At(trackID);
      if (NULL == track) continue;

      Int_t gcode = TMath::Abs(track->GetPdgCode());
      //Double_t momentum = track->GetP();
      Int_t motherId = track->GetMotherId();

      ring->SetRecFlag(-1);

      if (lPercTrue < fQuota){ // fake ring
         ring->SetRecFlag(1);
      }else{ // true rings
         if (gcode == 11 && motherId == -1){ // select primary electrons
            ring->SetRecFlag(3);
         }
      }
   }// iMatches
}

void CbmRichTrainAnnSelect::DiffFakeTrueCircle()
{
   Int_t nMatches = fRichRingMatches->GetEntriesFast();

   for (Int_t iMatches = 0; iMatches < nMatches; iMatches++) {
      CbmTrackMatch* match = (CbmTrackMatch*) fRichRingMatches->At(iMatches);
      if (NULL == match) continue;

      CbmRichRing* ring = (CbmRichRing*) fRichRings->At(iMatches);
      if (NULL == ring) continue;

      CbmRichRingLight ringLight;
      CbmRichConverter::CopyHitsToRingLight(ring, &ringLight);
      fFitCOP->DoFit(&ringLight);
     
      Int_t recFlag = ring->GetRecFlag();
      Double_t angle = fSelectImpl->GetAngle(&ringLight);
      Int_t hitsOnRing = fSelectImpl->GetNofHitsOnRingCircle(&ringLight);
      Double_t chi2 = ringLight.GetChi2() / ringLight.GetNofHits();
      Int_t nHits = ringLight.GetNofHits();
      Double_t radPos = ringLight.GetRadialPosition();
      Double_t radius = ringLight.GetRadius();
      
      RingSelectParam p;
      p.fAngle = angle;
      p.fChi2 = chi2;
      p.fHitsOnRing = hitsOnRing;
      p.fNofHits = nHits;
      p.fRadPos = radPos;
      p.fRadius = radius;

      if (recFlag == 1){
         fhNofHits[1]->Fill(nHits);
         fhAngle[1]->Fill(angle);
         fhNofHitsOnRing[1]->Fill(hitsOnRing);
         fhRadPos[1]->Fill(radPos);
         fhRadius[1]->Fill(radius);
         fhChi2[1]->Fill(chi2);
         fRSParams[1].push_back(p);
      }

      if (recFlag == 3){
         fhNofHits[0]->Fill(nHits);
         fhAngle[0]->Fill(angle);
         fhNofHitsOnRing[0]->Fill(hitsOnRing);
         fhRadPos[0]->Fill(radPos);
         fhRadius[0]->Fill(radius);
         fhChi2[0]->Fill(chi2);
         fRSParams[0].push_back(p);
      }
   }// iMatches
}

void CbmRichTrainAnnSelect::TrainAndTestAnn()
{
   TTree *simu = new TTree ("MonteCarlo","MontecarloData");
   Double_t x[6];
   Double_t xOut;

   simu->Branch("x0", &x[0],"x0/D");
   simu->Branch("x1", &x[1],"x1/D");
   simu->Branch("x2", &x[2],"x2/D");
   simu->Branch("x3", &x[3],"x3/D");
   simu->Branch("x4", &x[4],"x4/D");
   simu->Branch("x5", &x[5],"x5/D");
   simu->Branch("xOut", &xOut,"xOut/D");

   for (int j = 0; j < 2; j++){
      for (int i = 0; i < fRSParams[j].size(); i++){
         x[0] = fRSParams[j][i].fNofHits / 45.;
         x[1] = fRSParams[j][i].fAngle / 6.28;
         x[2] = fRSParams[j][i].fHitsOnRing / 45.;
         x[3] = fRSParams[j][i].fRadPos / 110.;
         x[4] = fRSParams[j][i].fRadius / 10.;
         x[5] = fRSParams[j][i].fChi2 / 0.4;

         for (int k = 0; k < 6; k++){
            if (x[k] < 0.) x[k] = 0.;
            if (x[k] > 1.) x[k] = 1.;
         }

         if (j == 0) xOut = 1.;
         if (j == 1) xOut = -1.;
         simu->Fill();
         if (i >= fMaxNofTrainSamples) break;
      }
   }

   TMultiLayerPerceptron network("x0,x1,x2,x3,x4,x5:10:xOut",simu,"Entry$+1");
   //network.LoadWeights("/u/slebedev/JUL09/trunk/parameters/rich/NeuralNet_RingSelection_Weights_Compact.txt");
   network.Train(300,"text,update=10");
   network.DumpWeights("rich_select_ann_weights.txt");
   //network.Export();

   Double_t params[6];

   fNofFakeLikeTrue = 0;
   fNofTrueLikeFake = 0;

   for (int j = 0; j < 2; j++){
      for (int i = 0; i < fRSParams[j].size(); i++){
         params[0] = fRSParams[j][i].fNofHits / 45.;
         params[1] = fRSParams[j][i].fAngle / 6.28;
         params[2] = fRSParams[j][i].fHitsOnRing / 45.;
         params[3] = fRSParams[j][i].fRadPos / 110.;
         params[4] = fRSParams[j][i].fRadius / 10.;
         params[5] = fRSParams[j][i].fChi2 / 0.4;

         for (int k = 0; k < 6; k++){
            if (params[k] < 0.) params[k] = 0.;
            if (params[k] > 1.) params[k] = 1.;
         }

         Double_t netEval = network.Evaluate(0, params);

         //if (netEval > maxEval) netEval = maxEval - 0.01;
        // if (netEval < minEval) netEval = minEval + 0.01;

         fhAnnOutput[j]->Fill(netEval);
         if (netEval >= fAnnCut && j == 1) fNofFakeLikeTrue++;
         if (netEval < fAnnCut && j == 0) fNofTrueLikeFake++;
      }
   }
}

void CbmRichTrainAnnSelect::Draw()
{
   cout <<"nof Trues = " << fRSParams[0].size() << endl;
   cout <<"nof Fakes = " << fRSParams[1].size() << endl;
   cout <<"Fake like True = " << fNofFakeLikeTrue << ", fake remain = " << (double)fNofFakeLikeTrue/fRSParams[1].size() << endl;
   cout <<"True like Fake = " << fNofTrueLikeFake << ", true loss = " << (double)fNofTrueLikeFake/fRSParams[0].size() << endl;
   cout << "ANN cut = " << fAnnCut << endl;

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

   TCanvas* c1 = new TCanvas("ann_select_ann_output", "ann_select_ann_output", 500, 500);
   fhAnnOutput[0]->Scale(1./fhAnnOutput[0]->Integral());
   fhAnnOutput[1]->Scale(1./fhAnnOutput[1]->Integral());
   DrawH1(list_of(fhAnnOutput[0])(fhAnnOutput[1]), list_of("True")("Fake"), kLinear, kLog, true, 0.8, 0.8, 0.99, 0.99);

   TCanvas* c2 = new TCanvas("ann_select_cum_prob", "ann_select_cum_prob", 500, 500);
   DrawH1(list_of(fhCumProb[0])(fhCumProb[1]), list_of("True")("Fake"), kLinear, kLinear, true, 0.8, 0.8, 0.99, 0.99);

   TCanvas* c3 = new TCanvas("ann_select_params", "ann_select_params", 900, 600);
   c3->Divide(3, 2);
   c3->cd(1);
   DrawH1(list_of(fhNofHits[0])(fhNofHits[1]), list_of("True")("Fake"), kLinear, kLog, true, 0.8, 0.8, 0.99, 0.99);
   c3->cd(2);
   DrawH1(list_of(fhAngle[0])(fhAngle[1]), list_of("True")("Fake"), kLinear, kLog, true, 0.8, 0.8, 0.99, 0.99);
   c3->cd(3);
   DrawH1(list_of(fhNofHitsOnRing[0])(fhNofHitsOnRing[1]), list_of("True")("Fake"), kLinear, kLog, true, 0.8, 0.8, 0.99, 0.99);
   c3->cd(4);
   DrawH1(list_of(fhRadPos[0])(fhRadPos[1]), list_of("True")("Fake"), kLinear, kLog, true, 0.8, 0.8, 0.99, 0.99);
   c3->cd(5);
   DrawH1(list_of(fhChi2[0])(fhChi2[1]), list_of("True")("Fake"), kLinear, kLog, true, 0.8, 0.8, 0.99, 0.99);
   c3->cd(6);
   DrawH1(list_of(fhRadius[0])(fhRadius[1]), list_of("True")("Fake"), kLinear, kLog, true, 0.8, 0.8, 0.99, 0.99);
}

void CbmRichTrainAnnSelect::FinishTask()
{
   TrainAndTestAnn();
   Draw();

   TDirectory *current = gDirectory;
   TDirectory *rich = current->mkdir("CbmRichTrainAnnSelect");
   rich->cd();

   for (int i = 0; i < fHists.size(); i++ ){
      fHists[i]->Write();
   }

   rich->cd();
   current->cd();

   delete fFitCOP;
   delete fSelectImpl;
}

ClassImp(CbmRichTrainAnnSelect)
