
#include <iostream>
#include <vector>
#include <fstream>

#include "CbmRichRingFinderHough.h"
#include "Stopwatch.h"

#include "tbb/tick_count.h"

int main( int argc, const char* argv[] )
{
	cout << "-I- Start ring finder" << endl;

	cout <<"-I- Read data from text file" << endl;

	std::ifstream fin("../data/events.txt");
	int nofEvents = 100;
	int evNum, nhits;
	float x,y;
	std::vector<CbmRichHoughHit> data;
	std::vector< std::vector<CbmRichHoughHit> > dataAll;
	data.reserve(1000);
	for (int iE = 0; iE < nofEvents; iE++){
		fin >> evNum >> nhits;
		cout << "event " << evNum << ", nof hits = " << nhits <<endl;
		data.clear();
		for(int iHit = 0; iHit < nhits; iHit++) {
			CbmRichHoughHit tempPoint;
			fin >> x >>y;
			tempPoint.fHit.fX = x;
			tempPoint.fHit.fY = y;
			tempPoint.fX2plusY2 = x * x + y * y;
			tempPoint.fId = iHit;
			tempPoint.fIsUsed = false;
			data.push_back(tempPoint);
		}
		dataAll.push_back(data);
	}

	//tbb::tick_count t0 = tbb::tick_count::now();

	//Stopwatch timer;

	CbmRichRingFinderHough* finder = new CbmRichRingFinderHough();
	timer.Start();
	for (int iE = 0; iE < dataAll.size(); iE++){
		cout << "-I- Event:" << iE << endl;
		finder->DoFind(dataAll[iE]);
	}
	//tbb::tick_count t1 = tbb::tick_count::now();
	//cout << "Exec time per event " << 1000.*(t1-t0).seconds()/nofEvents << " ms" << endl;


	timer.Stop();
	cout << "CPU time = " << 1000.*timer.CpuTime() / nofEvents<< " ms per event" << endl;
	cout << "Real time = " << 1000.*timer.RealTime() / nofEvents<< " ms per event" << endl;


	dataAll.clear();
	delete finder;
}
