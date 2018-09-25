
#include <iostream>

#include <TGeoVoxelFinder.h>
#include <TKey.h>

#include <EicGeoParData.h>
#include <EicDetName.h>

#include <EicDetector.h>

#define _64BIT_VALUE_INVALID_ (~ULong64_t(0))

// =======================================================================================

void EicDetector::Initialize()
{
  // Once the geometry is defined (so volume names are associated with 
  // unique IDs), calculate binary signatures for all maps;
  if (gptr) gptr->CalculateMappingTableSignatures();

  //FairDetector::Initialize();
} // EicDetector::Initialize()

// ---------------------------------------------------------------------------------------

int EicDetector::DeclareGeantSensitiveVolume(const char *name, SteppingType stType)
{
  if (!fListOfGeantSensitives) fListOfGeantSensitives = new EicNamePatternHub<SteppingType>();

#if _TODAY_
  // I guess no double-counting check is really needed here?; if non-trivial stepping 
  // type is given, use it; otherwise pick up the stepping type given at detector initialization;
  fListOfGeantSensitives->AddExactMatch(name, stType == qSteppingTypeDefault ? fStType : stType);
#endif

  return 0;
} // EicDetector::DeclareGeantSensitiveVolume()

// ---------------------------------------------------------------------------------------

int EicDetector::DeclareGeantSensitiveVolumePrefix(const char *name, SteppingType stType)
{
  if (!fListOfGeantSensitives) fListOfGeantSensitives = new EicNamePatternHub<SteppingType>();

  // Same logic as in DeclareGeantSensitiveVolume();
  fListOfGeantSensitives->AddPrefixMatch(name, stType == qSteppingTypeDefault ? fStType : stType);

  return 0;
} // EicDetector::DeclareGeantSensitiveVolumePrefix()

// ---------------------------------------------------------------------------------------

bool EicDetector::CheckIfSensitive(std::string name)
{
  if (mAllVolumesSensitiveFlag) return true;

  //return (fListOfGeantSensitives ? fListOfGeantSensitives->AnyMatch(name.c_str()) : false);
  return (fListOfGeantSensitives && fListOfGeantSensitives->AnyMatch(name.c_str()));
} // EicDetector::CheckIfSensitive()

// ---------------------------------------------------------------------------------------

//
//  Cut'n'paste from FairModule::ConstructRootGeometry() for now;
//

