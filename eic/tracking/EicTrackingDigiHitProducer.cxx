//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  Tracking digi hit producer class;
//

#include <assert.h>
#include <cmath>

#include "TRandom.h"

#include <KfMatrix.h>
#include <SensitiveVolume.h>
#include <EicRunDigi.h>
#include <EicTrackingDigiHit.h>
#include <EicTrackingDigiHitProducer.h>

// ---------------------------------------------------------------------------------------

EicTrackingDigiHitProducer::EicTrackingDigiHitProducer(const char *name,
						       SmearingModel smearingModel):
  EicDigiHitProducer(name), 
  //mOriginalSmearingModel (smearingModel), 
  mSmearingModel(smearingModel)
  //, mForceRealHitSmearing(false)
{ 
  // Check whether digitization pass was started under EicRunDigi rather than 
  // FairRunAna; also check, that hit file import was set up; change effective 
  // smearing model to NoAction if needed);
#if _LATER_
  mDigiRun = dynamic_cast<EicRunDigi *>(EicRunDigi::Instance());

  mHitImportMode = mDigiRun && mDigiRun->HitImportMode() ? true : false;
  if (mHitImportMode) {
    if (mOriginalSmearingModel != EicDigiHitProducer::NoAction)
      printf("-W- EicTrackingDigiHitProducer::EicTrackingDigiHitProducer() "
	     "-> effective smearing mode changed to 'NoAction'!\n");

    mEffectiveSmearingModel = EicDigiHitProducer::NoAction;
  } //if
#endif
} // EicTrackingDigiHitProducer::EicTrackingDigiHitProducer()

// -------------------------------------------------------------------------

InitStatus EicTrackingDigiHitProducer::ExtraInit()
{
  if (!mDigiHitClassName.IsNull()) {
    // May want to move this stuff to EicDigiHitProducer::Init();
    FairRootManager* ioman = FairRootManager::Instance();

    mDigiHitArray = new TClonesArray(mDigiHitClassName);

    ioman->Register(mDetName->Name() + "TrackingDigiHit", mDetName->NAME(), mDigiHitArray, mPersistence);
  } //if

  return kSUCCESS;
} // EicTrackingDigiHitProducer::ExtraInit()

// ---------------------------------------------------------------------------------------

