//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  Tracking reco hit and GenFit interface classes;
//

#include <assert.h>

#include <GeaneTrackRep.h>

#include "EicTrackingRecoHit.h"

// Move to a better place later;
#define _SQR_(arg) ((arg)*(arg))

// -----------------------------------------------------------------------------------------------

TMatrixT<double> EicTrackingRecoHit::getHMatrix(const GFAbsTrackRep* stateVector)
{  
  // Deal with Geane representation for now;
  if (dynamic_cast<const GeaneTrackRep*>(stateVector) != NULL) {
    // Do it better later once implement 1D X- and Y-silicon detectors;
    assert(GetMdim() == 2);

    TMatrixT<double> HMatrix(GetMdim(), 5);

    // Reset all elements;
    for(unsigned ip=0; ip<GetMdim(); ip++)
      for(unsigned iq=0; iq<5; iq++)
	HMatrix[ip][iq] = 0.0;
	
    // Set only slope coefficients (?);
    for(unsigned ip=0; ip<GetMdim(); ip++)
      HMatrix[ip][ip+3] = 1.0;

    return HMatrix;
  }
  else {
    std::cerr << "For now can only handle state"
              << " vectors of type GeaneTrackRep only -> abort"
        << std::endl;
     throw;
  } //if
} // EicTrackingRecoHit::getHMatrix()

// -----------------------------------------------------------------------------------------------

//
// FIXME: may want to optimize some of the actions here (check basis 
//        vector lookup availability, etc);
//

const LogicalVolumeLookupTableEntry *EicTrackingRecoHit::GetLookupTableNode(EicTrackingDigiHit* hit, void *ptr)
{
  EicGeoParData *gPtr = (EicGeoParData *)ptr;
  ULogicalIndex_t id = gPtr->GeantMultiToLogicalIndex(hit->GetMultiIndex());

  return gPtr->GetLookupTableNode(id);
} // EicTrackingRecoHit::GetLookupTableNode()

// -----------------------------------------------------------------------------------------------

EicPlanarRecoHit::EicPlanarRecoHit(EicTrackingDigiHit* hit, void *ptr) : 
  GFRecoHitIfc<GFPlanarHitPolicy>(hit->GetMdim()), mDim(hit->GetMdim())
{ 
  const LogicalVolumeLookupTableEntry *node = GetLookupTableNode(hit, ptr);

  // Reset hit coordinates to 0.0 and assign local (diagonal) covariance matrix; 
  // see virtual plane definition below (global[] enters there!);
  TVector3 local = hit->GetLocalCoordinates();

  // fHitCoord[][] & fHitCov[][] are not inherited from EicTrackingRecoHit, so perhaps
  // do not want to try to unify this code between EicPlanarRecoHit & EicSpaceRecoHit;
  for(int iq=0; iq<hit->GetMdim(); iq++) {
    fHitCoord[iq][0 ] = 0.0;

    //fHitCov  [iq][iq] = _SQR_(hit->GetSigma(iq));
    fHitCov  [iq][iq] = hit->GetCovariance(iq,iq);
  } //for iq

  if (hit->mXYmode) {
    TVector3 uu(1,0,0), vv(0,1,0);
    fPolicy.setDetPlane(GFDetPlane(LocalToMaster(node->mGeoMtx, local), 
				   LocalToMasterVect(node->mGeoMtx, uu), 
				   LocalToMasterVect(node->mGeoMtx, vv))); 
  } else {
    //assert(0);
    // NB: should be in sync with EicKfNodeTemplateCartesian2D::SmearLocalCoord();
    TVector3 uu = TVector3(local.Y(),-local.X(),0).Unit(), vv(0,0,1);
    fPolicy.setDetPlane(GFDetPlane(LocalToMaster(node->mGeoMtx, local), 
				   LocalToMasterVect(node->mGeoMtx, uu), 
				   LocalToMasterVect(node->mGeoMtx, vv))); 
  } //if
} // EicPlanarRecoHit::EicPlanarRecoHit()

// -----------------------------------------------------------------------------------------------

//
// FIXME: move cov.matrix to MARS; for now assume that TPC is aligned along Z axis 
// -> no rotation needed here; -> CHECK!!!
//

EicSpaceRecoHit::EicSpaceRecoHit(EicTrackingDigiHit* hit, void *ptr) : 
  GFRecoHitIfc<GFSpacepointHitPolicy>(3)
{
  const LogicalVolumeLookupTableEntry *node = GetLookupTableNode(hit, ptr);

  // Reset hit coordinates to 0.0 and assign local (diagonal) covariance matrix; 
  // see virtual plane definition below (global[] enters there!);
  TVector3 local = hit->GetLocalCoordinates();
  TVector3 global = LocalToMaster(node->mGeoMtx, local);

  double buffer[3][3];
  for(int ip=0; ip<hit->GetMdim(); ip++) 
    for(int iq=0; iq<hit->GetMdim(); iq++) 
      buffer[ip][iq] = hit->GetCovariance(ip,iq);
  TGeoRotation grr;
  grr.SetMatrix((double*)buffer); 
  
  TGeoRotation www = *node->mGeoMtx * grr * node->mGeoMtx->Inverse();
  //TGeoRotation www = node->mGeoMtx->Inverse() * grr * *node->mGeoMtx;

  for(int ip=0; ip<hit->GetMdim(); ip++) {
    fHitCoord[ip][0 ] = global[ip];

    for(int iq=0; iq<hit->GetMdim(); iq++) 
      //@@@ earlier version (buggy: no conversion to MARS) fHitCov[ip][iq] = hit->GetCovariance(ip,iq);
      fHitCov[ip][iq] = www.GetRotationMatrix()[ip*3+iq];
  } //for ip
} // EicSpaceRecoHit::EicSpaceRecoHit()

// -----------------------------------------------------------------------------------------------

ClassImp(EicSpaceRecoHit)
ClassImp(EicPlanarRecoHit)
