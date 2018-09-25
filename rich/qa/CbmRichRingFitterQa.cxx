/**
* \file CbmRichRingFitterQa.cxx
*
* \author Semen Lebedev <s.lebedev@gsi.de>
* \date 2009
**/

#include "CbmRichRingFitterQa.h"
#include "CbmRichRingFitterEllipseTau.h"
#include "CbmRichRingFitterEllipseMinuit.h"
#include "CbmRichRingFitterCOP.h"

#include "TRandom.h"
#include "TMath.h"
#include "CbmRichRing.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TMatrixD.h"
#include "TH1D.h"

#include <iostream>
#include <vector>

using std::vector;
using std::cout;
using std::endl;

CbmRichRingFitterQa::CbmRichRingFitterQa():
   fhErrorA(NULL),
   fhErrorB(NULL),
   fhErrorX(NULL),
   fhErrorY(NULL),
   fhErrorPhi(NULL),

   fhA(NULL),
   fhB(NULL),
   fhX(NULL),
   fhY(NULL),
   fhPhi(NULL),

   fhRadiusErr(NULL),
   fhCircleXcErr(NULL),
   fhCircleYcErr(NULL),

   fhRadius(NULL),
   fhCircleXc(NULL),
   fhCircleYc(NULL),

   fhRadiusPool(NULL),
   fhCircleXcPool(NULL),
   fhCircleYcPool(NULL)
{
	fhErrorA = new TH1D("fhErrorA","fhErrorA;dA [cm];Counter", 100, -2., 2.);
	fhErrorB = new TH1D("fhErrorB","fhErrorB;B [cm];Counter", 100, -2., 2.);
	fhErrorX = new TH1D("fhErrorX","fhErrorX;X [cm];Counter", 100, -2., 2.);
	fhErrorY = new TH1D("fhErrorY","fhErrorY;Y [cm];Counter", 100, -2., 2.);
	fhErrorPhi = new TH1D("fhErrorPhi","fhErrorPhi;d#Phi [rad];Counter", 100, -2., 2.);

	fhA = new TH1D("fhA","fhA;A [cm];Counter", 100, 5., 7.);
	fhB = new TH1D("fhB","fhB;B [cm];Counter", 100, 5., 7.);
	fhX = new TH1D("fhX","fhX;X [cm];Counter", 100, -1., 1.);
	fhY = new TH1D("fhY","fhY;Y [cm];Counter", 100, -1., 1.);
	Double_t pi = TMath::Pi();
	fhPhi = new TH1D("fhPhi","fhPhi;#Phi [rad];Counter", 100, - pi/2. - pi/6. , pi/2. + pi/6.);

	// circle fitting
   fhRadiusErr = new TH1D("fhRadiusErr","fhRadiusErr;dR [cm];Counter", 100, -2., 2.);
   fhCircleXcErr = new TH1D("fhCircleXcErr","fhCircleXcErr;dXc [cm];Counter", 100, -2., 2.);
   fhCircleYcErr = new TH1D("fhCircleYcErr","fhCircleYcErr;dYc [cm];Counter", 100, -2., 2.);
   fhRadius = new TH1D("fhRadius","fhRadius;Radius [cm];Counter", 100, 4., 8.);
   fhCircleXc = new TH1D("fhCircleXc","fhCircleXc;Xc [cm];Counter", 100, -2., 2.);
   fhCircleYc = new TH1D("fhCircleYc","fhCircleYc;Yc [cm];Counter", 100, -2., 2.);
   fhRadiusPool = new TH1D("fhRadiusPool","fhRadiusPool;Pool R;Counter", 100, -5., 5.);
   fhCircleXcPool = new TH1D("fhCircleXcPool","fhCircleXcPool;Pool Xc;Counter", 100, -5., 5.);
   fhCircleYcPool = new TH1D("fhCircleYcPool","fhCircleYcPool;Pool Yc;Counter", 100, -5., 5.);
}

CbmRichRingFitterQa::~CbmRichRingFitterQa()
{

}