void EicDetector::ConstructRootGeometry()
{
  /** Construct the detector geometry from ROOT files, possible inputs are:
   * 1. A TGeoVolume as a mother (master) volume containing the detector geometry
   * 2. A TGeoManager with the detector geometry
   * 3. A TGeoVolume as a mother or Master volume which is the output of the CAD2ROOT geometry, in this case
   *    the materials are not proprely defined and had to be reset
   *  In all cases we have to check that the material properties are the same or is the materials defined in
   *  the current simulation session
   */
  TGeoManager* OldGeo=gGeoManager;
  TGeoManager* NewGeo=0;
  TGeoVolume* volume=0;;
  TFile* f=new TFile(GetGeometryFileName().Data());
  TList* l= f->GetListOfKeys();
  TKey* key;
  TIter next( l);
  TGeoNode* n=0;
  TGeoVolume* v1=0;
  while ((key = (TKey*)next())) {
    //std::cout << key->GetClassName() << std::endl;
    /**loop inside the delivered root file and try to fine a TGeoManager object
     * the first TGeoManager found will be read
     */
    if (strcmp(key->GetClassName(),"TGeoManager") != 0) { continue; }
    gGeoManager=0;
    NewGeo = (TGeoManager*)key->ReadObj();
    break;
  }
  if (NewGeo!=0) {
    /** in case a TGeoManager was found get the top most volume and the node
     */

    NewGeo->cd();
    volume=(TGeoVolume*)NewGeo->GetNode(0)->GetDaughter(0)->GetVolume();
    v1=volume->MakeCopyVolume(volume->GetShape());
    // n=NewGeo->GetTopNode();
    n=v1->GetNode(0);
    //  NewGeo=0;
    // delete NewGeo; //move it to the end of the method

  } else {
    /** The file does not contain any TGeoManager, so we assume to have a file with a TGeoVolume
     * try to look for a TGeoVolume inside the file
     */

    //key=(TKey*) l->At(0);  //Get the first key in the list
    //volume=dynamic_cast<TGeoVolume*> (key->ReadObj());
    //if(volume!=0) { n=volume->GetNode(0); }
    //if(n!=0) { v1=n->GetVolume(); std::cout << n->GetName() << " " << GetName() << std::endl; }

    //
    //   - AYK, 2013/09/29 -> consider to perform a bit cleaner search (loop 
    //   through all keys and request name match); this way may use MC files
    //   to import detector geometry;
    //
    TKey* qkey;
    TIter qnext( l);
    
    while ((qkey = (TKey*)qnext())) {
      volume=dynamic_cast<TGeoVolume*> (qkey->ReadObj());
      if (!volume) continue;

      n = volume->GetNode(0);
      if (!n) continue;

      // Well, may want to do a better check later (remove "_N" suffix and request
      // exact match); later;
      if (TString(n->GetName()).BeginsWith(GetName()))
      {
	v1 = n->GetVolume();

	std::cout << n->GetName() << " " << GetName() << std::endl; 
	break;
      } //if
    }
  }

  if(v1==0) {
    //fLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31mFairModule::ConstructRootGeometry(): could not find any geometry in File!!  \033[0m", GetGeometryFileName().Data());
    printf("Could not find any geometry in %s!\n", GetGeometryFileName().Data());
    assert(0);
  }
  gGeoManager=OldGeo;
  gGeoManager->cd();
  //#if _TODAY_
  // If AddToVolume is empty add the volume to the top volume Cave
  // If it is defined check i´f the volume exists and if it exists add the volume from the root file
  // to the already existing volume
  TGeoVolume* Cave=NULL;
  if ( 0 == fMotherVolumeName.Length() ) {
    Cave= gGeoManager->GetTopVolume(); //printf("%s\n", Cave->GetName()); exit(0);
  } else {
    Cave = gGeoManager->GetVolume(fMotherVolumeName); //exit(0);
  }
  if(Cave!=NULL) {
    /**Every thing is OK, we have a TGeoVolume and now we add it to the simulation TGeoManager  */
    gGeoManager->AddVolume(v1); 
    /** force rebuilding of voxels */
    TGeoVoxelFinder* voxels = v1->GetVoxels();
    if (voxels) { voxels->SetNeedRebuild(); }

    // else { fLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31mFairModule::ConstructRootGeometry(): could not find voxels  \033[0m"); }

    /**To avoid having different names of the default matrices because we could have get the volume from another
     * TGeoManager, we reset the default matrix name
     */
    TGeoMatrix* M = n->GetMatrix();
    //#if _TODAY_
    SetDefaultMatrixName(M);
    //#endif

    /** NOw we can remove the matrix so that the new geomanager will rebuild it properly*/
    gGeoManager->GetListOfMatrices()->Remove(M);
    TGeoHMatrix* global = gGeoManager->GetHMatrix();
    gGeoManager->GetListOfMatrices()->Remove(global); //Remove the Identity matrix
    /**Now we can add the node to the existing cave */
    Cave->AddNode(v1,0, M);
    /** correction from O. Merle: in case of a TGeoVolume (v1) set the material properly */
    AssignMediumAtImport(v1);
    //#if _TODAY_
    /** now go through the herachy and set the materials properly, this is important becase the CAD converter
     *  produce TGeoVolumes with materials that have only names and no properties
     */
    ExpandNode(n);
    //#endif
    if(NewGeo!=0) { delete NewGeo; }
    delete f;
  } else {
    //fLogger->Fatal(MESSAGE_ORIGIN,"\033[5m\033[31mFairModule::ConstructRootGeometry(): could not find the given mother volume \033[0m   %s \033[5m\033[31m where the geomanger should be added. \033[0m", fMotherVolumeName.Data());
    printf("Could not find the given mother volume %s where the geometry should be added!\n", fMotherVolumeName.Data());
    assert(0);
  }
  //#endif
} // EicDetector::ConstructRootGeometry() 

