// --------------------------------------------------------------------------------------
// CbmRichRingFinderHough source file
// Base class for ring finders based on on HT method
// Implementation: Semen Lebedev (s.lebedev@gsi.de)

#include "CbmRichRingFinderHough.h"

#include "tbb/task_scheduler_init.h"
#include "tbb/task.h"
#include "tbb/tick_count.h"
#include "tbb/parallel_invoke.h"


#include <iostream>
#include <cmath>
#include <set>
#include <algorithm>
#include <fstream>

using std::cout;
using std::endl;
using std::vector;

class FinderTask{
	CbmRichRingFinderHoughImpl* fHTImpl;
public:
	FinderTask(CbmRichRingFinderHoughImpl* HTImpl) {
		fHTImpl = HTImpl;
	}
	void operator()() const {
		fHTImpl->DoFind();
	}
};

// -----   Standard constructor   ------------------------------------------
CbmRichRingFinderHough::CbmRichRingFinderHough  ( )
{
    fRingCount = 0;
    fNEvent = 0;

	fHTImpl1 = new CbmRichRingFinderHoughSimd();
	fHTImpl1->Init();

	fHTImpl2 = new CbmRichRingFinderHoughSimd();
	fHTImpl2->Init();

	tbb::task_scheduler_init init;
}

CbmRichRingFinderHough::~CbmRichRingFinderHough()
{
	delete fHTImpl1;
	delete fHTImpl2;
}

int CbmRichRingFinderHough::DoFind(const vector<CbmRichHoughHit>& data)
{
	fNEvent++;
	std::vector<CbmRichHoughHit> UpH;
	std::vector<CbmRichHoughHit> DownH;
    fRingCount = 0;

	UpH.reserve(data.size()/2);
	DownH.reserve(data.size()/2);

	for(int iHit = 0; iHit < data.size(); iHit++) {
		CbmRichHoughHit hit = data[iHit];
		if (hit.fHit.fY >= 0)
			UpH.push_back(hit);
		else
			DownH.push_back(hit);
	}

	fHTImpl1->SetData(UpH);
	fHTImpl1->DoFind();
	int nofFoundRings = fHTImpl1->GetFoundRings().size();
	UpH.clear();

	fHTImpl1->SetData(DownH);
	fHTImpl1->DoFind();
	nofFoundRings += fHTImpl1->GetFoundRings().size();
	DownH.clear();

	cout << "NofFoundRings = " << nofFoundRings << endl;
}

int CbmRichRingFinderHough::DoFindParallel(const vector<CbmRichHoughHit>& data)
{
	fNEvent++;
	std::vector<CbmRichHoughHit> UpH;
	std::vector<CbmRichHoughHit> DownH;
    fRingCount = 0;

	UpH.reserve(data.size()/2);
	DownH.reserve(data.size()/2);

	for(int iHit = 0; iHit < data.size(); iHit++) {
		CbmRichHoughHit hit = data[iHit];
		if (hit.fHit.fY >= 0)
			UpH.push_back(hit);
		else
			DownH.push_back(hit);
	}

	fHTImpl1->SetData(UpH);
	fHTImpl2->SetData(DownH);

	//fHTImpl1->DoFind();
	//fHTImpl2->DoFind();

	FinderTask a(fHTImpl1);
	FinderTask b(fHTImpl2);
	tbb::parallel_invoke(a, b);

	int nofFoundRings = fHTImpl1->GetFoundRings().size();
	nofFoundRings += fHTImpl2->GetFoundRings().size();

	UpH.clear();
	DownH.clear();

	cout << "NofFoundRings = " << nofFoundRings << endl;
}