int EicTrackingDigiHitProducer::HandleHit(const EicMoCaPoint *point)
{
  // Yes, prefer to take middle point; NB: as of 2014/09/20 this point will be given to 
  // FairHit instead of a result of global->local->smear->global sequence; rationale: 
  // eg 1D detectors know nothing about other two coordinates, so why one should 
  // confuse say a track finder by artificially composed quantities; 
  TVector3 middle( 0.5 * (point->GetPosIn() + point->GetPosOut()) );
  //TVector3 direction = (point->GetPosOut() - point->GetPosIn()).Unit();

  ULogicalIndex_t xy = mGptr->GeantMultiToLogicalIndex(point->GetMultiIndex());
  {
    const EicGeoMap *fmap = mGptr->GetMapPtrViaHitMultiIndex(point->GetMultiIndex());

    // If map is not declared as "sensitive" (so its top-level volume is not sensitive)
    // and energy deposit accouting is not requested, no reason to allocate a new cell;
    if (!fmap || !fmap->IsSensitive()) return 0;
  }

  // FIXME: do it better later; for now just prohibit using default HandleHit()
  // for cases when custom resolution calculation is required;
  assert (mSmearingModel != EicDigiHitProducer::Calculate);

  unsigned accu = 0;
  for(unsigned nd=0; nd<mKfNodeTemplates.size(); nd++) {
    EicKfNodeTemplate *kftmpl = mKfNodeTemplates[nd];

    // Coordinates in "node local" system (so not only master->local transformation, 
    // but local->node as well); yes, at most 3 components I guess; some sane direction 
    // vector for cases when it is not needed (let it be aligned with local Z axis);
    double localCoord[3];//, localDirection[3] = {0.0, 0.0, 1.0};

#if _LATER_
    if (mHitImportMode) {
      // This stuff is basically disabled; if ever want to use it, rework from scratch;
      assert(0);
      double sigma[3], *sigmaPtr = mForceRealHitSmearing ? 0 : sigma;
      //memset(sigma, 0x00, sizeof(sigma));

      // FIXME: GetZ() is a bad idea here -> do it better later;
      unsigned group = mGptr->GetZ(xy);
      
      // NB; after this call local[] component(s) are already in the 
      // "node local" coordinate system;
      if (mDigiRun->GetDetectorHits(mDetName->NAME().Data(), group, accu, 
				    kftmpl->GetMdim(), localCoord, sigma)) 
	goto _next_node;

      kftmpl->PackDigiHit(mDigiHitArray, mOriginalSmearingModel, mEffectiveSmearingModel, 
			  mDetName->Name(), point, nd, localCoord, localDirection, middle, sigmaPtr);
    }
    else 
#endif
      {
      const LogicalVolumeLookupTableEntry *node = mGptr->GetLookupTableNode(xy);
      assert(node);
      
      // FIXME: redo invariant (through TVector3 passing);
      double midarr[3] = {middle   [0], middle   [1], middle   [2]};
      //double dirarr[3] = {direction[0], direction[1], direction[2]};

      if (kftmpl->mNodeToSensitiveVolume) {
	double buffer[3];
	
	node->mGeoMtx->                 MasterToLocal(midarr, buffer);
	kftmpl->mNodeToSensitiveVolume->MasterToLocal(buffer, localCoord);

	//node->mGeoMtx->                 MasterToLocalVect(dirarr, buffer);
	//kftmpl->mNodeToSensitiveVolume->MasterToLocalVect(buffer, localDirection);
      }
      else {
	node->mGeoMtx->    MasterToLocal(midarr, localCoord);
	//node->mGeoMtx->MasterToLocalVect(dirarr, localDirection);
      } //if

      TVector3 local(localCoord);
      kftmpl->StoreDigiHit(mDigiHitArray, mDetName->Name(), point, nd, middle, 
			   local, mSmearingModel);
    } //if

  _next_node:
    accu += kftmpl->GetMdim();
  } //for nd

  return 0;
} //  EicTrackingDigiHitProducer::HandleHit()

// ---------------------------------------------------------------------------------------

//
// FIXME: re-incorporate this later (Calculate is of interest);
//

