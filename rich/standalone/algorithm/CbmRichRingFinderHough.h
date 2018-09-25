// --------------------------------------------------------------------------------------
// -----                 CbmRichRingFinderHough source file                         -----
// ----- Algorithm idea: G.A. Ososkov (ososkov@jinr.ru) and Semen Lebedev (s.lebedev@gsi.de)                            -----
// ----- Implementation: Semen Lebedev (s.lebedev@gsi.de)-----

#ifndef CBM_RICH_RING_FINDER_HOUGH_H
#define CBM_RICH_RING_FINDER_HOUGH_H


#include "CbmRichRingLight.h"
#include "CbmRichRingFinderHoughImpl.h"
#include "CbmRichRingFinderHoughSimd.h"
#include <vector>

class CbmRichRingFinderHough{

protected:
	int fNEvent; /// event number
	int fRingCount;

	CbmRichRingFinderHoughImpl *fHTImpl1;
	CbmRichRingFinderHoughImpl *fHTImpl2;

	double fExecTime;//evaluate execution time

public:
  	CbmRichRingFinderHough ();
	virtual ~CbmRichRingFinderHough();
	int DoFind(const std::vector<CbmRichHoughHit>& data);
	int DoFindParallel(const std::vector<CbmRichHoughHit>& data);
};
#endif // CBM_RICH_RING_FINDER_HOUGH_H
