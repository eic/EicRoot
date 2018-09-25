/**
* \file CbmRichRingFinderHoughImpl.cxx
*
* \author Semen Lebedev
* \date 2008
**/

#include "CbmRichRingFinderHoughImpl.h"
#include "CbmRichRingLight.h"

#include "../../littrack/std/utils/CbmLitMemoryManagment.h"
#include "CbmRichRingFitterCOP.h"
#include "CbmRichRingSelectAnn.h"

#include "TSystem.h"

#include <iostream>
#include <cmath>
#include <set>
#include <algorithm>
#include <iostream>

using std::cout;
using std::endl;
using std::vector;
using std::set;
using std::sort;

CbmRichRingFinderHoughImpl::CbmRichRingFinderHoughImpl():
   fNofParts(0),

   fMaxDistance(0.f),
   fMinDistance(0.f),
   fMinDistanceSq(0.f),
   fMaxDistanceSq(0.f),

   fMinRadius(0.f),
   fMaxRadius(0.f),

   fDx(0.f),
   fDy(0.f),
   fDr(0.f),
   fNofBinsX(0),
   fNofBinsY(0),
   fNofBinsXY(0),

   fHTCut(0),

   fNofBinsR(0),
   fHTCutR(0),

   fMinNofHitsInArea(0),

   fRmsCoeffEl(0.f),
   fMaxCutEl(0.f),
   fRmsCoeffCOP(0.f),
   fMaxCutCOP(0.f),

   fAnnCut(0.f),
   fUsedHitsAllCut(0.f),

   fCurMinX(0.f),
   fCurMinY(0.f),

   fData(),
   fHist(),
   fHistR(),
   fHitInd(),
   fFoundRings(),
   fFitCOP(NULL),
   fANNSelect(NULL)
{

}

CbmRichRingFinderHoughImpl::~CbmRichRingFinderHoughImpl()
{
	if (NULL != fFitCOP) delete fFitCOP;
	if (NULL != fANNSelect) delete fANNSelect;
}

void CbmRichRingFinderHoughImpl::Init()
{
    SetParameters();

    fHist.resize(fNofBinsXY);
    fHistR.resize(fNofBinsR);
	 fHitInd.resize(fNofParts);

    fFitCOP = new CbmRichRingFitterCOP();
    fANNSelect = new CbmRichRingSelectAnn();
    fANNSelect->Init();
}

void CbmRichRingFinderHoughImpl::DoFind()
{
	if (fData.size() > MAX_NOF_HITS) {
	   cout << endl << endl << "-E- CbmRichRingFinderHoughImpl::DoFind"
	         << ". Number of hits is more than " << MAX_NOF_HITS << endl << endl;
	   return;
	}

	for_each(fFoundRings.begin(), fFoundRings.end(), DeleteObject());
	fFoundRings.clear();
	fFoundRings.reserve(100);

	std::sort(fData.begin(), fData.end(), CbmRichHoughHitCmpUp());
	HoughTransformReconstruction();
	RingSelection();
	fData.clear();
}

void CbmRichRingFinderHoughImpl::SetParameters()
{
	fMaxDistance = 11.5;
	fMinDistance = 3.;
	fMinDistanceSq = fMinDistance*fMinDistance;
	fMaxDistanceSq = fMaxDistance*fMaxDistance;

	fMinRadius = 3.3;
	fMaxRadius = 5.7;

	fHTCut = 7;
	fHTCutR = 5;
	fMinNofHitsInArea = 4;

	fNofBinsX = 25;
	fNofBinsY = 25;
	fNofBinsR = 32;

	fAnnCut = 0.6;
	fUsedHitsAllCut = 0.4;

	fRmsCoeffEl = 2.5;
	fMaxCutEl = 1.0;
	fRmsCoeffCOP = 3.;
	fMaxCutCOP = 1.0;

	fNofParts = 2;
   fDx = 2.f * fMaxDistance / (float)fNofBinsX;
   fDy = 2.f * fMaxDistance / (float)fNofBinsY;
   fDr = fMaxRadius / (float)fNofBinsR;
   fNofBinsXY = fNofBinsX * fNofBinsY;
}

void CbmRichRingFinderHoughImpl::HoughTransformReconstruction()
{
   int indmin, indmax;
   unsigned int size = fData.size();
   for (unsigned int iHit = 0; iHit < size; iHit++){
      if (fData[iHit].fIsUsed == true) continue;

      fCurMinX = fData[iHit].fHit.fX - fMaxDistance;
      fCurMinY = fData[iHit].fHit.fY - fMaxDistance;

      DefineLocalAreaAndHits(fData[iHit].fHit.fX, fData[iHit].fHit.fY , &indmin, &indmax);
      HoughTransform(indmin, indmax);
      FindPeak(indmin, indmax);
   }
}

