/**
* \file CbmRichRingFinderHoughSimd.cxx
*
* \author Semen Lebedev
* \date 2008
**/

//#include "../L1/L1Algo/L1Types.h"
#include "../L1/L1Algo/vectors/P4_F32vec4.h"
#include <emmintrin.h>
#include "CbmRichRingFinderHoughSimd.h"

CbmRichRingFinderHoughSimd::CbmRichRingFinderHoughSimd()
{

}

void CbmRichRingFinderHoughSimd::HoughTransformGroup(
      unsigned short int indmin,
		unsigned short int indmax,
		Int_t iPart)
{
//	register Float_t r12, r13, r23;
//    register Float_t rx0, rx1, rx2, ry0, ry1,ry2; //rx[3], ry[3];//, x[3], y[3];
    //register Float_t xc, yc, r;
    //register Float_t xcs, ycs; // xcs = xc - fCurMinX
    register Int_t* intX, *intY, *intR;
    register Int_t* indXY;

    register unsigned short int iH11, iH12, iH13, iH14, iH2, iH3;
    register Int_t nofHitsNorm = fHitInd[0].size() + 1;
    register Int_t iPmulNofHits;

    //register Float_t t5, t10, t19, det, t6, t7;
    register Float_t dx = 1.0f/fDx, dy = 1.0f/fDy, dr = 1.0f/fDr;
    //register Float_t iH1X, iH1Y, iH2X, iH2Y, iH3X, iH3Y;

    fvec xcs, ycs;
    fvec fCurMinXV = fvec(fCurMinX), fCurMinYV = fvec(fCurMinY);
    fvec xc, yc, r;
	fvec r12, r13, r23;
    fvec rx0, rx1, rx2, ry0, ry1,ry2; //rx[3], ry[3];//, x[3], y[3];
    fvec t5, t10, t19, det, t6, t7;
    fvec iH1X, iH1Y, iH2X, iH2Y, iH3X, iH3Y;
//    fvec intXfv, intYfv, intRfv;

    __m128i intXv, intYv, intRv, indXYv;
    __m128i fNofBinsXv = _mm_set1_epi32(fNofBinsX);
    __m128i fNofBinsXYv = _mm_set1_epi32(fNofBinsXY + 1);
    __m128i m128i_0 = _mm_set1_epi32(0);
    fvec fMinDistanceSqv = fvec(fMinDistanceSq);
    fvec fMaxDistanceSqv = fvec(fMaxDistanceSq);
    __m128i fNofBinsRv = _mm_set1_epi32(fNofBinsR + 1);
    __m128i m128i_10 = _mm_set1_epi32(10);

    fvec dxv = fvec(1.0f/fDx), dyv = fvec(1.0f/fDy), drv = fvec(1.0f/fDr);
    fvec fvec05 = fvec(0.5f);
    fvec fvec0 = fvec(0.f);
    Int_t nofHits = fHitInd[iPart].size();
    if (nofHits <= fMinNofHitsInArea) return;
    iPmulNofHits = iPart * nofHitsNorm;

    vector<unsigned short> hitIndPart;
    hitIndPart.assign(fHitInd[iPart].begin(), fHitInd[iPart].end());

    CbmRichHoughHitVec h2, h3;

	for (unsigned short int iHit1 = 0; iHit1 < (nofHits & ~3); iHit1+=4) {
		iH11 = hitIndPart[iHit1];
		iH12 = hitIndPart[iHit1+1];
		iH13 = hitIndPart[iHit1+2];
		iH14 = hitIndPart[iHit1+3];

		iH1X = fvec(fData[iH11]->fX, fData[iH12]->fX, fData[iH13]->fX, fData[iH14]->fX);
		iH1Y = fvec(fData[iH11]->fY, fData[iH12]->fY, fData[iH13]->fY, fData[iH14]->fY);

		for (unsigned short int iHit2 = iHit1 + 1; iHit2 < nofHits; iHit2++) {
			iH2 = hitIndPart[iHit2];
			h2 = fDataV[iH2];
			iH2X = h2.fX;
			iH2Y = h2.fY;

			rx0 = iH1X - iH2X;//rx12
			ry0 = iH1Y- iH2Y;//ry12
			r12 = rx0 * rx0 + ry0 * ry0;
			if ( _mm_comineq_ss(_mm_cmpgt_ss(r12, fMinDistanceSqv), fvec0) == 1) continue;
			if ( _mm_comineq_ss(_mm_cmplt_ss(r12, fMaxDistanceSqv), fvec0) == 1) continue;


			t10 = fvec(fData[iH11]->fX2plusY2 - fData[iH2]->fX2plusY2,
					fData[iH12]->fX2plusY2 - fData[iH2]->fX2plusY2,
					fData[iH13]->fX2plusY2 - fData[iH2]->fX2plusY2,
					fData[iH14]->fX2plusY2 - fData[iH2]->fX2plusY2);
			for (unsigned short int iHit3 = iHit2 + 1; iHit3 < nofHits; iHit3++) {
				//iH3 = hitIndPart[iHit3];
				h3 = fDataV[ hitIndPart[iHit3] ];
//				iH3X = h3.fX;
//				iH3Y = h3.fY;
				t5 = h2.fX2plusY2 - h3.fX2plusY2;

				rx1 = iH1X - iH3X;//rx13
				ry1 = iH1Y - iH3Y;//ry13
				r13 = rx1 * rx1 + ry1 * ry1;
				if ( _mm_comineq_ss(_mm_cmpgt_ss(r13, fMinDistanceSqv), fvec0) == 1) continue;
				if ( _mm_comineq_ss(_mm_cmplt_ss(r13, fMaxDistanceSqv), fvec0) == 1) continue;

				rx2 = iH2X - h3.fX;//rx23
				ry2 = iH2Y - h3.fY;//ry23
				r23 = rx2 * rx2 + ry2 * ry2;
				if ( _mm_comineq_ss(_mm_cmpgt_ss(r23, fMinDistanceSqv), fvec0) == 1) continue;
				if ( _mm_comineq_ss(_mm_cmplt_ss(r23, fMaxDistanceSqv), fvec0) == 1) continue;

				t19 = fvec05 / (rx2*ry0 - rx0*ry2);

				xc = (t5 * ry0 - t10 * ry2) * t19;
				xcs = xc - fCurMinXV;
				//intX = int( xcs *dx);
				//if (intX < 0 || intX >= fNofBinsX ) continue;

				yc = (t10 * rx2 - t5 * rx0) * t19;
				ycs = yc - fCurMinYV;
				//intY = int( ycs *dy);
				//if (intY < 0 || intY >= fNofBinsY ) continue;

				 //radius calculation
				t6 = iH1X - xc;
				t7 = iH1Y - yc;
				r = sqrt(t6 * t6 + t7 * t7);

				//intR = int(r *dr);
				//if (intR < 0 || intR > fNofBinsR) continue;
				//indXY = intX * fNofBinsX + intY;
//				intXfv = xcs *dxv;
//				intYfv = ycs *dyv;
//				intRfv = r *drv;
				intRv = _mm_cvtps_epi32(r *drv);
//				if (_mm_movemask_epi8( _mm_cmpgt_epi32(intRv, m128i_10) ) == 0x0000)continue;
//				if (_mm_movemask_epi8( _mm_cmplt_epi32(intRv, fNofBinsRv) ) == 0x0000) continue;

				intXv = _mm_cvtps_epi32(xcs *dxv);
				intYv = _mm_cvtps_epi32(ycs *dyv);
				indXYv = intXv * fNofBinsXv + intYv;

//				if (_mm_movemask_epi8( _mm_cmpgt_epi32(indXYv, m128i_0) ) == 0x0000) continue;
//				if (_mm_movemask_epi8( _mm_cmplt_epi32(indXYv, fNofBinsXYv) ) == 0x0000) continue;


				indXY = (int*)&indXYv;
				intR = (int*)&intRv;

				if (intR[0] > 9 && intR[0] < fNofBinsR &&
					indXY[0] > -1 && indXY[0] < fNofBinsXY){
					fHist[indXY[0]]++;
					fHistR[intR[0]]++;
				}
				if (intR[1] > 9 && intR[1] < fNofBinsR &&
					indXY[1] > -1 && indXY[1] < fNofBinsXY){
					fHist[indXY[1]]++;
					fHistR[intR[1]]++;
				}

				if (intR[2] > 9 && intR[2] < fNofBinsR &&
					indXY[2] > -1 && indXY[2] < fNofBinsXY){
					fHist[indXY[2]]++;
					fHistR[intR[2]]++;
				}

				if (intR[3] > 9 && intR[3] < fNofBinsR &&
					indXY[3] > -1 && indXY[3] < fNofBinsXY){
					fHist[indXY[3]]++;
					fHistR[intR[3]]++;
				}
			}//iHit1
		}//iHit2
	}//iHit3
}



void CbmRichRingFinderHoughSimd::HoughTransformReconstruction()
{
    Int_t indmin, indmax;
    Float_t x0, y0;

    fDataV.clear();
    fDataV.reserve(fData.size());
    for (UInt_t iHit = 0; iHit < fData.size(); iHit++){
    	CbmRichHoughHitVec h;
    	h.fX =  fvec(fData[iHit].fX);
    	h.fY =  fvec(fData[iHit].fY);
    	h.fX2plusY2 =  fvec(fData[iHit].fX2plusY2);
    	fDataV.push_back(h);
    }

    for (UInt_t iHit = 0; iHit < fData.size(); iHit++)
    {
        if (fData[iHit].fIsUsed == true) continue;

		x0 = fData[iHit].fX;
		y0 = fData[iHit].fY;

        fCurMinX = x0  - fMaxDistance;
		fCurMinY = y0 - fMaxDistance;

		DefineLocalAreaAndHits(x0, y0, &indmin, &indmax);
		HoughTransform(indmin, indmax);
		FindPeak(indmin, indmax);

    }//main loop over hits
}