#if _HTC_
int EicTrackingDigiHitProducer::HandleHit(EicMoCaPoint *point)
{
  // Yes, prefer to take middle point;
  TVector3 middle( 0.5 * (point->GetPosIn() + point->GetPosOut()) );

  TVector3 local = fGeoH->MasterToLocalPath(middle, point->fVolumePath);

  // Do it better later; for now just prohibit using default HandleHit()
  // for cases when custom resolution calculation is required;
  //assert (_smearingModel != _CALCULATE_);

  double phi = 0.0, cov[6];
  memset(cov, 0x00, sizeof(cov));

  // So then smear local coordinates;
  switch (_smearingModel) {
  case _SMEAR_:
    {
      for(int iq=0; iq<3; iq++)
	if (_fResolution[iq]) 
	  local[iq] += gRandom->Gaus(0.0, _fResolution[iq]);
    }
    break;
  case _QUANTIZE_:
    {
      for(int iq=0; iq<3; iq++)
	if (_fPitch[iq]) 
	  // May make problem at small negative numbers -> fix later;
	  local[iq] = _fPitch[iq] * (int) floor (local[iq]/_fPitch[iq]);
    }
    break;
  case _CALCULATE_:
    {
      // Get plane ID as encoded in tracker.C;
      EicGeoMap *fmap = gptr->getMapPtrViaHitMultiIndex(point->fMultiIndex);
      assert(fmap);
      UGeo_t iz = (gptr->remapMultiIndex(point->fMultiIndex) & 0xFFFFFFFF) >> 16;
      //printf("%d\n", iz);

      //
      // A plain XY registering plane example;
      //
#if _OFF_
      {
	double resolution = 0.002;

	// Keep alfa equal 0.0 for now; smear XY;
	for(int iq=0; iq<2; iq++)
	  local[iq] += gRandom->Gaus(0.0, resolution);
	
	// Assume XY diagonal components (upper triangle of a 3x3 matrix);
	cov[0] = cov[3] = resolution * resolution;
      }
#endif

      //
      // (r,phi) registering plane example;
      //
      {
	// Convert (x,y) -> (r,phi) in the local wafer coordinate system;
	// this should work for *any* wafer type with GEANT volume XY=(0,0)
	// in the anticipated rotation center (so full circle as well as 
	// a sector shoul dwork the same);
	double x = local[0], y = local[1], r = sqrt(x*x + y*y);

	phi = atan2(y, x);

	// Assign resolutions in (r,phi) system to whatever numbers needed
	// based on wafer ID (see "iz" above), "r" value, etc; smear or quantize
	// by hand (here gaussian smearing with constant resolutions used); 
	// rotate back to the local wafer GEANT volume coordinate system;
	double dr = 0.010, dt = 0.002;

	TVector2 xq = TVector2(gRandom->Gaus(r, dr), gRandom->Gaus(0.0, dt)).Rotate(phi);
	for(int ip=0; ip<2; ip++)
	  local[ip] = ip ? xq.Y() : xq.X();
	//printf("@@@ %f: %f %f -> %f %f\n", phi, x, local[0], y, local[1]);
	
	// --> FIXME! Do I need to set fResolution to something meaningful?; hmm;
	
	// Assign covariance matrix in (r,phi) system; assume XY diagonal components 
	// in this example; [6]: upper triangle of a 3x3 matrix;
	cov[0] = dr*dr; cov[3] = dt*dt; 
      }
    } 
    break;
  default:
    assert(0);
  } //switch

  // Make FairHit happy; do not want to change the meaning of those variables;
  TVector3 global = fGeoH->LocalToMasterPath(local, point->fVolumePath);
  
  // Create hit; 
  new((*fDigiHitArray)[fDigiHitArray->GetEntriesFast()]) 
    EicTrackingDigiHit(dname->cname(), _odim, point, local, global, 
		       _smearingModel == _SMEAR_ ? _fResolution : _fPitchSqrt12, 
		       phi, _smearingModel == _CALCULATE_ ? cov : 0);

  return 0;
} //  EicTrackingDigiHitProducer::HandleHit()
#endif

// ---------------------------------------------------------------------------------------

//
//  May want to unify with Calorimeter hit producer and move all this stuff
//  to EicDigiHitProducer class;
//

void EicTrackingDigiHitProducer::Finish()
{
  //FairRun *fRun = FairRun::Instance();

  // I guess there is no need to save/restore current directory here?;
  //fRun->GetOutputFile()->cd();

  EicDigiParData *digi = getEicDigiParDataPtr();

  // Yes, save under detector-specific pre-defined name;
  //if (digi) digi->mergeIntoOutputFile(mDetName->Name() + "DigiParData");
  if (digi) digi->mergeIntoOutputFile(mDetName->Name() + "TrackingDigiParData");

  // Save object itself;
  //Write(mDetName->Name() + "DigiHitProducer");
  Write(mDetName->Name() + "TrackingDigiHitProducer");
} // EicTrackingDigiHitProducer::Finish()

// -----------------------------------------------------------------------------------------------

//
// FIXME: clean up interface here; or better get rid of KfMatrix completely;
//

KfMatrix *EicKfNodeTemplate1D::GetMeasurementNoise(const EicTrackingDigiHit *hit) const
{
  static KfMatrix V = KfMatrix(1,1);

  //V.KFM(0,0) = hit->GetSigma(0) * hit->GetSigma(0);
  V.KFM(0,0) = hit->GetCovariance(0,0);

  return &V;
} // EicKfNodeTemplate1D::GetMeasurementNoise()

