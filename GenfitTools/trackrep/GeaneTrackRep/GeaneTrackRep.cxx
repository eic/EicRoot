//-----------------------------------------------------------
// File and Version Information:
// $Id$
//
// Description:
//      Implementation of class GeaneTrackRep
//      see GeaneTrackRep.hh for details
//
// Environment:
//      Software developed for the PANDA Detector at FAIR.
//
// Author List:
//      Sebastian Neubert    TUM            (original author)
//
//
//-----------------------------------------------------------

// Panda Headers ----------------------

// This Class' Header ------------------
#include "GeaneTrackRep.h"
#include "FairGeaneUtil.h"
#include "FairTrackParH.h"

// C/C++ Headers ----------------------
#include <iostream>
#include <cmath>

// Collaborating Class Headers --------
#include "GFAbsRecoHit.h"
#include "GFException.h"
#include "FairGeanePro.h"

// Class Member definitions -----------

#define THETACUT 0.4
#define EPSILON 1E-4

GeaneTrackRep::GeaneTrackRep()
  : GFAbsTrackRep(5), _pdg(211),_backw(0), _spu(1)
{

}

GeaneTrackRep::GeaneTrackRep(FairGeanePro* geane, 
			     const GFDetPlane& plane,
			     const TVector3& mom,
			     const TVector3& poserr,
			     const TVector3& momerr,
			     double q,
			     int PDGCode) 
  : GFAbsTrackRep(5), _geane(geane), _pdg(PDGCode), _backw(0), _spu(1)
{
  FairTrackParP par(plane.getO(),mom,poserr,momerr,(int)TMath::Sign(1.0, q),plane.getO(),plane.getU(),plane.getV());

  _spu=par.GetSPU(); // direction of the momentum

  fState[0][0]=par.GetQp();
  fState[1][0]=par.GetTV();
  fState[2][0]=par.GetTW();
  fState[3][0]=par.GetV();
  fState[4][0]=par.GetW();

  // blow up cov-array: ROOT does not support init with symmetric data
  // See ROOT docu source-file for TMatrixTSym
  // i=row, j=collumn
  double* covarray=par.GetCov();
  int count=0;
  for(int i=0;i<5;++i){
    for(int j=i;j<5;++j){
      fCov[i][j]=covarray[count];
      if(i!=j)fCov[j][i]=covarray[count];
      ++count;
    }
  }
  fRefPlane=plane;
}

GeaneTrackRep::GeaneTrackRep(FairGeanePro* geane, 
			     const GFDetPlane& plane,
			     const TVector3& mom,
			     const TVector3& poserr,
			     const TVector3& momerr,
			     int q,
			     int PDGCode) 
  : GFAbsTrackRep(5), _geane(geane), _pdg(PDGCode), _backw(0), _spu(1)
{
  FairTrackParP par(plane.getO(),mom,poserr,momerr,q,plane.getO(),plane.getU(),plane.getV());

  _spu=par.GetSPU(); // direction of the momentum

  fState[0][0]=par.GetQp();
  fState[1][0]=par.GetTV();
  fState[2][0]=par.GetTW();
  fState[3][0]=par.GetV();
  fState[4][0]=par.GetW();

  // blow up cov-array: ROOT does not support init with symmetric data
  // See ROOT docu source-file for TMatrixTSym
  // i=row, j=collumn
  double* covarray=par.GetCov();
  int count=0;
  for(int i=0;i<5;++i){
    for(int j=i;j<5;++j){
      fCov[i][j]=covarray[count];
      if(i!=j)fCov[j][i]=covarray[count];
      ++count;
    }
  }
  fRefPlane=plane;
}

//  GeaneTrackRep::GeaneTrackRep(const GeaneTrackRep& rep) 
//   : GFAbsTrackRep(rep)
// {
//   _geane=rep._geane;
// }


GeaneTrackRep::~GeaneTrackRep()
{
  
}




double
GeaneTrackRep::extrapolate(const GFDetPlane& pl, 
			   TMatrixT<double>& statePred)
{
  TMatrixT<double> covPred(5,5);
  return  extrapolate(pl,statePred,covPred);
  //! TODO: make this faster by neglecting covariances ?
}


