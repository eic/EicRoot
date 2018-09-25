 /*
*  Description : Implementation for concrete RICH ring selection algorithm:
*                reject rings using a trained neural net (input file with weights needed!)
*                store resulting value (0-1) in "SelectionNN":
*                0 = good rings
*                1 = rings to be rejected
*                --> choose a value in between depending on required purity/ efficiency
*
*  Author : Semen Lebedev
*  E-mail : S.Lebedev@gsi.de
*  */

#ifndef CBM_RICH_RING_SELECT_NEURALNET_H
#define CBM_RICH_RING_SELECT_NEURALNET_H

#include "NNfunction.h"
#include "CbmRichRingLight.h"
#include "CbmRichRingSelectImplLight.h"

#include <iostream>

using std::cout;
using std::endl;


class CbmRichRingSelectNeuralNet
{

	const char* fNeuralNetWeights;

public:
  	CbmRichRingSelectNeuralNet (const char* NNFile);// Standard constructor
	~CbmRichRingSelectNeuralNet();

	virtual void Init();
	void DoSelect(CbmRichRingLight* ring);

	NNfunction* fNNfunction;
	CbmRichRingSelectImplLight* fSelectImpl;
};

#endif


inline CbmRichRingSelectNeuralNet::CbmRichRingSelectNeuralNet (const char* NNFile )
{
    fNeuralNetWeights = NNFile;
}


inline CbmRichRingSelectNeuralNet::~CbmRichRingSelectNeuralNet()
{}

inline void CbmRichRingSelectNeuralNet::Init ()
{
    fSelectImpl = new CbmRichRingSelectImplLight();
    fNNfunction = new NNfunction();
}

// -----   Exec   ----------------------------------------------------
inline void CbmRichRingSelectNeuralNet::DoSelect(CbmRichRingLight* ring)
{
    if (ring->GetRadius() >= 10.f || ring->GetRadius() <= 0.f ||
        ring->GetNofHits() <= 5.f ||
        ring->GetRadialPosition() <= 0.f || ring->GetRadialPosition() >= 999.f ){

        ring->SetSelectionNN(-1.f);
        return;
    }
    ring->SetNofHitsOnRing(fSelectImpl->GetNofHitsOnRingCircle(ring));
    if (ring->GetNofHitsOnRing() < 5){
    	ring->SetSelectionNN(-1.f);
    	return;
    }

    ring->SetAngle(fSelectImpl->GetAngle(ring));
    if (ring->GetAngle() < 0.f || ring->GetAngle() > 6.3f){
    	ring->SetSelectionNN(-1.f);
    	return;
    }

    double nnPar[10];
    nnPar[0] =  ring->GetNofHits();
    nnPar[1] =  ring->GetAngle();
    nnPar[2] =  ring->GetNofHitsOnRing();
    nnPar[3] =  ring->GetRadialPosition();
    nnPar[4] =  ring->GetRadius();
    nnPar[5] =  ring->GetChi2() / ring->GetNDF();

    float nnEval = fNNfunction->Value(0,nnPar);

    ring->SetSelectionNN(nnEval);
}

