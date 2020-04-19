//
// AYK (ayk@bnl.gov), 2014/08/22
//
//  A trivial (for now) extension of FairRunSim class;
//

#include <TRandom.h>
#include <TH2D.h>
#include <TStopwatch.h>
#include <TKey.h>

#include <FairRuntimeDb.h>
#include <FairTrajFilter.h>
#include <FairParRootFileIo.h>
#include <FairPrimaryGenerator.h>

#include <PndCave.h>

#include <EicRunSim.h>
#include <EicMCApplication.h>

EicRunSim* EicRunSim::mInstance = 0;

#define _OUTPUT_FILE_DEFAULT_     "simulation.root"
#define _MEDIA_FILE_DEFAULT_      "media.geo"
#define _CAVE_FILE_NAME_DEFAULT_  "cave.geo"

// ---------------------------------------------------------------------------------------

EicRunSim::EicRunSim(const char *engine): mInitState(stUndefined), 
					  mSuppressSecondariesFlag(false), mFluxMonitorGrid(0),
					  mOutputFileName(_OUTPUT_FILE_DEFAULT_),
					  mMediaFileName(_MEDIA_FILE_DEFAULT_),
					  mSeed(_SEED_DEFAULT_),
					  mCaveFileName(_CAVE_FILE_NAME_DEFAULT_),
					  mCaveDefinedFlag(false),
					  mTimerFlag(true),
					  mField(0), 
					  mIgnoreBlackHoleVolumes(false), mSuppressHitProductionFlag(false),
					  mSuppressFairRootSteppingCallFlag(false)
					    
{
  if (mInstance) {
    Fatal("EicRunSim::EicRunSim()", "Singleton instance already exists.");
    return;
  } //if

  if (engine) SetName(engine);

  mInstance = this;
} // EicRunSim::EicRunSim()

// ---------------------------------------------------------------------------------------

void EicRunSim::SetMaterials(const char *mediaFileName)
{
  mMediaFileName = mediaFileName;
} // EicRunSim::SetMaterials()

// ---------------------------------------------------------------------------------------

void EicRunSim::SetOutputFile(const char *outputFileName)
{
  mOutputFileName = outputFileName;
} // EicRunSim::SetOutputFile()

// ---------------------------------------------------------------------------------------

void EicRunSim::SetCaveFileName(const char *caveFileName)
{
  mCaveFileName = caveFileName;
} // EicRunSim::SetCaveFileName()

// ---------------------------------------------------------------------------------------

void EicRunSim::AddModule(FairModule *module)
{
  // Unless cave was defined already, do it now;
  if (!mCaveDefinedFlag) {
    FairModule *Cave= new PndCave("CAVE");
    Cave->SetGeometryFileName(mCaveFileName);
    FairRunSim::AddModule(Cave); 

    mCaveDefinedFlag = true;
  } //if

  FairRunSim::AddModule(module);
} // EicRunSim::AddModule()

// ---------------------------------------------------------------------------------------

void EicRunSim::AddGenerator(FairGenerator *generator)
{
  if (!GetPrimaryGenerator()) {
    FairPrimaryGenerator* primGen = new FairPrimaryGenerator();
    SetGenerator(primGen);
  } //if

  GetPrimaryGenerator()->AddGenerator(generator);
} // EicRunSim::AddGenerator()

// ---------------------------------------------------------------------------------------

void EicRunSim::AddField(FairField *field)
{
  if (!GetField()) {
    mField = new PndMultiField();
    SetField(mField);
  } //if

  mField->AddField(field);
} // EicRunSim::AddField()

// ---------------------------------------------------------------------------------------