KfMatrix *EicKfNodeTemplateOrth2D::GetMeasurementNoise(const EicTrackingDigiHit *hit) const
{
  static KfMatrix V = KfMatrix(2,2);

  V.KFM(0,1)     = V.KFM(1,0) = 0.0;
  for(unsigned iq=0; iq<2; iq++)
    //V.KFM(iq,iq) = hit->GetSigma(iq) * hit->GetSigma(iq);
    V.KFM(iq,iq) = hit->GetCovariance(iq,iq);

  return &V;
} // EicKfNodeTemplateOrth2D::GetMeasurementNoise()

KfMatrix *EicKfNodeTemplateOrth3D::GetMeasurementNoise(const EicTrackingDigiHit *hit) const
{
  assert(0);

  static KfMatrix V = KfMatrix(3,3);

  for(unsigned ip=0; ip<3; ip++)
    for(unsigned iq=0; iq<3; iq++)
      V.KFM(ip,iq) = ip == iq ? hit->GetCovariance(iq,iq) : 0.0;

  return &V;
} // EicKfNodeTemplateOrth3D::GetMeasurementNoise()

#if _LATER_
KfMatrix *EicKfNodeTemplateAxial3D::GetMeasurementNoise(const EicTrackingDigiHit *hit) const
{
  assert(0);

#if _OLD_
  static KfMatrix V = KfMatrix(2,2);

  V.KFM(0,1)     = V.KFM(1,0) = 0.0;
  for(unsigned iq=0; iq<2; iq++)
    //V.KFM(iq,iq) = hit->GetSigma(iq) * hit->GetSigma(iq);
    V.KFM(iq,iq) = hit->GetCovariance(iq,iq);

  return &V;
#endif
} // EicKfNodeTemplateAxial3D::GetMeasurementNoise()
#endif

// -----------------------------------------------------------------------------------------------

// FIXME: do it better later;
#define _DIM_ 4

//
// FIXME: I guess this stuff can work only with scan axis being Z axis?;
//

int EicKfNodeTemplate::IncrementLinearTrackFitMatrices(SensitiveVolume *sv, 
						       EicTrackingDigiHit *hit, double zRef,
						       KfMatrix *A, KfMatrix *b)
{
#if _TODAY_
  // FIXME: need to apply local->node rotation as well; perhaps even something more tricky;
  assert(GetMdim() == 2);

  for(unsigned ipp=0; ipp<GetMdim(); ipp++)  {
    KalmanNodeWrapper *wrapper = &sv->mKfNodeWrappers[hit->GetKfNodeID()];
    double alfa = wrapper->GetAxisComponent(ipp,_X_), beta = wrapper->GetAxisComponent(ipp,_Y_);
    double dz = wrapper->GetOrigin()[_Z_] - zRef, R[_DIM_] = {alfa, beta, alfa*dz, beta*dz};

    double xx = (wrapper->GetOrigin() + hit->GetCoord(ipp) * 
		 (*wrapper->GetAxis(ipp))).Dot((*wrapper->GetAxis(ipp)));
    //printf("%f %f\n", wrapper->GetOrigin()[_Z_], xx);
    double rsqr = hit->GetSigma(ipp) * hit->GetSigma(ipp);

    for(int ip=0; ip<_DIM_; ip++)
      for(int iq=0; iq<_DIM_; iq++)
	A->KFM(ip,iq) += R[ip]*R[iq]/rsqr;
      
    for(int ip=0; ip<_DIM_; ip++)
      b->KFV(ip) += R[ip]*xx/rsqr;
  } //for ip
#endif

  return 0;
} // EicKfNodeTemplate::IncrementLinearTrackFitMatrices()

// -----------------------------------------------------------------------------------------------

ClassImp(EicTrackingDigiHitProducer)
ClassImp(EicKfNodeTemplate)
ClassImp(EicKfNodeTemplate1D)
ClassImp(EicKfNodeTemplateLinear1D)
ClassImp(EicKfNodeTemplateRadial1D)
ClassImp(EicKfNodeTemplateAsimuthal1D)
ClassImp(EicKfNodeTemplateOrth2D)
ClassImp(EicKfNodeTemplateCartesian2D)
ClassImp(EicKfNodeTemplateCylindrical2D)
ClassImp(EicKfNodeTemplateOrth3D)
//+ClassImp(EicKfNodeTemplateAxial3D)