void CbmRichRingFinderHoughImpl::DefineLocalAreaAndHits(
      float x0,
      float y0,
		int *indmin,
		int *indmax)
{
   CbmRichHoughHit mpnt;
   vector<CbmRichHoughHit>::iterator itmin, itmax;

	//find all hits which are in the corridor
	mpnt.fHit.fX = x0 - fMaxDistance;
	itmin = std::lower_bound(fData.begin(), fData.end(), mpnt, CbmRichHoughHitCmpUp());

	mpnt.fHit.fX = x0 + fMaxDistance;
	itmax = std::lower_bound(fData.begin(), fData.end(), mpnt, CbmRichHoughHitCmpUp()) - 1;

	*indmin = itmin - fData.begin();
	*indmax = itmax - fData.begin();

	int arSize = *indmax - *indmin + 1;
	if (arSize <= fMinNofHitsInArea) return;

	for (unsigned short i = 0; i < fNofParts; i++){
		fHitInd[i].clear();
		fHitInd[i].reserve( (*indmax-*indmin) / fNofParts);
	}

	unsigned short indmin1 = (unsigned short)(*indmin);
	unsigned short indmax1 = (unsigned short)(*indmax);

   for (unsigned short i = indmin1; i <= indmax1; i++) {
      if (fData[i].fIsUsed == true) continue;
      float ry = y0 - fData[i].fHit.fY;
      if (fabs(ry) > fMaxDistance) continue;
      float rx = x0 - fData[i].fHit.fX;
      float d = rx * rx + ry * ry;
      if (d > fMaxDistanceSq) continue;
	   fHitInd[i % fNofParts].push_back(i);
   }

	for (unsigned short j = 0; j < fNofBinsXY; j++){
		fHist[j] = 0;
	}

	for (unsigned short k = 0; k < fNofBinsR; k++) {
		fHistR[k] = 0;
	}
}

void CbmRichRingFinderHoughImpl::HoughTransform(
      unsigned short int indmin,
      unsigned short int indmax)
{
    for (int iPart = 0; iPart < fNofParts; iPart++){
    	 HoughTransformGroup(indmin, indmax, iPart);
    }//iPart
}

void CbmRichRingFinderHoughImpl::HoughTransformGroup(
      unsigned short int indmin,
		unsigned short int indmax,
		int iPart)
{
   unsigned short nofHits = fHitInd[iPart].size();
   float xcs, ycs; // xcs = xc - fCurMinX
   float dx = 1.0f/fDx, dy = 1.0f/fDy, dr = 1.0f/fDr;

   vector<CbmRichHoughHit> data;
   data.resize(nofHits);
   for (int i = 0; i < nofHits; i++){
      data[i] = fData[ fHitInd[iPart][i] ];
   }

   typedef vector<CbmRichHoughHit>::iterator iH;

	for (iH iHit1 = data.begin(); iHit1 != data.end(); iHit1++) {
	   float iH1X = iHit1->fHit.fX;
		float iH1Y = iHit1->fHit.fY;

      for (iH iHit2 = iHit1 + 1; iHit2 != data.end(); iHit2++) {
         float iH2X = iHit2->fHit.fX;
         float iH2Y = iHit2->fHit.fY;

         float rx0 = iH1X - iH2X;//rx12
         float ry0 = iH1Y- iH2Y;//ry12
         float r12 = rx0 * rx0 + ry0 * ry0;
         if (r12 < fMinDistanceSq || r12 > fMaxDistanceSq)	continue;

         float t10 = iHit1->fX2plusY2 - iHit2->fX2plusY2;
         for (iH iHit3 = iHit2 + 1; iHit3 != data.end(); iHit3++) {
            float iH3X = iHit3->fHit.fX;
            float iH3Y = iHit3->fHit.fY;

            float rx1 = iH1X - iH3X;//rx13
            float ry1 = iH1Y - iH3Y;//ry13
            float r13 = rx1 * rx1 + ry1 * ry1;
            if (r13 < fMinDistanceSq || r13 > fMaxDistanceSq)continue;

            float rx2 = iH2X - iH3X;//rx23
            float ry2 = iH2Y - iH3Y;//ry23
            float r23 = rx2 * rx2 + ry2 * ry2;
            if (r23	< fMinDistanceSq || r23 > fMaxDistanceSq)continue;

            float det = rx2*ry0 - rx0*ry2;
            if (det == 0.0f) continue;
            float t19 = 0.5f / det;
            float t5 = iHit2->fX2plusY2 - iHit3->fX2plusY2;

            float xc = (t5 * ry0 - t10 * ry2) * t19;
            xcs = xc - fCurMinX;
            int intX = int( xcs *dx);
            if (intX < 0 || intX >= fNofBinsX ) continue;

            float yc = (t10 * rx2 - t5 * rx0) * t19;
            ycs = yc - fCurMinY;
            int intY = int( ycs *dy);
            if (intY < 0 || intY >= fNofBinsY ) continue;

            //radius calculation
            float t6 = iH1X - xc;
            float t7 = iH1Y - yc;
            //if (t6 > fMaxRadius || t7 > fMaxRadius) continue;
            float r = sqrt(t6 * t6 + t7 * t7);
            //if (r < fMinRadius) continue;
            int intR = int(r *dr);
            if (intR < 0 || intR >= fNofBinsR) continue;

            fHist[intX * fNofBinsX + intY]++;
            fHistR[intR]++;
         }// iHit1
      }// iHit2
   }// iHit3
	//hitIndPart.clear();
}

