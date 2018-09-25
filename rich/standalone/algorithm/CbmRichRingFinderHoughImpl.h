// --------------------------------------------------------------------------------------
// -----                 CbmRichRingFinderHoughImpl source file                         -----
// ----- Algorithm idea: G.A. Ososkov (ososkov@jinr.ru) and Semen Lebedev (s.lebedev@gsi.de)                            -----
// ----- Implementation: Semen Lebedev (s.lebedev@gsi.de)-----

#ifndef CBM_RICH_RING_FINDER_HOUGH_IMPL_H
#define CBM_RICH_RING_FINDER_HOUGH_IMPL_H

#include "CbmRichRingLight.h"
#include "CbmRichRingFitterCOPLight.h"
#include "CbmRichRingSelectNeuralNet.h"

#include <vector>
#include <map>
#include <functional>

class CbmRichHoughHit {
public:
	CbmRichHitLight fHit;
	float fX2plusY2;
    unsigned short fId;
	bool fIsUsed;
};

class CbmRichHoughHitCmpUp:
       public std::binary_function<
	          const CbmRichHoughHit,
	          const CbmRichHoughHit,
	          bool>
{
public:
	bool operator()(const CbmRichHoughHit &m1, const CbmRichHoughHit &m2) const {
		return m1.fHit.fX < m2.fHit.fX;
	}
};

class CbmRichRingComparatorMore:
       public std::binary_function<
	          const CbmRichRingLight*,
	          const CbmRichRingLight*,
	          bool>
{
public:
	bool operator()(const CbmRichRingLight* ring1, const CbmRichRingLight* ring2) const {
		return ring1->GetSelectionNN() > ring2->GetSelectionNN();
	}
};

class CbmRichRingFinderHoughImpl{

protected:
	static const unsigned short kMAX_NOF_HITS = 65500;
	static const unsigned short fNofParts = 2;

	float fMaxDistance;
	float fMinDistance;
    float fMinDistanceSq; ///= fMinDistance*fMinDistance
    float fMaxDistanceSq;

	float fMinRadius;
	float fMaxRadius;

	float fDx;
	float fDy;
	float fDr;
	unsigned short  fNofBinsX;
	unsigned short  fNofBinsY;
	unsigned short  fNofBinsXY;
	unsigned short  fHTCut;
	unsigned short  fHitCut;

	unsigned short  fNofBinsR;
	unsigned short  fHTCutR;
	unsigned short  fHitCutR;

	unsigned short fMinNofHitsInArea;

	float fRmsCoeffEl;
	float fMaxCutEl;
	float fRmsCoeffCOP;
	float fMaxCutCOP;

	float fAnnCut;
	float fUsedHitsCut;
	float fUsedHitsAllCut;

	float fCurMinX;
	float fCurMinY;

	std::vector<CbmRichHoughHit> fData;  ///Rich hits
	std::vector<CbmRichHoughHit> fDataPart1;  ///Rich hits
	std::vector<CbmRichHoughHit> fDataPart2;  ///Rich hits

	std::vector<unsigned short> fHist;
	std::vector<unsigned short> fHistR;
	std::vector< std::vector<unsigned short> > fHitInd;

	std::vector<CbmRichRingLight*> fFoundRings;///collect found rings

	CbmRichRingFitterCOPLight* fFitCOP;
	CbmRichRingSelectNeuralNet* fANNSelect;

public:
  	CbmRichRingFinderHoughImpl ();
	virtual ~CbmRichRingFinderHoughImpl();
	void SetParameters();
	virtual void HoughTransformReconstruction();
	virtual void DefineLocalAreaAndHits(float x0, float y0, int *indmin, int *indmax);
	virtual void HoughTransform(unsigned short indmin,
			unsigned short indmax);
	virtual void HoughTransformGroup(unsigned short indmin,
			unsigned short indmax, int iPart);
	void FindPeak(int indmin, int indmax);
    void RingSelection();
    void ReAssingSharedHits(int ringInd1, int ringInd2);
    int GetHitIndex(unsigned short hitInd);
    void RemoveHitsAroundRing(int indmin, int indmax, CbmRichRingLight* ring);
	void Init();
	void DoFind();

	void SetData(const std::vector<CbmRichHoughHit>& data){
		fData.clear();
		fData = data;
	}

	std::vector<CbmRichRingLight*>& GetFoundRings(){
		return fFoundRings;
	}
};
#endif // CBM_RICH_RING_FINDER_HOUGH_IMPL_H
