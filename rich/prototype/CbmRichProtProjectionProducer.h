// -------------------------------------------------------------------------
// -----             CbmRichProtProjectionProducer header file             -----
// -------------------------------------------------------------------------

#ifndef CBM_RICH_PROTPROJECTION_PRODUCER
#define CBM_RICH_PROTPROJECTION_PRODUCER

#include "CbmRichProjectionProducerBase.h"

class TClonesArray;
class CbmGeoRichPar;
class TObjArray;

class CbmRichProtProjectionProducer : public CbmRichProjectionProducerBase{

public:
   /**
    * \brief Standard constructor.
    * \param[in] zflag Flag whether to use point in imaginary plane (zflag=1) or mirror point (zflag=2) for extrapolation.
    */
   CbmRichProtProjectionProducer(
         int zflag);

  /**
   * \brief Destructor.
   */
  virtual ~CbmRichProtProjectionProducer();

  /**
   * \brief Initialization of the task.
   */
  virtual void Init();

  /**
   * \brief Initialization  of Parameter Containers.
   */
  virtual void SetParContainers();

  /**
   * \brief Execute task.
   * \param[out] richProj Output array of created projections.
   */
  virtual void DoProjection(
        TClonesArray* richProj);

private:
   TClonesArray* fListRICHImPlanePoint; // Starting points&directions

   int fNHits; // Number of hits
   int fEvent; // number of events

   double fDetX; // X-coordinate of photodetector
   double fDetY; // Y-coordinate of photodetector
   double fDetZ; // Z-coordinate of photodetector
   double fDetWidthX; // width of photodetector in x
   double fDetWidthY; // width of photodetector in y
   double fThetaDet; // tilting angle of photodetector (around x-axis)
   double fPhiDet; // tilting angle of photodetector (around y-axis)

   double fDetXTransf; // X-coordinate of photodetector (transformed system)
   double fDetYTransf; // Y-coordinate of photodetector (transformed system)
   double fDetZTransf; // Z-coordinate of photodetector (transformed system)

   double fZm[4]; // Z-coordinate of mirror center
   double fYm[4]; // Y-coordinate of mirror center
   double fXm[4]; // X-coordinate of mirror center
   double fR[4]; // mirror radius

   double fMaxXTrackExtr; // reasonable max x value for track extrapolation
   double fMaxYTrackExtr; // reasonable max y value for track extrapolation

   TObjArray* fSensNodes;
   TObjArray* fPassNodes;
   CbmGeoRichPar* fPar;

   /**
   * \brief Copy constructor.
   */
   CbmRichProtProjectionProducer(const CbmRichProtProjectionProducer&);

   /**
   * \brief Assignment operator.
   */
   CbmRichProtProjectionProducer& operator=(const CbmRichProtProjectionProducer&);

};

#endif