void EicDetector::SetDefaultMatrixName(TGeoMatrix* matrix)
{
  // Copied from root TGeoMatrix::SetDefaultName() and modified (memory leak)
  // If no name was supplied in the ctor, the type of transformation is checked.
  // A letter will be prepended to the name :
  //   t - translation
  //   r - rotation
  //   s - scale
  //   c - combi (translation + rotation)
  //   g - general (tr+rot+scale)
  // The index of the transformation in gGeoManager list of transformations will
  // be appended.
  if (!gGeoManager) { return; }
  if (strlen(matrix->GetName())) { return; }
  char type = 'n';
  if (matrix->IsTranslation()) { type = 't'; }
  if (matrix->IsRotation()) { type = 'r'; }
  if (matrix->IsScale()) { type = 's'; }
  if (matrix->IsCombi()) { type = 'c'; }
  if (matrix->IsGeneral()) { type = 'g'; }
  TObjArray* matrices = gGeoManager->GetListOfMatrices();
  Int_t index = 0;
  if (matrices) { index =matrices->GetEntriesFast() - 1; }
  matrix->SetName(Form("%c%i", type, index));
}

void EicDetector::AssignMediumAtImport(TGeoVolume* v)
{
#if _TODAY_
  /**
   * Assign medium to the the volume v, this has to be done in all cases:
   * case 1: For CAD converted volumes they have no mediums (only names)
   * case 2: TGeoVolumes, we need to be sure that the material is defined in this session
   */
  FairGeoMedia* Media       = FairGeoLoader::Instance()->getGeoInterface()->getMedia();
  FairGeoBuilder* geobuild  = FairGeoLoader::Instance()->getGeoBuilder();
#endif

  TGeoMedium* med1=v->GetMedium();
  if(med1 && strcmp(med1->GetName(), "dummy")) {
    TGeoMaterial* mat1=v->GetMaterial();
    TGeoMaterial* newMat = gGeoManager->GetMaterial(med1->GetName());

    if( newMat==0) {
      void *MediumFromDatabase = 0;

      if (!MediumFromDatabase) {
	if (fRootMaterialImportFlag) {
#if _LATER_
	  // Create medium a la FairRoot by hand; sort of mimic FairGeoMedium::read() call 
	  // sequence; this may cause conflicts if more than one ROOT file have material 
	  // with the same name; FIXME: if this ever becomes a problem; for now I just need 
	  // to import STAR geometry database;
	  FairMedium = new FairGeoMedium(med1->GetName());
	  
	  FairMedium->setNComponents(mat1->GetNelements());
	  //printf("%3d\n", mat1->GetNelements());
	  for(unsigned iq=0; iq<mat1->GetNelements(); iq++) {
	    double a, z, w;
	    
	    mat1->GetElementProp(a, z, w, iq);
	    FairMedium->setComponent(iq, a, z, w);
	  } //for iq
	  
	  FairMedium->setDensity(mat1->GetDensity());
	  
	  // Well, FairGeoMedium::read() also calculates rad.length for pure elements only;
	  if (mat1->GetNelements() == 1) FairMedium->calcRadiationLength();
	  
	  // Parameter order is *ucked up compared to TGeo and parameters renamed -> 
	  // carelessly use FairGeoMedium::setMediumPar() sequence; 
	  double params[8];
	  for(unsigned iq=0; iq<8; iq++)
	    params[iq] = med1->GetParam(iq);
	  FairMedium->sensFlag = params[0];//=sensFlag;
	  FairMedium->fldFlag  = params[1];//=fldFlag;
	  FairMedium->fld      = params[2];//=fld;
	  FairMedium->madfld   = params[3];//=madfld;
	  FairMedium->maxstep  = params[4];//=maxstep;
	  FairMedium->maxde    = params[5];//=maxde;
	  FairMedium->epsil    = params[6];//=epsil;
	  FairMedium->minstep  = params[7];//=minstep;
#endif	  

	  // And do not care about Cerenkov, sorry;
	} else {
	  //fLogger->Fatal(MESSAGE_ORIGIN,"Material %s is not defined in ASCII file nor in Root file we Stop creating geometry", 
	  printf("Material %s is not defined in ASCII file nor in Root file we Stop creating geometry",
		 mat1->GetName());
	} //if
      } //if

#if _TODAY_
      Int_t nmed=geobuild->createMedium(FairMedium);
      //FairMedium->print();
      v->SetMedium(gGeoManager->GetMedium(nmed));
#endif
      gGeoManager->SetAllIndex();
    } else {
      /**Material is already available in the TGeoManager and we can set it */
      TGeoMedium* med2= gGeoManager->GetMedium(med1->GetName());

      v->SetMedium(med2);
    }
  } else {
    if (strcmp(v->ClassName(),"TGeoVolumeAssembly") != 0) {
      //[R.K.-3.3.08]  // When there is NO material defined, set it to avoid conflicts in Geant
      //fLogger->Fatal(MESSAGE_ORIGIN,"The volume  %s  Has no medium information and not an Assembly so we have to quit", v->GetName());
      printf("The volume  %s  Has no medium information and not an Assembly so we have to quit", v->GetName());
      assert(0);
    }
  }
}