void CbmRichRingFitterQa::GenerateEllipse()
{
	Double_t maxX = 15;
	Double_t maxY = 15;
	Int_t nofHits = 50;
	Double_t A = 6.;
	Double_t B = 6.;
	Double_t sigmaError = 0.2;
	CbmRichRingLight ellipse;
	Int_t nofBadFit = 0;
   Double_t X0 = 0.;//gRandom->Rndm()*(maxX - A);
   Double_t Y0 = 0.;//gRandom->Rndm()* (maxY - A);

	CbmRichRingFitterEllipseTau * fitEllipse = new CbmRichRingFitterEllipseTau();
   CbmRichRingFitterCOP * fitCircle = new CbmRichRingFitterCOP();

	for (Int_t iR = 0; iR < 50000; iR++){
		Double_t phi = 0.;//TMath::Pi()*(6./12.); //gRandom->Rndm()*TMath::Pi() - TMath::Pi()/2.;
		ellipse.SetXYABP(X0, Y0, A, B, phi);
		for (Int_t iH = 0; iH < nofHits; iH++){
			Double_t alfa = gRandom->Rndm()*2.*TMath::Pi();

			Double_t errorX = gRandom->Gaus(0, sigmaError);
			Double_t errorY = gRandom->Gaus(0, sigmaError);

			Double_t hx = A * cos(alfa);
			Double_t hy = B * sin(alfa);

			Double_t hitXRot = hx * cos(phi) - hy * sin(phi);
			Double_t hitYRot = hx * sin(phi) + hy * cos(phi);

			CbmRichHitLight hit(hitXRot + X0 + errorX, hitYRot + Y0 + errorY);
			ellipse.AddHit(hit);
		}
		// ellipse fit
		fitEllipse->DoFit(&ellipse);
		fhErrorA->Fill(A - ellipse.GetAaxis());
		fhErrorB->Fill(B - ellipse.GetBaxis());
		fhErrorX->Fill(X0 - ellipse.GetCenterX());
		fhErrorY->Fill(Y0 - ellipse.GetCenterY());
		fhErrorPhi->Fill(phi - ellipse.GetPhi());
		fhA->Fill(ellipse.GetAaxis());
		fhB->Fill(ellipse.GetBaxis());
		fhX->Fill(ellipse.GetCenterX());
		fhY->Fill(ellipse.GetCenterY());
		fhPhi->Fill(ellipse.GetPhi());

		// circle fit
      fitCircle->DoFit(&ellipse);
		TMatrixD cov(3,3);
      CalculateFitErrors(&ellipse, sigmaError, cov);
      Double_t mcR = (A + B) / 2.;
      fhRadiusErr->Fill(mcR - ellipse.GetRadius());
      fhCircleXcErr->Fill(X0 - ellipse.GetCenterX());
      fhCircleYcErr->Fill(Y0 - ellipse.GetCenterY());
      fhRadius->Fill(ellipse.GetRadius());
      fhCircleXc->Fill(ellipse.GetCenterX());
      fhCircleYc->Fill(ellipse.GetCenterY());
      fhRadiusPool->Fill( (mcR - ellipse.GetRadius()) / sqrt(cov(2,2)) );
      fhCircleXcPool->Fill( (X0 - ellipse.GetCenterX()) / sqrt(cov(0,0)) );
      fhCircleYcPool->Fill( (Y0 - ellipse.GetCenterY()) / sqrt(cov(1,1)) );
	}// iR

	Draw();
   cout << nofBadFit << endl;
}

