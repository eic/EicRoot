/**
* \file CbmRichRingFinderHoughImpl.h
*
* \brief Ring finder implementation based on Hough Transform method.
*
* \author Semen Lebedev
* \date 2008
**/

#ifndef CBM_RICH_RING_FINDER_HOUGH_IMPL
#define CBM_RICH_RING_FINDER_HOUGH_IMPL

#include "CbmRichRingLight.h"
#include "CbmRichRingFinderData.h"

#include <vector>
#include <map>

using std::vector;

class CbmRichRingFitterCOP;
class CbmRichRingSelectAnn;


/**
* \class CbmRichRingFinderHoughImpl
*
* \brief Ring finder implementation based on Hough Transform method.
*
* \author Semen Lebedev
* \date 2008
**/
class CbmRichRingFinderHoughImpl
{

protected:
	static const unsigned short MAX_NOF_HITS = 65000; // maximum number of hits in RICH detector

   // parameters of the Hough Transform algorithm
	unsigned short fNofParts; // number of groups of hits for HT

	float fMaxDistance; // maximum distance between two hits
	float fMinDistance; // minimum distance between two hits
	float fMinDistanceSq; // = fMinDistance*fMinDistance
	float fMaxDistanceSq; // = fMaxDistance*fMaxDistance

	float fMinRadius; // minimum radius of the ring
	float fMaxRadius; // maximum radius of the ring

	float fDx; // x bin width of the ring center histogram
	float fDy; // y bin width of the ring center histogram
	float fDr; // width of the ring radius histogram
	unsigned short fNofBinsX; // number of bins in X direction
	unsigned short fNofBinsY; // number of bins in Y direction
	unsigned short fNofBinsXY; // fNofBinsX*fNofBinsY

	unsigned short fHTCut; // cut number of entries in maximum bin of XY histogram

	unsigned short fNofBinsR;// number of bins in radius histogram
	unsigned short fHTCutR; // cut number of entries in maximum bin of Radius histogram

	unsigned short fMinNofHitsInArea; // minimum number of hits in the local area

	float fRmsCoeffEl;
	float fMaxCutEl;
	float fRmsCoeffCOP;
	float fMaxCutCOP;

	float fAnnCut; //remove found hits only for good quality rings
	float fUsedHitsAllCut; // percent of used hits

	float fCurMinX; // current minimum X position of the local area
	float fCurMinY; // current minimum Y position of the local area

	vector<CbmRichHoughHit> fData; // Rich hits
	vector<unsigned short> fHist; // XY histogram
	vector<unsigned short> fHistR; // Radius histogram
	vector< vector<unsigned short> > fHitInd; // store hit indexes for different group of hits
	vector<CbmRichRingLight*> fFoundRings; // collect found rings
	CbmRichRingFitterCOP* fFitCOP; // COP ring fitter
	CbmRichRingSelectAnn* fANNSelect; // ANN selection criteria

public:
	/**
	 * \brief Standard constructor.
	 */
  	CbmRichRingFinderHoughImpl ();

   /**
    * \brief Distructor.
    */
	virtual ~CbmRichRingFinderHoughImpl();

	/**
	 * \brief Set parameters of the algorithm.
	 */
	void SetParameters();

	/**
	 * \brief Calculate circle center and radius.
	 * \param[in] x[] Array of 3 X coordinates.
	 * @param[in] y[] Array of 3 Y coordinates.
	 * @param[out] xc X coordinate of the ring center.
	 * @param[out] yc Y coordinate of the ring center.
	 * @param[out] r Ring radius.
	 */
	void CalculateRingParameters(
	      float x[],
			float y[],
			float *xc,
			float *yc,
			float *r);

	/**
	 * \brief Run HT for each hit.
	 */
	virtual void HoughTransformReconstruction();

	/**
	 * \brief Find hits in a local area.
	 * \param[in] x0 X coordinate of the local area center.
	 * \param[in] y0 Y coordinate of the local area center.
	 * \param[out] indmin Minimum index of the hit in local area.
	 * \param[out] indmax Maximum index of the hit in local area.
	 */
	virtual void DefineLocalAreaAndHits(
	      float x0,
	      float y0,
	      int *indmin,
	      int *indmax);

	/**
	 * \brief Run HoughTransformGroup for each group of hits.
	 * \param[in] indmin Minimum index of the hit in local area.
	 * \param[in] indmax Maximum index of the hit in local area.
	 */
	virtual void HoughTransform(
	      unsigned short indmin,
			unsigned short indmax);

	/**
	 * \brief Main procedure for Hough Transform.
	 * \param[in] indmin Minimum index of the hit in local area.
    * \param[in] indmax Maximum index of the hit in local area.
    * \param[in] iPart Index of the hit group.
    */
	virtual void HoughTransformGroup(
	      unsigned short indmin,
			unsigned short indmax,
			int iPart);

	/**
	 * \brief Find peak in the HT histograms.
    * \param[in] indmin Minimum index of the hit in local area.
    * \param[in] indmax Maximum index of the hit in local area.
	 */
	void FindPeak(
	      int indmin,
	      int indmax);

	/**
	 * \brief Ring selection procedure.
	 */
   void RingSelection();

   /**
    * \brief Reassign shared hits from two rings to only one of the rings.
    * \param[in,out] ringInd1 Index of the first ring.
    * \param[in,out] ringInd2 Index of the second ring.
    */
   void ReAssignSharedHits(
         int ringInd1,
         int ringInd2);

   /**
    * \brief Return hit indez in the internal Array.
    * \param[in] hitInd Index in TClonesArray.
    */
   int GetHitIndex(
         unsigned short hitInd);

   /**
    * \brief Set fIsUsed flag to true for hits attached to the ring.
    * \param[in] indmin Minimum index of the hit in local area.
    * \param[in] indmax Maximum index of the hit in local area.
    * \param[in] ring Found ring.
    */
   void RemoveHitsAroundRing(
         int indmin,
         int indmax,
         CbmRichRingLight* ring);

   /**
    * Initialize algorithm parameters.
    */
	void Init();

	/**
	 * \brief Start point to run algorithm.
	 */
	void DoFind();

	/**
	 * \brief Set array of hits.
	 * \param[in] data Array of hits.
	 */
	void SetData(
	      const vector<CbmRichHoughHit>& data)
	{
		fData.clear();
		fData = data;
	}

	/**
	 * \brief Return vector of found rings.
	 */
	vector<CbmRichRingLight*>& GetFoundRings()
	{
		return fFoundRings;
	}

private:
   /**
    * \brief Copy constructor.
    */
   CbmRichRingFinderHoughImpl(const CbmRichRingFinderHoughImpl&);

   /**
    * \brief Assignment operator.
    */
   CbmRichRingFinderHoughImpl& operator=(const CbmRichRingFinderHoughImpl&);
};
#endif