void EicDetector::ExpandNode(TGeoNode* fN)
{
  TGeoMatrix* Matrix =fN->GetMatrix();
  if(gGeoManager->GetListOfMatrices()->FindObject(Matrix)) { gGeoManager->GetListOfMatrices()->Remove(Matrix); }
  TGeoVolume* v1=fN->GetVolume();
  TObjArray* NodeList=v1->GetNodes();
  for (Int_t Nod=0; Nod<NodeList->GetEntriesFast(); Nod++) {
    TGeoNode* fNode =(TGeoNode*)NodeList->At(Nod);
    TGeoMatrix* M =fNode->GetMatrix();
    SetDefaultMatrixName(M);

    TGeoVolume* v= fNode->GetVolume();

    // Skip th erest of the loop (including ExpandNode() recursion) once a volume was served once; 
    // NB: this implementation is still not clean since v->RegisterYourself() has a recursion inside
    // (so daughters will be registered there more than one time, depending on level difference
    // in the hierarchy); ideally one would call n->GetVolume()->RegisterYourself() in 
    // ConstructRootGeometry() outside of the ExpandNode() call; this way it works, but 
    // simulation itself (for FEMC) becomes few times slower (why?); the implemented trivial 
    // solution speeds up FEMC init phase from ~2 minutes to <1 second and does not affect 
    // simulation time; leave it like this and be happy; eventually may want to arrange
    // a similar lookup table in AssignMediumAtImport() - see eg eic.patch file in EicRoot 
    // r219 where this stuff is commented out but not removed (media lookup is not really
    // needed, volume lookup suffices there);
    if (fGeoVolumeLut.find(v) != fGeoVolumeLut.end()) continue;
    fGeoVolumeLut.insert(v);

    if(fNode->GetNdaughters()>0) { ExpandNode(fNode); }
    AssignMediumAtImport(v);
    if (!gGeoManager->FindVolumeFast(v->GetName())) {
      //fLogger->Debug2(MESSAGE_ORIGIN,"Register Volume : %s ", v->GetName());
      printf("Register Volume : %s ", v->GetName());
      v->RegisterYourself();
    }
    if (CheckIfSensitive(v->GetName())) {
      //fLogger->Debug2(MESSAGE_ORIGIN,"Sensitive Volume : %s ", v->GetName());
      printf("Sensitive Volume : %s ", v->GetName());
      AddSensitiveVolume(v);
    }
  }
}

void EicDetector::AddSensitiveVolume(TGeoVolume* v)
{
  //fLogger->Debug2(MESSAGE_ORIGIN, "FairModule::AddSensitiveVolume", v->GetName());
  printf("FairModule::AddSensitiveVolume", v->GetName());

#if _LATER_
  // Only register volumes which are not already registered
  // Otherwise the stepping will be slowed down
  if( ! vList->findObject(v->GetName() ) ) {
    FairVolume*  volume = NULL;
    volume = new FairVolume(v->GetName(), fNbOfVolumes++);
    vList->addVolume(volume);
    volume->setModId(fModId);
    volume->SetModule(this);
    svList->Add(volume);
    fNbOfSensitiveVol++;
  }
#endif
}

// ---------------------------------------------------------------------------------------

