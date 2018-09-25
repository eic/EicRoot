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

using std::vector;

class CbmRichHitLight
{
public:
   /**
    * \brief Default constructor.
    */
   CbmRichHitLight():
      fX(0.),
      fY(0.){}

   /**
    * \brief Distructor.
    */
   virtual ~CbmRichHitLight(){}

   /**
    * \brief Constructor with initialization.
    * \param x X coordinate of hit.
    * \param y Y coordinate of hit.
    */
   CbmRichHitLight(
         float x,
         float y):
      fX(x),
      fY(y) {}

	float fX; // x coordinate of the hit
	float fY; // y coordinate of the hit
};

class CbmRichRingLight
{
public:

   /**
    * \brief Standard constructor.
    */
   CbmRichRingLight():
      fHits(),
      fHitIds(),

      fCenterX(0.f),
      fCenterY(0.f),
      fRadius(0.f),

      fAaxis(0.f),
      fBaxis(0.f),
      fPhi(0.f),

      fAPar(0.f),
      fBPar(0.f),
      fCPar(0.f),
      fDPar(0.f),
      fEPar(0.f),
      fFPar(0.f),

      fRecFlag(0),

      fChi2(0.f),
      fAngle(0.f),
      fNofHitsOnRing(0),
      fSelectionNN(0.f)

   {
      fHits.reserve(30);
      fHitIds.reserve(30);
   }

   /**
    * \brief Destructor.
    */
   virtual ~CbmRichRingLight()
   {
      fHits.clear();
      fHitIds.clear();
   }

   /**
    * \brief Add new hit to the ring.
    * \param[in] hit New hit to be added.
    * \param[in] id index of hit in TClonesArray.
    */
   void AddHit(
         CbmRichHitLight hit,
         unsigned short id = -1)
   {
      fHits.push_back(hit);
      fHitIds.push_back(id);
   }

   /**
    * \brief Remove hit from the ring.
    * \param[in] hitId index of hit in TClonesArray.
    * \return true if hit was removed, false if hit was NOT removed.
    */
   bool RemoveHit(
         int hitId)
   {
      vector<unsigned short>::iterator it;
      for (it = fHitIds.begin(); it!= fHitIds.end(); it++){
         if (hitId == *it){
            fHitIds.erase(it);
            return true;
         }
      }
      return false;
   }

   /**
    * \brief Return number of hits in ring.
    */
	int GetNofHits() const {return fHitIds.size(); }

	/**
	 * \brief Return hit by the index.
	 * \param[in] ind Index of hit in local array.
	 */
	CbmRichHitLight GetHit(int ind) {return fHits[ind];}

	/**
	 * \brief Return hit index in TClonesArray.
	 * \param[in] ind Index of hit in local array.
	 */
	unsigned short GetHitId(int ind) {return fHitIds[ind];}

	void SetCenterX(float x) {fCenterX = x;}
   void SetCenterY(float y) {fCenterY = y;}
	void SetRadius(float r) {fRadius = r;}

   /**
    * \brief Set all 3 circle parameters.
    * \param[in] x X coordinate of circle center.
    * \param[in] y Y coordinate of circle center.
    * \param[in] r Radius of circle.
    */
	void SetXYR(
	      float x,
	      float y,
	      float r)
	{
	   fCenterX = x;
	   fCenterY = y;
	   fRadius = r;
	}

   /**
    * \brief Set all 5 ellipse parameters.
    * \param[in] x X coordinate of ellipse center.
    * \param[in] y Y coordinate of ellipse center.
    * \param[in] a Major half axis of ellipse.
    * \param[in] b Minor half axis of ellipse.
    * \param[in] p Rotation angle of ellipse [rad].
    */
   void SetXYABP(
         float x,
         float y,
         float a,
         float b,
         float p)
   {
      fCenterX = x;
      fCenterY = y;
      fAaxis = a;
      fBaxis = b;
      fPhi = p;
   }

	void SetChi2(float chi2) {fChi2 = chi2;}
	void SetAngle( float angle) {fAngle = angle;}
	void SetNofHitsOnRing(unsigned short onring) {fNofHitsOnRing = onring;}
	void SetSelectionNN (float selectionNN ) {fSelectionNN = selectionNN;}

