/*
 * CbmRichRingLight.h
 *
 *  Created on: 09.03.2010
 *  Author: Semen Lebedev
 */

#ifndef CBMRICHRINGLIGHT_H_
#define CBMRICHRINGLIGHT_H_

#include <vector>
#include <cmath>

class CbmRichHitLight
{
public:
	float fX;
	float fY;
};

class CbmRichRingLight
{
public:
    CbmRichRingLight(){
    	fHitCollection.reserve(30);
    	fHitIdCollection.reserve(30);
    }

	~CbmRichRingLight(){
		fHitCollection.clear();
	}

	void AddHit(CbmRichHitLight pHit, unsigned short id) {
		fHitCollection.push_back(pHit);
		fHitIdCollection.push_back(id);

	}
	bool RemoveHit(int hitId){
		//Int_t nofHits = fHitCollection.size();
		std::vector<unsigned short>::iterator it;
		for (it = fHitIdCollection.begin(); it!=fHitIdCollection.end(); it++){
			if (hitId == *it){
				fHitIdCollection.erase(it);
				return true;
			}
		}
		return false;
	}

	int GetNofHits() const {return fHitIdCollection.size(); }
	CbmRichHitLight GetHit(int i) {return fHitCollection[i];}
	unsigned short GetHitId(int i) {return fHitIdCollection[i];}

	void SetCenterX(float x) {fCenterX = x;}
    void SetCenterY(float y) {fCenterY = y;}
	void SetRadius(float r) {fRadius = r;}
	void SetChi2(float chi2) {fChi2 = chi2;}
	void SetAngle( float angle) {fAngle = angle;}
	void SetNofHitsOnRing(unsigned short onring) {fNofHitsOnRing = onring;}
	/** number between -1 and 1: -1 = fake ring, 1 = good ring (selection by neural net)*/
	void SetSelectionNN (float selectionNN ) {fSelectionNN = selectionNN;}
	void SetRecFlag(short recFlag){fRecFlag = recFlag;}

	float GetCenterX() const {return fCenterX;}
	float GetCenterY() const {return fCenterY;}
	float GetRadius() const {return fRadius;}
	float GetSelectionNN() const {return fSelectionNN;}
	float GetChi2() const {return fChi2;}
	float GetNDF() const {return (float)(GetNofHits()-5);}
	float GetRadialPosition() const{
		if (fCenterY > 0.f){
		       return sqrt(fCenterX*fCenterX +(fCenterY - 110.f)*(fCenterY - 110.f));
		   } else {
		       return sqrt(fCenterX*fCenterX+(fCenterY + 110.f)*(fCenterY + 110.f));
		};
	}

	float GetAngle() const {return fAngle;}
	unsigned short GetNofHitsOnRing() const {return fNofHitsOnRing;}
	short GetRecFlag(){return fRecFlag;}

private:
   std::vector<CbmRichHitLight> fHitCollection;
   std::vector<unsigned short> fHitIdCollection; /** STL container to hold the hit indexes */
protected:
    float fCenterX;
	float fCenterY;
	float fRadius;
	float fChi2;
	float fAngle;
	unsigned short fNofHitsOnRing;
	short fRecFlag;
	float fSelectionNN;
};

#endif /* CBMRICHRINGLIGHT_H_ */