void EicDetector::ConstructGeometry() 
{
  std::cout<<" --- Building " << mDetectorName->NAME() << " Geometry ---"<<std::endl;
  std::cout << GetGeometryFileName() << std::endl;
  
  assert(GetGeometryFileName().EndsWith(".root"));

  // For now assume it is quite natural to pack basic geometric parameter 
  // information into the same ROOT file used to describe the GEANT geometry;
  // ideally this should go into GeoPar stuff, but I fail to make it working;
  // so for now just infect the output MC file with this class information;
  // if this ever becomes problematic (say, because of compatibility issues), 
  // consider to look once again into the GeoPar mechanism available in FairRoot;
  {
    TFile fgeo(GetGeometryFileName());
    
    // Yes, expect object with a predefined name;
    fgeo.GetObject(mDetectorName->Name() + "GeoParData", gptr);
    
    // May want to print out some service info (and then perhaps exit);
    //if (gptr && mPrintGeometryInfoFlag) {
    //gptr->Print();
      
    //if (mPrintGeometryInfoOption.EqualTo("and exit")) exit(0);
    //} //if
    //if (gptr && !mAttachedFilePrintoutRequestName.IsNull()) {
    //gptr->PrintAttachedSourceFile(mAttachedFilePrintoutRequestName.Data());
      
    //if (mAttachedFilePrintoutOption.EqualTo("and exit")) exit(0);
    //} //if
    
      // Loop through all maps and declare sensitive volumes (top-level
      // volume names of all maps); do this only if no sensitive volumes were
      // declared by hand already (then assume user is able to define him/herself
      // which ones does he want; perhaps activate only fiber cores and not absorber
      // volumes to save CPU time);
    if (gptr && !fListOfGeantSensitives)
      for(int iq=0; iq<gptr->GetMapNum(); iq++)
	{
	  const EicGeoMap *fmap = gptr->GetMapPtrViaMapID(iq);
	  
	  printf("@SV@ %s\n", fmap->GetInnermostVolumeName()->Data());
	  // No 2-d argument -> will use stepping type defined at detector initialization;
	  DeclareGeantSensitiveVolume(fmap->GetInnermostVolumeName()->Data());
	} //if..for iq
    
    fgeo.GetObject(mDetectorName->Name() + "GeantGeoWrapper", vptr); 
    
    fgeo.Close(); 
  }
  
  // And make a standard FairRoot call to parse GEANT geometry information;
  ConstructRootGeometry();

#if _TODAY_
  // Once all the volumes are added to the geometry (so their IDs are known), create 
  // step enforced volume lookup table;
  if (gptr && !strcmp(FairRun::Instance()->GetName(), "TGeant4")) {
    // Could not have this all been arranged in a bit more straighforward way?;
    FairGeoLoader *fgLoader = FairGeoLoader::Instance();
    FairGeoInterface *fgInterface = fgLoader->getGeoInterface();
    FairGeoMedia *fgMedia = fgInterface->getMedia();
    
    for (std::set<TString>::iterator it=gptr->GetStepEnforcedVolumes().begin(); 
	 it!=gptr->GetStepEnforcedVolumes().end(); ++it) {
      TGeoVolume *volume = gGeoManager->GetVolume(it->Data());
      
      // Well, volume with such name should exist, otherwise something went wrong;
      if (!volume)
	fLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Step enforcement for unknown "
		       "volume attempted (%s)!  \033[0m", it->Data());
      
      FairGeoMedium *fgMedium = fgMedia->getMedium(volume->GetMedium()->GetName());
      
      // Assume medium at this point exists for sure; extract max step value;
      // FIXME: well, hardcode for now both 10 and 4?;
      double params[10];
      fgMedium->getMediumPar(params);
      
      gptr->AddStepEnforcedVolumeLookupEntry(volume->GetNumber(), params[4]);
    } //for it 
  } //if
#endif
} // EicDetector::ConstructGeometry() 

// ---------------------------------------------------------------------------------------