void EicRunSim::Init() 
{ 
  // If failed once, fail forever;
  if (mInitState == stFailed) return;

  // This basically means default EicRunSim::EicRunSim() constructor used and 
  // no SetName() call happened afterwards;
  if (fName.IsNull()) {
    printf("\n\n  EicRunSim::Init(): no G3/G4 engine specified.\n\n");
    mInitState = stFailed;
    return;
  } //if

  gRandom->SetSeed(mSeed); 

  // Deal the same way as with SetMaterials() and SetOutputFile() once ever need 
  // to set this flag to kFALSE;
  SetStoreTraj(kTRUE);

  FairRunSim::SetMaterials(mMediaFileName);
  FairRunSim::SetOutputFile(mOutputFileName);

  // Basically want to expand original virtual FairMCApplication::Stepping() call;
  fApp = new EicMCApplication("Fair","The Fair VMC App",ListOfModules, MatFname);
  FairRunSim::Init(); 

  // FairRunSim::Init() has no return value -> assume success;
  mInitState = stSucceeded;
} // EicRunSim::Init() 

// ---------------------------------------------------------------------------------------

void EicRunSim::Run(Int_t NEvents, Int_t NotUsed) 
{ 
  // This is indeed a clear hack;
  //if (mJanaPluginMode) return;

  RunCoreStart(NEvents, NotUsed);

  // Yes, just exit; change the default behaviour if this ever becomes a problem;
  exit(0);
} // EicRunSim::Run()

void EicRunSim::RunCoreStart(Int_t NEvents, Int_t NotUsed) 
{ 
  // Attempt to call Init(0 if it has not happened so far);
  if (mInitState == stUndefined) Init();

  // If initialization failed, do not do anything;
  if (mInitState == stFailed) return;
  
  // Get pointer to the runtime database;
  FairRuntimeDb *rtdb = GetRuntimeDb();
  FairParRootFileIo* output = new FairParRootFileIo(kTRUE);
  char simparamsFile[1024];
  // Well, assume /tmp is always available?; switch to the working 
  // directory if this ever becomes an issue;
  snprintf(simparamsFile, 1024-1, "/tmp/simparams-%05d.root", getpid());
  output->open(simparamsFile);
  rtdb->setOutput(output);

  // Help EicEventGenerator::ReadEvent() to open its current TTree in simulation.root
  // rather than in temporary simparams.root file;
  GetOutputFile()->cd();

  FairTrajFilter* trajFilter = FairTrajFilter::Instance();
  // Deal the same way as with SetMaterials() and SetOutputFile() once ever need 
  // to set this flag to kFALSE;
  trajFilter->SetStorePrimaries(kTRUE);
  trajFilter->SetStoreSecondaries(mSuppressSecondariesFlag ? kFALSE : kTRUE);

  // Yes, I'm mostly interested to see what is the actual per-event simulation time, 
  // so place Stopwatch initialization here;
  {
    TStopwatch timer;
    if (mTimerFlag) timer.Start();

    // Call the core FairRoot routine (actual simulation run);
    FairRunSim::Run(NEvents, NotUsed); 

    if (mTimerFlag) {
      // Print some info;
      timer.Stop();
      printf("\n  Wall Time = %7.1f s, CPU Time = %7.1f s\n\n", timer.RealTime(), timer.CpuTime());
    } //if
  }

  // Save the parameters;
  rtdb->closeOutput();

  // Fine, at this point both parameter database file and the output file 
  // are closed; re-open them again and copy over parameter file contents;
  // yes, this is a hack, but is a simpliest way to get rid of simparams.root file;
  {
    TFile *fpar = new TFile(simparamsFile);
    TFile *fsim = new TFile(mOutputFileName, "UPDATE");

    TIter next(fpar->GetListOfKeys());
    TKey *key;
    while ((key = (TKey*)next())) {
      //printf("%s %s\n", key->GetName(), key->GetClassName());
      
      if (!strcmp(key->GetClassName(), "TProcessID")) continue;
      
      TObject *obj = fpar->Get(key->GetName());
      assert(obj);
      
      fsim->cd();
      obj->Write();
    } //while 
    
    fpar->Close();
    fsim->Close();
  }

  unlink(simparamsFile);

  // Yes, and get rid of this file as well; change the default behaviour if this
  // ever becomes an issue;
  if (!access(_GPHYSI_DAT_, W_OK)) unlink(_GPHYSI_DAT_);

  // Yes, just exit; change the default behaviour if this ever becomes a problem;
  //++exit(0);
} // EicRunSim::RunCoreStart() 