double
GeaneTrackRep::extrapolate(const GFDetPlane& pl, 
			   TMatrixT<double>& statePred,
			   TMatrixT<double>& covPred)
{
  if(fabs(getMom(fRefPlane).Theta()/TMath::Pi()*180.) < THETACUT){
    GFException exc("GEANE propagation not possible for p.theta<THETACUT",__LINE__,__FILE__);
    exc.setFatal();
    throw exc;
  }
  statePred.ResizeTo(fDimension,1);
  covPred.ResizeTo(fDimension,fDimension);
  TVector3 o=pl.getO();
  TVector3 u=pl.getU();
  TVector3 v=pl.getV();

  TVector3 ofrom=fRefPlane.getO();
  TVector3 ufrom=fRefPlane.getU();
  TVector3 vfrom=fRefPlane.getV();

  _geane->PropagateFromPlane(ufrom,vfrom);
  _geane->PropagateToPlane(o,u,v);

  FairTrackParP result;
  FairTrackParH result2;
  
  //std::cout<<"Before prop:"<<std::endl;
  //Print();



  double cova[15];
  int count=0;;
  for(int i=0; i<5;++i){
    for(int j=i;j<5;++j){
      cova[count++]=fCov[i][j];
    }
  }
  //protect against low momentum:
  if(fabs(fState[0][0])>10){
    GFException exc("GeaneTrackRep: PROTECT AGAINST LOW MOMENTA",__LINE__,__FILE__);
    exc.setFatal();
    throw exc;
  }

  checkState();
  
  
  FairTrackParP par(fState[3][0],fState[4][0],fState[1][0],fState[2][0],fState[0][0],cova,ofrom,ufrom,vfrom,_spu);
  bool backprop=_backw<0;
  if(_backw==0){
    //Try to guess if we are doing a forward or backward step:
    TVector3 pos(par.GetX(),par.GetY(),par.GetZ());
    TVector3 dir=pl.dist(pos); // direction from pos to plane;
    //Assume B=(0,0,BZ) -> compare signs of dir.Z and mom.Z:
    //backprop= (dir.Z()*par.GetPz())<0 ? true : false;
	backprop= (dir*getMom(fRefPlane))<0;
  }
  if(backprop){
    _geane->setBackProp();
    //std::cout<<"GEANETRACKREP: USING BACKPROPAGATION!" << std::endl;
  }

  Bool_t prop = kTRUE;
  prop = _geane->Propagate(&par,&result,_pdg);   //211
  if (prop==kFALSE){
    GFException exc("GEANE propagation failed",__LINE__,__FILE__);
    //exc.setFatal();
    throw exc;
  }
  

  double l=_geane->GetLengthAtPCA();
 
  statePred[0][0]=result.GetQp();
  statePred[1][0]=result.GetTV();
  statePred[2][0]=result.GetTW();
  statePred[3][0]=result.GetV();
  statePred[4][0]=result.GetW();

  

  double* rescov=result.GetCov();
  count=0;
  for(int i=0;i<5;++i){
    for(int j=i;j<5;++j){
      covPred[i][j]=rescov[count];
      if(i!=j)covPred[j][i]=rescov[count];
      ++count;
    }
  }
  
  //   if(result.GetSPU()!=_spu)std::cout<<"SPU HAS CHANGED! "<<_spu<<" --> "<<result.GetSPU()<<std::endl;
  _spu=result.GetSPU();

  //std::cout<<"AFTER EXTRAPOLATE:"<<std::endl;
  //result.Print();
  //pl.Print();
  //statePred.Print();
  //covPred.Print();
  


  return l;
}



