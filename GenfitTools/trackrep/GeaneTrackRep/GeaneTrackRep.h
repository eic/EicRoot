//-----------------------------------------------------------
// File and Version Information:
// $Id$
//
// Description:
//      a GEANE (sd-system)  track representation
//      (q/p, v',w',v,w) 
//      (v,w) refers to DetPlane system
//
// Environment:
//      Software developed for the PANDA Detector at FAIR.
//
// Author List:
//      Sebastian Neubert    TUM            (original author)
//      Andrea Fontana       INFN
//
//
//-----------------------------------------------------------

#ifndef GeaneTRACKREP_HH
#define GeaneTRACKREP_HH

// Base Class Headers ----------------
#include "GFAbsTrackRep.h"
#include "FairTrackParP.h"

// Collaborating Class Headers -------
#include <ostream> // remove if you do not need streaming op
#include "TVectorT.h"


// Collaborating Class Declarations --
class FairGeanePro;


class GeaneTrackRep : public GFAbsTrackRep {
public:

  // Constructors/Destructors ---------
  GeaneTrackRep();
  GeaneTrackRep(FairGeanePro* geane,
		const GFDetPlane& plane, // will be defined at origin of plane
		const TVector3& mom,
		const TVector3& poserr,
		const TVector3& momerr,
		double q,
		int PDGCode);

  GeaneTrackRep(FairGeanePro* geane,
                const GFDetPlane& plane, // will be defined at origin of plane
                const TVector3& mom,
                const TVector3& poserr,
                const TVector3& momerr,
                int q,
                int PDGCode);

  virtual ~GeaneTrackRep();


  virtual GFAbsTrackRep* clone() const {return new GeaneTrackRep(*this);}
  virtual GFAbsTrackRep* prototype()const{return new GeaneTrackRep();}

  // Operators
  friend std::ostream& operator<< (std::ostream& s, const GeaneTrackRep& me);

  // Accessors -----------------------

  // Modifiers

  // Operations ----------------------

  virtual double extrapolate(const GFDetPlane&, TMatrixT<double>& statePred);
  //virtual void extrapolate(const GFDetPlane&, 
  //			   const TMatrixT<double>& stateFrom 
  //			   TMatrixT<double>& stateResult);

  virtual double extrapolate(const GFDetPlane&, 
			   TMatrixT<double>& statePred,
			   TMatrixT<double>& covPred);

  //these two are overwritting ABsTrackRep methods
  void extrapolateToPoint(const TVector3& pos,
			 TVector3& poca,
			 TVector3& dirInPoca);

  void extrapolateToLine(const TVector3& point1,
	 		 const TVector3& point2,
			 TVector3& poca,
			 TVector3& dirInPoca,
			 TVector3& poca_onwire);


  TVector3 getPocaOnLine(const TVector3& p1, 
			 const TVector3& p2, 
			 bool back=false);

  virtual TVector3 getPos(const GFDetPlane&) ;
  virtual TVector3 getMom(const GFDetPlane&) ;
  virtual void getPosMom(const GFDetPlane&,TVector3& pos,TVector3& mom) ;
  virtual void getPosMomCov(const GFDetPlane& pl,TVector3& pos,TVector3& mom,TMatrixT<double>& cov);
  virtual double getCharge()const {return fState[0][0] > 0 ? 1.: -1.;}
  int getPropDir() {return _backw;} 
  FairGeanePro* getPropagator() {return _geane;}
  int getPDG() {return _pdg;};
  double getSPU() {return _spu;}

  void setPropagator(FairGeanePro* g){_geane=g;}
  void setPropDir(int d){_backw=d;} 
  void switchDirection(){_backw=-_backw;}

  // (-1,0,1) -> (backward prop,decide myself,forward)

private:

  void checkState(); // checks if state vector is inside numerical limits

  // Private Data Members ------------
  FairGeanePro* _geane; //!
  double _spu; // sign of z-component of momentum
  int _pdg; // pdg code of the particle to be tracked
  int _backw; // (-1,0,1) -> (backward prop,decide myself,forward)

  // Private Methods -----------------
  
  // calculate jacobian of extrapolation
  //void Jacobian(const GFDetPlane& pl,
  //	const TMatrixT<double>& statePred,
  //	TMatrixT<double>& jacResult);

 public:
  ClassDef(GeaneTrackRep,1)

};


#endif

//--------------------------------------------------------------
// $Log$
//--------------------------------------------------------------