	float GetCenterX() const {return fCenterX;}
	float GetCenterY() const {return fCenterY;}
	float GetRadius() const {return fRadius;}

   float GetAaxis() const {return fAaxis;}
   float GetBaxis() const {return fBaxis;}
   float GetPhi() const {return fPhi;}
   void SetAaxis(double a) {fAaxis = a;}
   void SetBaxis(double b) {fBaxis = b;}
   void SetPhi(double phi) {fPhi = phi;}

   /**
    * \brief Calculate and return X coordinate of the first focus.
    */
   double GetXF1() const
   {
      double c = sqrt(fAaxis * fAaxis - fBaxis * fBaxis);
      double xc = c*cos(fabs(fPhi));

      return fCenterX+xc;
   }

   /**
    * \brief Calculate and return Y coordinate of the first focus.
    */
   double GetYF1() const
   {
      double c = sqrt(fAaxis * fAaxis - fBaxis * fBaxis);
      double yc = c * sin(fabs(fPhi));
      if (fPhi >=0){
         return fCenterY+yc;
      }else{
         return fCenterY-yc;
      }
   }

   /**
    * \brief Calculate and return X coordinate of the second focus.
    */
   double GetXF2() const
   {
      double c = sqrt(fAaxis * fAaxis - fBaxis * fBaxis);
      double xc = c*cos(fabs(fPhi));

      return fCenterX-xc;
   }

   /**
    * \brief Calculate and return Y coordinate of the second focus.
    */
   double GetYF2() const
   {
      double c = sqrt(fAaxis * fAaxis - fBaxis * fBaxis);
      double yc = c * sin(fabs(fPhi));
      if (fPhi >= 0){
         return fCenterY - yc;
      }else{
         return fCenterY + yc;
      }
   }

   /**
    * \brief Set all 6 parameters of curve equation Axx+Bxy+Cyy+Dx+Ey+F.
    * \param[in] a A parameter.
    * \param[in] b B parameter.
    * \param[in] c C parameter.
    * \param[in] d D parameter.
    * \param[in] e E parameter.
    * \param[in] f F parameter.
    */
   void SetABCDEF(
         float a,
         float b,
         float c,
         float d,
         float e,
         float f)
   {
      fAPar = a;
      fBPar = b;
      fCPar = c;
      fDPar = d;
      fEPar = e;
      fFPar = f;
   }

   float GetAPar() const {return fAPar;}
   float GetBPar() const {return fBPar;}
   float GetCPar() const {return fCPar;}
   float GetDPar() const {return fDPar;}
   float GetEPar() const {return fEPar;}
   float GetFPar() const {return fFPar;}

	float GetSelectionNN() const {return fSelectionNN;}
	float GetChi2() const {return fChi2;}

	/**
	 * \brief Return radial position of the ring.
	 */
	float GetRadialPosition() const{
		if (fCenterY > 0.f){
		   return sqrt(fCenterX * fCenterX + (fCenterY - 110.f) * (fCenterY - 110.f));
		} else {
		   return sqrt(fCenterX * fCenterX + (fCenterY + 110.f) * (fCenterY + 110.f));
		};
		return 0.;
	}

	float GetAngle() const {return fAngle;}

	unsigned short GetNofHitsOnRing() const {return fNofHitsOnRing;}

	int GetRecFlag() const {return fRecFlag;}
	void SetRecFlag(int r) { fRecFlag = r;}

private:
   vector<CbmRichHitLight> fHits; // STL container for CbmRichHitLight
   vector<unsigned short> fHitIds; // STL container for hit indexes

   float fCenterX;
	float fCenterY;
	float fRadius;

	float fAaxis;
	float fBaxis;
	float fPhi;

   float fAPar; // Axx+Bxy+Cyy+Dx+Ey+F
   float fBPar;
   float fCPar;
   float fDPar;
   float fEPar;
   float fFPar;

   int fRecFlag;

	float fChi2;
	float fAngle;
	unsigned short fNofHitsOnRing;
	float fSelectionNN;
};

#endif /* CBMRICHRINGLIGHT_H_ */
