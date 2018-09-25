/**
* \file CbmRichRingSelectAnn.h
*
* \brief Implementation for concrete RICH ring selection algorithm:
* reject rings using a trained neural net (input file with weights needed!)
* store resulting value (0-1) in "SelectionNN":
* 0 = good rings
* 1 = rings to be rejected
* --> choose a value in between depending on required purity/ efficiency
*
* \author Semen Lebedev
* \date 2008
**/

#ifndef CBM_RICH_RING_SELECT_ANN
#define CBM_RICH_RING_SELECT_ANN

#include <string>

class CbmRichRingLight;
class CbmRichRingSelectImpl;
class NNfunction;
class TMultiLayerPerceptron;
using namespace std;

/**
* \class CbmRichRingSelectAnn
*
* \brief Implementation for concrete RICH ring selection algorithm:
* reject rings using a trained neural net (input file with weights needed!)
* store resulting value (0-1) in "SelectionNN":
* 0 = good rings
* 1 = rings to be rejected
* --> choose a value in between depending on required purity/ efficiency
*
* \author Semen Lebedev
* \date 2008
**/
class CbmRichRingSelectAnn
{
private:
	std::string fAnnWeights;
	TMultiLayerPerceptron* fNN;
	CbmRichRingSelectImpl* fSelectImpl;

public:
	/**
	 * \brief Standard constructor.
	 */
  	CbmRichRingSelectAnn();

   /**
    * \brief Destructor.
    */
	virtual ~CbmRichRingSelectAnn();

	/**
	 * \brief Initialize ANN.
	 */
	virtual void Init();

	/**
	 * \Perform selection.
	 * \param[in,out] ring Found and fitted ring.
	 */
	void DoSelect(
	      CbmRichRingLight* ring);

	/**
	 * \brief Set path to the file with ANN weights.
	 * \param[in] fileName Path to the file name with ANN weights.
	 */
	void SetAnnWeights(const string& fileName){fAnnWeights = fileName;}

private:
   /**
    * \brief Copy constructor.
    */
   CbmRichRingSelectAnn(const CbmRichRingSelectAnn&);

   /**
    * \brief Assignment operator.
    */
   CbmRichRingSelectAnn& operator=(const CbmRichRingSelectAnn&);
};

#endif