void
GeaneTrackRep::extrapolateToPoint(const TVector3& pos,
				 TVector3& poca,
				 TVector3& dirInPoca){
  if(fabs(getMom(fRefPlane).Theta()/TMath::Pi()*180.) < THETACUT){
    GFException exc("GEANE propagation not possible for p.theta<THETACUT",__LINE__,__FILE__);
    exc.setFatal();
    throw exc;
  }
  int dim = getDim();
  TMatrixT<double> statePred(dim,1);
  TMatrixT<double> covPred(dim,dim);
  //std::cout<<"GeaneTrackRep::extrapolateToPoint"<<std::endl;
  //fRefPlane.Print();

  TVector3 ofrom=fRefPlane.getO();
  TVector3 ufrom=fRefPlane.getU();
  TVector3 vfrom=fRefPlane.getV();

  _geane->SetPoint(pos);
  _geane->PropagateFromPlane(ufrom,vfrom);

  double cova[15];
  int count=0;;
  for(int i=0; i<5;++i){
    for(int j=i;j<5;++j){
      cova[count++]=fCov[i][j];
    }
  }
  //protect against low momentum:
  if(fabs(fState[0][0])>10){
    GFException exc("GeaneTrackRep: PROTECT AGAINST LOW MOMENTA",__LINE__,__FILE__);
    exc.setFatal();
    throw exc;
  }

  checkState();
    
  FairTrackParP par(fState[3][0],fState[4][0],fState[1][0],fState[2][0],fState[0][0],cova,ofrom,ufrom,vfrom,_spu);
  //par.Print();
  bool backprop=_backw<0;
  if(_backw==0){
    // check if new point is after or before my position
    TVector3 dir=fRefPlane.dist(pos); // direction from pos to plane;
	backprop= (dir*getMom(fRefPlane))>0;
  }
  if(!backprop){ // point lies in same direction of flight as momentum
    //std::cout<<" Propagate in flight direction"<<std::endl;
    _geane->PropagateToVirtualPlaneAtPCA(1);
  }
  else{
    //std::cout<<" backPropagate"<<std::endl;
    _geane->BackTrackToVirtualPlaneAtPCA(1);
  }

  FairTrackParP result;
  Bool_t prop = kTRUE;
  prop = _geane->Propagate(&par,&result,_pdg);   //211
  if (prop==kFALSE) {
    GFException exc("GEANE propagation failed",__LINE__,__FILE__);
    //exc.setFatal();
    throw exc;
    //pl=fRefPlane;
    //return pos;
  }

  statePred[0][0]=result.GetQp();
  statePred[1][0]=result.GetTV();
  statePred[2][0]=result.GetTW();
  statePred[3][0]=result.GetV();
  statePred[4][0]=result.GetW();

  double* rescov=result.GetCov();
  count=0;
  for(int i=0;i<5;++i){
    for(int j=i;j<5;++j){
      covPred[i][j]=rescov[count];
      if(i!=j)covPred[j][i]=rescov[count];
      ++count;
    }
  }

  poca.SetXYZ(result.GetX(),result.GetY(),result.GetZ());
  dirInPoca = result.GetJVer().Cross( result.GetKVer() );
}


void 
GeaneTrackRep::extrapolateToLine(const TVector3& point1,
				 const TVector3& point2,
				 TVector3& poca,
				 TVector3& dirInPoca,
				 TVector3& poca_onwire)
{
  if(fabs(getMom(fRefPlane).Theta()/TMath::Pi()*180.) < THETACUT){
    GFException exc("GEANE propagation not possible for p.theta<THETACUT",__LINE__,__FILE__);
    exc.setFatal();
    throw exc;
  }

  // call propagation to closest approach to a wire 
  Int_t pca = 2;

  // calculate a very large track length 
  TVector3 start = getPos(fRefPlane);
  Double_t distance1, distance2;
  distance1 = (point1 - start).Mag();
  distance2 = (point2 - start).Mag();
  Double_t maxdistance;
  if(distance1 < distance2) maxdistance = distance2;
  else maxdistance = distance1;
  maxdistance *= 2.;  

  // variables for FindPCA:
  TVector3 point(0,0,0);
  Double_t Rad = 0.;
  // poca = vpf = point of closest approach on track
  // poca_onwire = vwi = point of closest approach on wire
  Double_t Di = 0.;
  Float_t trklength = 0.;
  
  // covariance matrix
  FairGeaneUtil util;
  Double_t cov55[5][5];
  for(int i = 0; i < 5; i++) for(int j = 0; j < 5; j++) cov55[i][j] = fCov[i][j];
  Double_t cova[15];
  util.FromMat25ToVec15(cov55, cova);
  
  TVector3 o  = fRefPlane.getO();
  TVector3 dj = fRefPlane.getU();
  TVector3 dk = fRefPlane.getV();
  
  FairTrackParP par(fState[3][0],fState[4][0],fState[1][0],fState[2][0],fState[0][0],cova,o,dj,dk,_spu);

  // get propagation direction
  Int_t direction = getPropDir();
  
  _geane->ActualFindPCA(pca, &par, direction);
  Int_t findpca = _geane->FindPCA(pca, _pdg, point, point1, point2, maxdistance, Rad, poca, poca_onwire, Di, trklength);
  
  if(findpca != 0) {
    GFException exc("findpca failure", __LINE__,__FILE__);	
    throw exc;    
  }

  // dir in poca not filled now
  dirInPoca.SetXYZ(0., 0., 0.);
  
}