void CbmRichRingFitterQa::Draw()
{
	TCanvas * c = new TCanvas("rich_fitter_errors", "rich_fitter_errors", 900, 600);
	c->Divide(3,2);
	c->cd(1);
	fhErrorA->Draw();
	c->cd(2);
	fhErrorB->Draw();
	c->cd(3);
	fhErrorX->Draw();
	c->cd(4);
	fhErrorY->Draw();
	c->cd(5);
	fhErrorPhi->Draw();
	cout.precision(4);
	cout << fhErrorA->GetMean() << " " << fhErrorA->GetRMS() << endl;
	cout << fhErrorB->GetMean() << " " << fhErrorB->GetRMS() << endl;
	cout << fhErrorX->GetMean() << " " << fhErrorX->GetRMS() << endl;
	cout << fhErrorY->GetMean() << " " << fhErrorY->GetRMS() << endl;
	cout << fhErrorPhi->GetMean() << " " << fhErrorPhi->GetRMS() << endl;

	TCanvas * c2 = new TCanvas("rich_fitter_params", "rich_fitter_params", 900, 600);
	c2->Divide(3,2);
	c2->cd(1);
	fhA->Draw();
	c2->cd(2);
	fhB->Draw();
	c2->cd(3);
	fhX->Draw();
	c2->cd(4);
	fhY->Draw();
	c2->cd(5);
	fhPhi->Draw();

   TCanvas * c3 = new TCanvas("rich_fitter_circle", "rich_fitter_circle", 900, 900);
   c3->Divide(3,3);
   c3->cd(1);
   fhRadiusErr->Draw();
   c3->cd(2);
   fhCircleXcErr->Draw();
   c3->cd(3);
   fhCircleYcErr->Draw();
   c3->cd(4);
   fhRadius->Draw();
   c3->cd(5);
   fhCircleXc->Draw();
   c3->cd(6);
   fhCircleYc->Draw();
   c3->cd(7);
   fhRadiusPool->Draw();
   c3->cd(8);
   fhCircleXcPool->Draw();
   c3->cd(9);
   fhCircleYcPool->Draw();

}

void CbmRichRingFitterQa::CalculateFitErrors(
      CbmRichRingLight* ring,
      Double_t sigma,
      TMatrixD& cov)
{
   //TMatrixD H3(3,3);
   TMatrixD HY3(3,1);
   //TMatrixD Cov3(3,3);
   TMatrixD HC3(3,1);

   for (Int_t i = 0; i < 3; i++){
      HY3(i,0) = 0;
      HC3(i,0) = 0;
      for (Int_t j = 0; j < 3; j++){
         //H3(i,j) = 0;
         cov(i,j) = 0;
      }
   }

   Double_t xc = ring->GetCenterX();
   Double_t yc = ring->GetCenterY();
   Double_t R = ring->GetRadius();
   for (Int_t iHit = 0; iHit < ring->GetNofHits(); iHit++) {
      Double_t xi = ring->GetHit(iHit).fX;
      Double_t yi = ring->GetHit(iHit).fY;
      Double_t ri = sqrt((xi - xc) * (xi - xc) + (yi - yc) * (yi - yc));
      Double_t err = sigma;

      Double_t f1 = (-1.0 * (xi - xc)) / (ri * err);
      Double_t f2 = (-1.0 * (yi - yc))/(ri * err);
      Double_t f3 = (-1.) / err;
      Double_t Y = (R - ri) / err;

      cov(0,0) = cov(0,0) + f1*f1;
      cov(0,1) = cov(0,1) + f1*f2;
      cov(0,2) = cov(0,2) + f1*f3;

      cov(1,0) = cov(0,1);
      cov(1,1) = cov(1,1) + f2*f2;
      cov(1,2) = cov(1,2) + f2*f3;

      cov(2,0) = cov(0,2);
      cov(2,1) = cov(1,2);
      cov(2,2) = cov(2,2) + f3*f3;

      HY3(0,0) = HY3(0,0) + Y*f1;
      HY3(1,0) = HY3(1,0) + Y*f2;
      HY3(2,0) = HY3(2,0) + Y*f3;
   }// iHit

   //H3.Print();
   Double_t det = 0.0;
   cov.Invert(&det);
   //Cov3 = H3;
   //H3.Print();
   //Cov3.Print();

   //HC3 = H3 * HY3;
   //HC3.Print();

   //cout << "dX0= " << HC3(0,0) << " +- " << sqrt(Cov3(0,0)) << endl;
  // cout << "dY0= " << HC3(1,0) <<  " +- " << sqrt(Cov3(1,1)) << endl;
   //cout << "dR= " << HC3(2,0) << " +- " << sqrt(Cov3(2,2)) << endl;

   //cout << "dX0= " << HC3(0,0) << " +- " << sqrt(Cov3(0,0)) << endl;
  // cout << "dY0= " << HC3(1,0) <<  " +- " << sqrt(Cov3(1,1)) << endl;
  // cout << "dR= " << HC3(2,0) << " +- " << sqrt(Cov3(2,2)) << endl;


}

ClassImp(CbmRichRingFitterQa);