ULong64_t EicDetector::GetNodeMultiIndex()
{
  ULong64_t ret = _64BIT_VALUE_INVALID_;

  // In particular HADES-style geometries have no chance to share their mapping; so 
  // just return back nonsense index; NB: also new-style geometry files may be missing 
  // this info;
  if (!gptr) return ret;

  //
  //  Later may want to create a look-up table (STL map) based on node ID;
  //  really needed?;
  //

  UInt_t lvVolumeIds[gptr->GetMaxVolumeLevelNum()], lvNodeIds[gptr->GetMaxVolumeLevelNum()];

  // If current path is not the same as it was upon sensitive volume entry, 
  // have to fool the geometry manager; check later, what's wrong with GEANT4 here;
  TString returnBackPath;
  if (strcmp(gGeoManager->GetPath(), fPathUponEntry.Data()))
  {
    returnBackPath = gGeoManager->GetPath();
    gGeoManager->cd(fPathUponEntry);
  } //if

  for(int lv=0; lv<gptr->GetMaxVolumeLevelNum(); lv++)
  {
    // May also use gGeoManager->GetMother() for lv=0; who cares;
    TGeoNode *node = lv ?  gGeoManager->GetMother(lv) : gGeoManager->GetCurrentNode();

    // Could this crash if node=0 (for instance levels exhausted in this branch)?;
    // well, check on that; setting values to 0 do not hurt (may also leave them 
    // uninitialized, but Ok);
    lvVolumeIds        [lv] = node ? node->GetVolume()->GetNumber()  : 0;
    lvNodeIds          [lv] = node ? node->             GetNumber()  : 0; 
  } // for lv

  // Loop through all maps and find matching one;
  for(int iq=0; iq<gptr->GetMapNum(); iq++)
  {
    EicGeoMap *fmap = gptr->GetMapPtrViaMapID(iq);
  
    if (fmap->IsMySignature(lvVolumeIds)) 
    {
      ret = (ULong64_t(iq) & _SERVICE_BIT_MASK_) << _GEANT_INDEX_BIT_NUM_; 

      // Tune shifts to this particular map;
      for(int lv=0; lv<fmap->GetGeantVolumeLevelNum(); lv++)
      {
	const GeantVolumeLevel *lptr          = fmap->GetGeantVolumeLevelPtr(lv);
	const EicBitMask<UGeantIndex_t> *mask = lptr->GetBitMaskPtr();

	// Skip "dummy" levels (for instance individual fibers);
	if (!lptr->GetMaxEntryNum()) continue;

	// Check range;
	if (lvNodeIds[lv] >= lptr->GetMaxEntryNum()) 
	{
	  // Just a warning; want to see this situation first;
	  std::cout<<"-E- Eic"<< mDetectorName->Name() <<": " << 
	    "Sensitive volume multi-index too large!" << std::endl;

	  // Restore GEANT4 geo manager state if had to switch it;
	  if (!returnBackPath.IsNull()) gGeoManager->cd(returnBackPath);
	  return _64BIT_VALUE_INVALID_;
	} //if

	// Well, in fact there is no need to mask out bits here (see check above);
	ret |= (lvNodeIds[lv] & mask->GetBitMask()) << mask->GetShift();
      } //for lv

      // Well, consider to fill out base name if not done so yet; obviously, 
      // if no hits were registered at this map, string will not be filled 
      // out at all, which is Ok;
      if (fmap->GetBaseVolumePath()->IsNull()) 
      {
	// Record current path; 
	TString path = gGeoManager->GetPath();
	
	// Go few levels up;
	for(int lv=0; lv<fmap->GetGeantVolumeLevelNum(); lv++)
	  gGeoManager->CdUp();

	fmap->AssignBaseVolumePath(gGeoManager->GetPath());

	// Return back in the node tree;
	gGeoManager->cd(path);
      } //if

      break;
    } /*if*/
  } //for iq

  // Return back to the insensitive neighboring volume where GEANT4 probably 
  // ended up exiting from the "true" sensitive volume;
  if (!returnBackPath.IsNull()) gGeoManager->cd(returnBackPath);

  // Do it better later; at least don't care about this warning for all-sensitive
  // (basically dummy) detectors in a special-purpose mode (say neutron flux calculation);
  if (ret == _64BIT_VALUE_INVALID_ && !mAllVolumesSensitiveFlag) 
    printf("%s vs %s\n", gGeoManager->GetPath(), fPathUponEntry.Data());

  return ret;
} // EicDetector::GetNodeMultiIndex()

// ---------------------------------------------------------------------------------------

void EicDetector::FinishRun()
{
#if _TODAY_
  FairRun *fRun = FairRun::Instance();

  // I guess there is no need to save/restore current directory here?;
  fRun->GetOutputFile()->cd();
#endif

  if (gptr) gptr->Write(mDetectorName->Name() + "GeoParData");

  // As of 2013/09/29 just dump detector geometry ROOT record into the MC output
  // file; disk space overhead is indeed small; MC files can now be used to import 
  // detector geometry in calls like EicEmc() which comes handy for instance for 
  // safe shower database creation;
  if (vptr) vptr->Write(mDetectorName->Name() + "GeantGeoWrapper"); 
} // EicDetector::FinishRun()

// =======================================================================================