void CbmRichRingFinderHoughImpl::FindPeak(
      int indmin,
      int indmax)
{
   // Find MAX bin R and compare it with CUT
   int maxBinR = -1, maxR = -1;
   unsigned int size = fHistR.size();
   for (unsigned int k = 0; k < size; k++){
      if (fHistR[k] > maxBinR){
         maxBinR = fHistR[k];
         maxR = k;
      }
   }
	if (maxBinR < fHTCutR) return;

   // Find MAX bin XY and compare it with CUT
	int maxBinXY = -1, maxXY = -1;
   size = fHist.size();
   for (unsigned int k = 0; k < size; k++){
      if (fHist[k] > maxBinXY){
         maxBinXY = fHist[k];
         maxXY = k;
      }
   }
   if (maxBinXY < fHTCut) return;

	CbmRichRingLight* ring1 = new CbmRichRingLight();

   // Find Preliminary Xc, Yc, R
   float xc, yc, r;
   float rx, ry, dr;
	xc = (maxXY / fNofBinsX + 0.5f)* fDx + fCurMinX;
	yc = (maxXY % fNofBinsX + 0.5f)* fDy + fCurMinY;
	r = (maxR + 0.5f) * fDr;
	for (int j = indmin; j < indmax + 1; j++) {
		rx = fData[j].fHit.fX - xc;
		ry = fData[j].fHit.fY - yc;

		dr = fabs(sqrt(rx * rx + ry * ry) - r);
		if (dr > 0.6f) continue;
		ring1->AddHit(fData[j].fHit, fData[j].fId);
	}

	if (ring1->GetNofHits() < 7) {
	   delete ring1;
	   return;
	}

	fFitCOP->DoFit(ring1);
	float drCOPCut = fRmsCoeffCOP * sqrt(ring1->GetChi2() / ring1->GetNofHits());
	if (drCOPCut > fMaxCutCOP)	drCOPCut = fMaxCutCOP;

	xc = ring1->GetCenterX();
	yc = ring1->GetCenterY();
	r = ring1->GetRadius();

	delete ring1;

	CbmRichRingLight* ring2 = new CbmRichRingLight();
	for (int j = indmin; j < indmax + 1; j++) {
		rx = fData[j].fHit.fX - xc;
		ry = fData[j].fHit.fY - yc;

		dr = fabs(sqrt(rx * rx + ry * ry) - r);
		if (dr > drCOPCut) continue;
		//fData[j+indmin].fIsUsed = true;
		ring2->AddHit(fData[j].fHit, fData[j].fId);
	}

	if (ring2->GetNofHits() < 7) {
	   delete ring2;
	   return;
	}

	fFitCOP->DoFit(ring2);

	fANNSelect->DoSelect(ring2);
	float select = ring2->GetSelectionNN();

	// Remove found hits only for good quality rings
	if (select > fAnnCut) {
		RemoveHitsAroundRing(indmin, indmax, ring2);
	}

	if (select > -0.7) {
		fFoundRings.push_back(ring2);
	}
	ring2=NULL;
	delete ring2;
}

void CbmRichRingFinderHoughImpl::RemoveHitsAroundRing(
      int indmin,
      int indmax,
		CbmRichRingLight* ring)
{
	float rms = sqrt(ring->GetChi2() / ring->GetNofHits());
	float dCut = fRmsCoeffEl * rms;
	if (dCut > fMaxCutEl) dCut = fMaxCutEl;

	for (int j = indmin; j < indmax + 1; j++) {
	   float rx = fData[j].fHit.fX - ring->GetCenterX();
	   float ry = fData[j].fHit.fY - ring->GetCenterY();

	   float dr = fabs(sqrt(rx * rx + ry * ry) - ring->GetRadius());
		if (dr < dCut) {
			fData[j].fIsUsed = true;
		}
	}
}

