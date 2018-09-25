// -------------------------------------------------------------------------
// -----                CbmPrimaryVertexFinder header file             -----
// -----                  Created 28/11/05  by V. Friese               -----
// -------------------------------------------------------------------------


/** CbmPrimaryVertexFinder
 *@author V.Friese ,v.friese@gsi.de>
 **
 ** Abstract base class for primary vertex finder algorithms. Each derived
 ** class must implement the method FindPrimaryVertex. The input is the
 ** array of CbmStsTracks (later to be replaced by CbmGlobalTracks), the
 ** output is one object of type CbmVertex.
 **/


#ifndef CBMPRIMARYVERTEXFINDER_H
#define CBMPRIMARYVERTEXFINDER_H 1


#include "TObject.h"


class TClonesArray;
class CbmVertex;


class CbmPrimaryVertexFinder : public TObject
{

 public:

  /** Default constructor **/
  CbmPrimaryVertexFinder() { };


  /** Destructor **/
  virtual ~CbmPrimaryVertexFinder() { };


  /** Initialisation (virtual). If needed, to be implemented in the
   ** derived class. Else no action.
   **/
  virtual void Init() { };


  /** Execution of PV finding (abstract). To be implemented in 
   ** the derived class. Task: Take the track array and find the vertex.
   ** Set the vertex parameters of the object vertex with its SetVertex
   ** method.
   *@param tracks   TClonesArray of CbmStsTracks
   *@param vertex   Primary vertex (output)
   **/
  virtual Int_t FindPrimaryVertex(TClonesArray* tracks, 
				  CbmVertex* vertex) = 0;


  ClassDef(CbmPrimaryVertexFinder,1);

};

#endif