// ---------------------------------------------------------------------------------------

int EicRunSim::DefineFluxMonitorGrid(double rMax, unsigned rDim, double zMin, double zMax, 
				     unsigned zDim)
{
  if (mFluxMonitorGrid) {
    Fatal("EicRunSim", "EicRunSim::DefineFluxMonitorGrid() called more than once.");
    return -1;
  } //if
  
  mFluxMonitorGrid = new FluxMonitorGrid(rMax, rDim, zMin, zMax, zDim);

  // Arrange a separate FairTask with the only purpose to call Write() at the end of 
  // simulation run; FairRunSim::Instance() pointer should indeed be always available, or?;
  /*FairRunSim::Instance()->*/AddTask(new EicFluxMonitorTask(/*outputTree*/));

  return 0;
} // EicRunSim::DefineFluxMonitorGrid()

// ---------------------------------------------------------------------------------------

int EicRunSim::AddFluxMonitorParticleType(int pdg, double eMin, double eMax)
{
  if (!mFluxMonitorGrid) {
    Fatal("EicRunSim", "EicRunSim::AddFluxMonitorParticleType() called prior to "
	  "EicRunSim::DefineFluxMonitorGrid().");
    return -1;
  } //if
  
  return mFluxMonitorGrid->AddFluxMonitorParticleType(pdg, eMin, eMax);
} // EicRunSim::AddFluxMonitorParticleType()

// ---------------------------------------------------------------------------------------

FluxMonitorGrid::FluxMonitorGrid(double rMax, unsigned rDim, double zMin, double zMax, unsigned zDim):
  mRmax(rMax), mRdim(rDim), mZmin(zMin), mZmax(zMax), mZdim(zDim), mRadiationMapType(false), 
  mNormalizedCrossSection(0.0), mTrials(0), mTotalOriginalStat(0)
{ 
  // FIXME: sanity check, please;
  mRbwidth =  rMax        /rDim; 
  mZbwidth = (zMax - zMin)/zDim; 

  //mDensity = new double [mRdim*mZdim];
  //for(unsigned iz=0; iz<mZdim; iz++) 
  //for(unsigned ir=0; ir<mRdim; ir++) 
  //  mDensity[iz*mRdim+ir] = 0.0; 
} // FluxMonitorGrid::FluxMonitorGrid()

// ---------------------------------------------------------------------------------------

int FluxMonitorGrid::AddFluxMonitorParticleType(int pdg, double eMin, double eMax)
{
  if (eMin < 0.0 || eMin > eMax) {
    Fatal("EicRunSim", "FluxMonitorGrid::AddFluxMonitorParticleType(): eMin >= eMax.");
    return -1;
  } //if

  mParticles.push_back(FluxMonitorParticleType(pdg, eMin, eMax));

  // Initialize flux array;
  FluxMonitorParticleType *ptype = &mParticles[mParticles.size()-1];
  ptype->mLength        = new double [mRdim*mZdim];
  ptype->mDensityLength = new double [mRdim*mZdim];
  ptype->mEdep          = new double [mRdim*mZdim];
  for(unsigned iz=0; iz<mZdim; iz++) 
    for(unsigned ir=0; ir<mRdim; ir++) 
      ptype->mLength[iz*mRdim+ir] = ptype->mEdep[iz*mRdim+ir] = 
	ptype->mDensityLength[iz*mRdim+ir] = 0.0; 

  return 0;
} // FluxMonitorGrid::AddFluxMonitorParticleType()

// ---------------------------------------------------------------------------------------

#include <TVirtualMC.h>