TVector3 
GeaneTrackRep::getPocaOnLine(const TVector3& p1, const TVector3& p2, bool back){
  
  //std::cout<<"GeaneTrackRep::getPocaToWire"<<std::endl;

  TVector3 ofrom=fRefPlane.getO();
  TVector3 ufrom=fRefPlane.getU();
  TVector3 vfrom=fRefPlane.getV();

  _geane->SetWire(p1,p2);
  _geane->PropagateFromPlane(ufrom,vfrom);
  double cova[15];
  int count=0;;
  for(int i=0; i<5;++i){
    for(int j=i;j<5;++j){
      cova[count++]=fCov[i][j];
    }
  }
  // protect against low momentum:
  if(fabs(fState[0][0])>10){
    GFException exc("GeaneTrackRep: PROTECT AGAINST LOW MOMENTA",__LINE__,__FILE__);
    exc.setFatal();
    throw exc;
  }

  checkState();
    
  FairTrackParP par(fState[3][0],fState[4][0],fState[1][0],fState[2][0],fState[0][0],cova,ofrom,ufrom,vfrom,_spu);

  
  if(!back){ // point lies in same direction of flight as momentum
    //std::cout<<" Propagate in flight direction"<<std::endl;
    _geane->PropagateToVirtualPlaneAtPCA(2); // option 2 means wire!
  }
  else{
    //std::cout<<" backPropagate"<<std::endl;
    _geane->BackTrackToVirtualPlaneAtPCA(2);
  }

  FairTrackParP result;
  Bool_t prop = kTRUE;

  prop = _geane->Propagate(&par,&result,_pdg);
  if (prop==kFALSE) {
    GFException exc("GEANE propagation failed",__LINE__,__FILE__);
    //exc.setFatal();
    throw exc;
  }

  return _geane->GetPCAOnWire();
}






TVector3 
GeaneTrackRep::getPos(const GFDetPlane& pl)
{
  TMatrixT<double> statePred(fState);
  if(pl!=fRefPlane)extrapolate(pl,statePred);
  return pl.getO()+(statePred[3][0]*pl.getU())+(statePred[4][0]*pl.getV());
}
 
TVector3 
GeaneTrackRep::getMom(const GFDetPlane& pl)
{
  TMatrixT<double> statePred(fState);
  if(pl!=fRefPlane)extrapolate(pl,statePred);
  double fSPU  = _spu;
  TVector3 mom = fSPU*pl.getNormal()+fSPU*statePred[1][0]*pl.getU()+fSPU*statePred[2][0]*pl.getV();
  mom.SetMag(1./fabs(statePred[0][0]));
  return mom;
}
void
GeaneTrackRep::getPosMom(const GFDetPlane& pl,TVector3& pos, TVector3& mom)
{
  TMatrixT<double> statePred(fState);
  if(pl!=fRefPlane)extrapolate(pl,statePred);
  mom = pl.getNormal()+statePred[1][0]*pl.getU()+statePred[2][0]*pl.getV();

  mom.SetMag(1./fabs(statePred[0][0]));
  pos = pl.getO()+(statePred[3][0]*pl.getU())+(statePred[4][0]*pl.getV());
}


void
GeaneTrackRep::getPosMomCov(const GFDetPlane& pl,TVector3& pos,TVector3& mom,TMatrixT<double>& cov){
  cov.ResizeTo(6,6);

  TMatrixT<double> statePred(fState), covPred(fCov);
  if(pl!=fRefPlane)extrapolate(pl, statePred, covPred);

  // position
  pos = pl.getO()+(statePred[3][0]*pl.getU())+(statePred[4][0]*pl.getV());

  // momentum
  double fSPU  = _spu;
  mom = fSPU*pl.getNormal()+fSPU*statePred[1][0]*pl.getU()+fSPU*statePred[2][0]*pl.getV();
  mom.SetMag(1./fabs(statePred[0][0]));
  
  // covariance matrix
  FairGeaneUtil util;
  // covPred 5 X 5 ==> cov55[5][5]
  double cov55[5][5];
  for(int i = 0; i < 5; i++) for(int j = 0; j < 5; j++) cov55[i][j] = covPred[i][j];
  // cov55[5][5] ==> cov15[15]
  double cov15[15];
  util.FromMat25ToVec15(cov55, cov15);
  
  FairTrackParP parPred(statePred[3][0], 
			statePred[4][0], statePred[1][0], 
			statePred[2][0], statePred[0][0], 
			cov15, 
			pl.getO(), pl.getU(), pl.getV(), 
			_spu);
  double cov66[6][6];
  parPred.GetMARSCov(cov66);
  for(int i = 0; i < 6; i++) for(int j = 0; j < 6; j++) cov[i][j] = cov66[i][j];
 
}


void
GeaneTrackRep::checkState(){
  if(fabs(fState[3][0])<1.E-4)fState[3][0]=1.E-4;
  if(fabs(fState[4][0])<1.E-4)fState[4][0]=1.E-4;
  
  //if (!(fState.Abs()>1.E-15) || !(fState.Abs()<1.E50)){
  //  GFException exc("fState out of numerical bounds",__LINE__,__FILE__);
  //  exc.setFatal();
  //  throw exc;
  //}
}

 
ClassImp(GeaneTrackRep)