void CbmRichRingFinderHoughImpl::RingSelection()
{
	int nofRings = fFoundRings.size();
	sort(fFoundRings.begin(), fFoundRings.end(), CbmRichRingComparatorMore());
	set<unsigned short> usedHitsAll;
	vector<unsigned short> goodRingIndex;
	goodRingIndex.reserve(nofRings);
	CbmRichRingLight* ring2;

	for (int iRing = 0; iRing < nofRings; iRing++){
		CbmRichRingLight* ring = fFoundRings[iRing];
		ring->SetRecFlag(-1);
		int nofHits = ring->GetNofHits();
		bool isGoodRingAll = true;
		int nofUsedHitsAll = 0;
		for(int iHit = 0; iHit < nofHits; iHit++){
			set<unsigned short>::iterator it = usedHitsAll.find(ring->GetHitId(iHit));
			if(it != usedHitsAll.end()){
				nofUsedHitsAll++;
			}
		}
		if ((float)nofUsedHitsAll/(float)nofHits > fUsedHitsAllCut){
			isGoodRingAll = false;
		}

		if (isGoodRingAll){
			fFoundRings[iRing]->SetRecFlag(1);
			for (int iRSet = 0; iRSet < goodRingIndex.size(); iRSet++){
				ReAssignSharedHits(goodRingIndex[iRSet],iRing);
			}
			goodRingIndex.push_back(iRing);
			for(int iHit = 0; iHit < nofHits; iHit++){
				usedHitsAll.insert(ring->GetHitId(iHit));
			}
		}// isGoodRing
	}// iRing

//	usedHits.clear();
	usedHitsAll.clear();
	goodRingIndex.clear();
}

void CbmRichRingFinderHoughImpl::ReAssignSharedHits(
      int ringInd1,
      int ringInd2)
{
	CbmRichRingLight* ring1 = fFoundRings[ringInd1];
	CbmRichRingLight* ring2 = fFoundRings[ringInd2];
	int nofHits1 = ring1->GetNofHits();
	int nofHits2 = ring2->GetNofHits();

	for(int iHit1 = 0; iHit1 < nofHits1; iHit1++){
		unsigned short hitInd1 = ring1->GetHitId(iHit1);
		for(int iHit2 = 0; iHit2 < nofHits2; iHit2++){
			unsigned short hitInd2 = ring2->GetHitId(iHit2);
			if(hitInd1 != hitInd2) continue;
			int hitIndData =  GetHitIndex(hitInd1);
			float hitX = fData[hitIndData].fHit.fX;
			float hitY = fData[hitIndData].fHit.fY;
			float rx1 = hitX - ring1->GetCenterX();
			float ry1 = hitY - ring1->GetCenterY();
			float dr1 = fabs(sqrt(rx1 * rx1 + ry1 * ry1) - ring1->GetRadius());

			float rx2 = hitX - ring2->GetCenterX();
			float ry2 = hitY - ring2->GetCenterY();
			float dr2 = fabs(sqrt(rx2 * rx2 + ry2 * ry2) - ring2->GetRadius());

			if (dr1 > dr2){
				ring1->RemoveHit(hitInd1);
			} else {
				ring2->RemoveHit(hitInd2);
			}
		}//iHit2
	}//iHit1
}

int CbmRichRingFinderHoughImpl::GetHitIndex(
      unsigned short hitInd)
{
   unsigned short size = fData.size();
	for (unsigned short i = 0; i < size; i++){
		if (fData[i].fId == hitInd) return i;
	}
	return -1;
}

void CbmRichRingFinderHoughImpl::CalculateRingParameters(
      float x[],
      float y[],
      float *xc,
      float *yc,
      float *r)
{
    register float t1, t2, t3, t4, t5, t6, t8, t9,
        t10, t11, t14, t16, t19, t21, t41;

    t1 = x[1] * x[1];
    t2 = x[2] * x[2];
    t3 = y[1] * y[1];
    t4 = y[2] * y[2];
    t5 = t1 - t2 + t3 - t4;
    t6 = y[0] - y[1];
    t8 = x[0] * x[0];
    t9 = y[0] * y[0];
    t10 = t8 - t1 + t9 - t3;
    t11 = y[1] - y[2];
    t14 = x[1] - x[2];
    t16 = x[0] - x[1];
    t19 = 1.0f / (t14 * t6 - t16 * t11);

    *xc = 0.5e0 * (t5 * t6 - t10 * t11) * t19;
    *yc = 0.5e0 * (t10 * t14 - t5 * t16) * t19;

    t21 = (x[0] - *xc)*(x[0] - *xc);
    t41 = (y[0] - *yc)*(y[0] - *yc);
    *r = sqrt(t21 + t41);
}