void FluxMonitorGrid::AddEntry(int pdg, double eKin, double dE, TVector3 in, TVector3 out)
{
  // No particle types required -> exit;
  if (!mParticles.size()) return;

  // First figure out if this particle is of interest at all;
  unsigned needed[mParticles.size()], nCounter = 0;
  memset(needed, 0x00, sizeof(needed));
  for(unsigned pt=0; pt<mParticles.size(); pt++) {
    FluxMonitorParticleType *ptype = &mParticles[pt];

    // NB: energy range check here will work for mEmin=mEmin=0.0 case as well;
    if ((!ptype->mPDG || pdg == ptype->mPDG) && eKin >= ptype->mEmin && 
	(!ptype->mEmax || eKin <= ptype->mEmax)) {
      needed[pt] = 1;
      nCounter++;
    } //if
  } //for pt

  // Well, nCounter=0 means either wrong PDG or energy range out of interest; 
  if (!nCounter) return;

  // Then figure out which space cells (cylinders in case of axially-symmetric 
  // {RZ}-grid considered for now) were crossed by this track segment; assume it 
  // was a straight line; start with Z-slices; reorder in Z for simplicity if needed;
  TVector3 *v1, *v2;
  if (in.z() <= out.z()) {
    v1 = &in; v2 = &out;
  }
  else {
    v1 = &out; v2 = &in;
  } //if
  // Figure out Z-cell ID range; i1 and i2 are ordered here, for sure;
  int iz1 = (int)floor((v1->z() - mZmin)/mZbwidth), iz2 = (int)floor((v2->z() - mZmin)/mZbwidth);
  // Check out-of-range condition;
  if (iz1 > mZdim || iz2 < 0) return;
  double dz = v2->z() - v1->z();
  // Calculate vector along [v1,v2] straight line segment and its length;
  TVector3 dv = *v2 - *v1; double dvlen = dv.Mag(); //assert(dvlen);
  
  //printf("\nZ (%3d %3d): %9.3f -> %9.3f; R: %9.3f -> %9.3f (L = %9.3f)\n", 
  //	 iz1, iz2, v1->z(), v2->z(), 
  //	 sqrt(v1->x()*v1->x()+v1->y()*v1->y()), sqrt(v2->x()*v2->x()+v2->y()*v2->y()), 
  //	 dv.Mag());

  // Loop through all indices between iz1 and iz2; it looks like there will be 
  // at most as many pieces as iz2-iz1+1, right?;
  for(int iqz=iz1; iqz<=iz2; iqz++) {
    if (iqz < 0) continue;
    if (iqz >= mZdim) break;

    // Z-coordinate of the left and right segment piece points;
    double zl = iqz == iz1 ? v1->z() : mZmin + (iqz+0)*mZbwidth; if (zl < mZmin) zl = mZmin;
    double zr = iqz == iz2 ? v2->z() : mZmin + (iqz+1)*mZbwidth; if (zr > mZmax) zr = mZmax;
    //printf("   %3d -> %9.3f .. %9.3f\n", iqz, zl, zr);

    // Calculate respective 3D points;
    TVector3 vl = *v1 + ((zl - v1->z())/dz) * dv; //vl.Print();
    TVector3 vr = *v1 + ((zr - v1->z())/dz) * dv; //vr.Print();
    TVector3 dvlr = vr - vl;

    // Fine, this segment piece crossed iqz-th Z-slice along the straight line 
    // segment between 3D points 'vl' and 'vr'; figure out which radial bins
    // were crossed; work in 2D now;
    {
      std::vector<std::pair<double, double> > parts;
      // Find the closest approach point to (0,0) on straight line {vl,vr} in XY-projection;
      TVector2 wl(vl.x(), vl.y()), wr(vr.x(), vr.y());
      TVector2 dw = wr - wl;
      double t = -(wl * dw)/dw.Mod2(), rl = wl.Mod(), rr = wr.Mod();
      //printf("       t(PCA) = %9.3f; (%9.3f %9.3f %9.3f)\n", t, rl, (wl + t*dw).Mod(), rr);

      // Now, if 0<=t<=1, should consider 2 parts separately; otherwise
      // radius changes monotonously from 'v1' to 'v2' -> one part;
      if (t <= 0.0 || t >= 1.0) {
	// Want to order vertices in R in order to simplify logic later;
	if (rl < rr)
	  parts.push_back(std::pair<double, double>(0.0, 1.0));
	else
	  parts.push_back(std::pair<double, double>(1.0, 0.0));
	//printf("    ---> so 1 part!\n");
      }
      else {
	// Here ordering is trivial of course ('t' comes first - PCA);
	parts.push_back(std::pair<double, double>(t, 0.0));
	parts.push_back(std::pair<double, double>(t, 1.0));
	//printf("    ---> so 2 parts!\n");
      } //if
      
      // Loop through either 1 or 2 parts, does not matter;
      for(unsigned i=0; i<parts.size(); i++) {
	std::pair<double, double> *part = &parts[i];

	// 2D ends of this part and their radii; they are ordered in R;
	TVector2 _u1 = wl + part->first*dw, _u2 = wl + part->second*dw;
	double r1 = _u1.Mod(), r2 = _u2.Mod();

	// Inner radius too big -> skip;
	if (r1 > mRmax) continue;
	
	int ir1 = (int)floor(r1/mRbwidth), ir2 = (int)floor(r2/mRbwidth);
	//printf("        %3d %3d\n", ir1, ir2);
	// Loop through all indices between ir1 and ir2; it looks like there will be 
	// at most as many pieces as ir2-ir1+1, right?;
	for(int iqr=ir1; iqr<=ir2; iqr++) {
	  if (iqr >= mRdim) break;

	  // Either take one of the ends or calculate crossing point with the 
	  // circle of respective radius; NB: use W-vectors (not U-ones) since these 
	  // are those who are related to the t-parameterization;
	  double t1, t2;
	  double A = dw.Mod2();
	  double B = 2 * wl * dw;
	  if (iqr == ir1)
	    t1 = part->first;
	  else {
	    double R = (iqr+0)*mRbwidth;
	    double C = wl.Mod2() - R*R;
	    
	    // FIXME: unify quadratic equation solution later;
	    double D = sqrt(B*B - 4*A*C);
	    double roots[2] = {(-B-D)/(2*A), (-B+D)/(2*A)};
	    // Need root in the [first .. second] range;
	    int id = -1;
	    for(unsigned irt=0; irt<2; irt++) {
	      //printf("               R = %7.3f; r#%d -> %7.3f\n", R, irt, roots[irt]);
	      if ((roots[irt] >= part->first  && roots[irt] <= part->second) ||
		  (roots[irt] >= part->second && roots[irt] <= part->first)) {
		assert(id == -1);
		id = irt;
	      } //if
	    } //for irt
	    assert(id != -1);

	    t1 = roots[id];
	  } //if
	  if (iqr == ir2)
	    t2 = part->second;
	  else {
	    double R = (iqr+1)*mRbwidth;
	    double C = wl.Mod2() - R*R;

	    // FIXME: unify quadratic equation solution later;
	    double D = sqrt(B*B - 4*A*C);
	    double roots[2] = {(-B-D)/(2*A), (-B+D)/(2*A)};
	    // Need root in the [first .. second] range;
	    int id = -1;
	    for(unsigned irt=0; irt<2; irt++)
	      if ((roots[irt] >= part->first  && roots[irt] <= part->second) ||
		  (roots[irt] >= part->second && roots[irt] <= part->first)) {
		assert(id == -1);
		id = irt;
	      } //for irt..if
	    assert(id != -1);

	    t2 = roots[id];
	  } //if
	  //printf("t1,2: %f %f\n", t1, t2);

	  // Fine, so now t-parameters of both ends are known; return back to 3D 
	  // and calculate respective points; 
	  TVector3 s1 = vl + t1 * dvlr; //s1.Print();
	  TVector3 s2 = vl + t2 * dvlr; //s2.Print();
	  double crlen = (s2 - s1).Mag();

	  // Fine, so iqz & iqr are known, as well as the crossing length;
	  // loop through all the booked particle types and fill out respective arrays;
	  for(unsigned pt=0; pt<mParticles.size(); pt++) {
	    FluxMonitorParticleType *ptype = &mParticles[pt];

	    if (!needed[pt]) continue;

	    //if (len > 20.) {
	    //printf("%d, @@@ %f\n", pdg, len);
	    //in.Print(); out.Print();
	    //printf("IZ: %3d %3d; IR: %3d %3d\n", iz1, iz2, ir1, ir2);

	    //exit(0);
	    //} //if

	    //printf("%f\n", dE);
	    ptype->mLength[iqz*mRdim+iqr] += crlen;

	    if (mRadiationMapType) {
	      Float_t fA, fZmat, fDensity, fRadl, fAbsl;

	      gMC->CurrentMaterial(fA, fZmat, fDensity, fRadl, fAbsl);
	      //printf("%f\n", fDensity);

	      ptype->mEdep[iqz*mRdim+iqr] += dvlen ? dE*(crlen/dvlen) : dE;
	      ptype->mDensityLength[iqz*mRdim+iqr] += fDensity*crlen;
	    } //if
	  } //for pt
	} //for iqr
      } //for i
    }
  } //for iqz
} // FluxMonitorGrid::AddEntry()

