/**
* \file CbmRichElectronIdAnn.h
*
* \brief Implementation of the electron identification algorithm in the RICH detector using
* Artificial Neural Network(ANN).
*
* \author Semen Lebedev
* \date 2008
**/

#ifndef CBM_RICH_ELECTRONID_ANN
#define CBM_RICH_ELECTRONID_ANN

#include <string>

class CbmRichRing;
class TMultiLayerPerceptron;

using std::string;

/**
* \class CbmRichElectronIdAnn
*
* \brief Implementation of the electron identification algorithm in the RICH detector using
* Artificial Neural Network(ANN).
*
* \author Semen Lebedev
* \date 2008
**/
class CbmRichElectronIdAnn
{
private:
   string fAnnWeights; // path to the file with weights for ANN
   TMultiLayerPerceptron* fNN; // Pointer to the ANN
public:

   /**
    * \brief Standard constructor.
    */
   CbmRichElectronIdAnn();

   /**
    * \brief Destructor.
    */
   virtual ~CbmRichElectronIdAnn();

   /**
    * \brief Initialize ANN before use.
    */
   void Init();

   /**
    * \brief Calculate output value of the ANN.
    * \param[in] ring Found and fitted ring.
    * \param[in] momentum Momentum of the track attached to this ring.
    * \return ANN output value.
    */
   double DoSelect(
         CbmRichRing* ring,
         double momentum);

   /**
    * \brief Set path to the file with ANN weights.
    * \param[in] fileName path to the file with ANN weights.
    */
   void SetAnnWeights(const string& fileName){fAnnWeights = fileName;}

private:
   /**
    * \brief Copy constructor.
    */
   CbmRichElectronIdAnn(const CbmRichElectronIdAnn&);

   /**
    * \brief Assignment operator.
    */
   CbmRichElectronIdAnn& operator=(const CbmRichElectronIdAnn&);
};

#endif