// ---------------------------------------------------------------------------------------

// Well, automatic determination of cross-section from .root files requires 
// eic-smear support compiled in; in fact any support of ROOT event format requires it;
#ifdef _EICSMEAR_
#include <EicEventGenerator.h>
#endif

void FluxMonitorGrid::FillOutHistograms(unsigned evStat)
{
  // Try to get luminosity info from the .root file with MC events if not provided 
  // via command line call FluxMonitorGrid::SetNormalization(); NB: it is pretty easy 
  // to screw up with normalization in these calculations, so consider to use built-in
  // capability provided below; this ONLY WORKS if 0) EicEventGenerator() was used 
  // in simulation.C script rather than any other FairGenerator-derived class, 
  // 1) .root file input with MC events is used rather than ASCII file (since
  // ASCII generator files do not contain normalization info), 2) it is a single ROOT
  // file (so no chains), 3) file was PROPERLY converted by eic-smear BuildTree call
  // (so the 4-th parameter with matching logfile name was given); this functionality 
  // is technically available for PYTHIA (as well as perhaps Pepsi, Djangoh and Milou);
#ifdef _EICSMEAR_
  if (!mNormalizedCrossSection || !mTrials || !mTotalOriginalStat) {
    // Check that EicEventGenerator is used in simulation.C;
    EicEventGenerator *generator = EicEventGenerator::Instance(); 

    if (generator) {
      const TChain *chain = generator->GetInputTree(); 

      // Check that ROOT input is used rather than ASCII, and only one file; 
      if (chain && chain->GetListOfFiles()->GetEntriesFast() == 1) {
	TObjString *crossSectionString(NULL), *trialsString(NULL);

	TFile *file = chain->GetFile();
	file->GetObject("crossSection", crossSectionString); assert(crossSectionString);
	file->GetObject("nEvents", trialsString); assert(trialsString);

	//TTree* tree(NULL);
	//file->GetObject("EICTree", tree);
	//mTotalOriginalStat = tree->GetEntriesFast();

	mNormalizedCrossSection = atof(crossSectionString->GetString().Data());
	mTrials = mTotalOriginalStat = atoi(trialsString->GetString().Data());
	printf("%7.2f %d %d\n", mNormalizedCrossSection, mTrials, mTotalOriginalStat);
      } //if
    } //if
  } //if
#endif

  // At this point normalization should be available, either this or that way; 
  // FIXME: do this check better later; the below 2D histograms will be normalized to 
  // 1 fb^-1 integrated luminosity; FIXME: well, 1 inverse femtobarn is hardcoded here;
  assert(mNormalizedCrossSection && mTrials && mTotalOriginalStat);
  double lumi_exp = 1.0/1E-15, lumi_mc = 
    (1.0*evStat/mTotalOriginalStat)*(mTrials/(mNormalizedCrossSection*1E-6));
  double norm = (lumi_exp/lumi_mc);

  double cVolumes[mRdim];

  // Calculate single cylindrical volume volumes;
  for(unsigned ir=0; ir<mRdim; ir++) {
    double r1 = ir*mRbwidth, r2 = (ir+1)*mRbwidth;

    cVolumes[ir] = TMath::Pi()*(r2*r2-r1*r1)*mZbwidth;
  } //for ir

  FairRun *fRun = FairRun::Instance();
  // I guess there is no need to save/restore current directory here?;
  fRun->GetOutputFile()->cd();

  // Conversion factor for radiation dose calculation;
  double gev2joule = 1.6E-10;

  // Loop through all the booked particle groups, declare 2D histograms, etc;
  for(unsigned pt=0; pt<mParticles.size(); pt++) {
    FluxMonitorParticleType *ptype = &mParticles[pt];

    double density[mZdim][mRdim];
    for(unsigned iz=0; iz<mZdim; iz++) 
      for(unsigned ir=0; ir<mRdim; ir++) 
	density[iz][ir] = 
	  ptype->mLength[iz*mRdim+ir] ? ptype->mDensityLength[iz*mRdim+ir]/ptype->mLength[iz*mRdim+ir] : 0.0;

    char htitle[1024] = "", hname[1024], pname[1024] = "neutron", lumi[1024];
    snprintf(hname,  1024-1, "flux%02d", pt); 
    // FIXME: pdg hardcoded ...;
    if (ptype->mPDG != 2112) snprintf(pname,  1024-1, "PDG#%d", ptype->mPDG);
    snprintf(lumi,  1024-1, "%5.1f fb^{-1} integrated luminosity", lumi_exp*1E-15); 
    
    if (mRadiationMapType) 
      snprintf(htitle, 1024-1, "Radiation dose in [J/cm^{3}] for %s", lumi);
    else 
      snprintf(htitle, 1024-1, "%s flux above %5.1f keV in [n/cm^{2}] for %s", pname,
	       ptype->mEmin*1E6, lumi);
    TH2D *hh = new TH2D(hname, htitle, mZdim, mZmin, mZmax, mRdim, 0.0, mRmax);
    hh->GetXaxis()->SetTitle("Z coordinate (along the beam line), [cm]");
    hh->GetYaxis()->SetTitle("Radial coordinate, [cm]");

    for(unsigned iz=0; iz<mZdim; iz++) 
      for(unsigned ir=0; ir<mRdim; ir++) {
	// Yes, will be different estimates for different particle types;
	// just because statistically these particles sample different 
	// amount of material (otherwise would get huge spikes at the 
	// border of heavy materials and air if a given cylindrical cell 
	// is shared between these media);
	double cMass = cVolumes[ir] * density[iz][ir];
 
	//double flux = ptype->mFlux[iz*mRdim+ir], quantity = //density[iz][ir];//mDensity[iz*mRdim+ir];
	//mRadiationMapType ? (cMass ? flux/cMass : 0.0) : 
	//(flux/cVolumes[ir])/norm;
	
	//hh->SetBinContent(iz+1, ir+1, cMass ? ptype->mEdep[iz*mRdim+ir]/cMass : 0.0);
	//+hh->SetBinContent(iz+1, ir+1, ptype->mEdep[iz*mRdim+ir]/cVolumes[ir]);
	hh->SetBinContent(iz+1, ir+1, norm*(mRadiationMapType ? gev2joule*ptype->mEdep[iz*mRdim+ir] : 
					    ptype->mLength[iz*mRdim+ir])/cVolumes[ir]);
      } //for iz..ir

    hh->Write();
  } //for pt
} // FluxMonitorGrid::FillOutHistograms()

// ---------------------------------------------------------------------------------------

void EicFluxMonitorTask::FinishTask()
{
  printf("FluxMonitorTask called!\n");

  EicRunSim *fRun = EicRunSim::Instance();
  // In fact EicRunSim is the only user of this task; yet check its presence;
  if (fRun && fRun->GetFluxMonitorGrid()) fRun->GetFluxMonitorGrid()->FillOutHistograms(mStat);

  FairTask::FinishTask();
} // EicFluxMonitorTask::FinishTask()

// ---------------------------------------------------------------------------------------

ClassImp(EicRunSim)
ClassImp(FluxMonitorGrid)
ClassImp(EicFluxMonitorTask)
ClassImp(FluxMonitorParticleType)
