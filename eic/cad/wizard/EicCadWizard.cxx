
#include <assert.h>       
#include <sys/stat.h>
#include <dirent.h> 

#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <StlAPI_Writer.hxx>

#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <Message_PrinterOStream.hxx>

#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <Geom_Surface.hxx>

#include <Geom_Plane.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_SphericalSurface.hxx>
#include <Geom_ToroidalSurface.hxx>

#include <Geom_BSplineSurface.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>

#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepBuilderAPI_Transform.hxx>

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

#include <gp_Pln.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Cone.hxx>
#include <gp_Sphere.hxx>
#include <gp_Torus.hxx>

#include <TopExp_Explorer.hxx>

#include <TGeoCompositeShape.h>
#include <TGeoMatrix.h>
#include <TGeoSphere.h>

#include <EicStlFactory.h>
#include <EicCadWizard.h>
// FIXME: get rid of this include file;
#include <EicCadFileConfig.h>
#include <EicCompositeShape.h>

#define _DOT_STEP_   (".stp")
#define _DOT_ROOT_  (".root")
#define _DOT_SPLIT_ (".split")

// =======================================================================================

EicCadWizardFile *EicCadWizard::AddSourceFile(const char *fname, const char *material) {
  EicCadWizardFile *wfile = new EicCadWizardFile(fname, material);

  mSourceFiles.push_back(wfile);
  
  return wfile;
} // EicCadWizard::AddSourceFile()

// ---------------------------------------------------------------------------------------
#if _LATER_
void EicCadWizard::AddSourceDirectory(const char *dname) {
  struct stat St;
  unsigned stlen = strlen(_DOT_STP_);
  // NB: stat(), since I want to dereference the link;
  stat(dname, &St);
    
  DIR *curr_dir = opendir(dname); assert(curr_dir);
  struct dirent *curr_file;

  while((curr_file = readdir(curr_dir))) {
    int len = strlen(curr_file->d_name);

    if (len >= stlen && 
	!memcmp(curr_file->d_name + len - stlen, _DOT_STP_, stlen)) {
      TString fileName = TString(dname) + "/" + curr_file->d_name;
      AddSourceFile(fileName.Data());
    } //if
  } //while
} // EicCadWizard::AddSourceDirectory()
#endif
// ---------------------------------------------------------------------------------------

int EicCadWizard::ConvertSourceFilesToRoot( void )
{ 
  // Either convert or check already existing conversion for all files recursively;
  for(unsigned fl=0; fl<mSourceFiles.size(); fl++) {
    EicCadWizardFile *wfile = mSourceFiles[fl];

    //std::vector<EicCadWizardCut*> *tried_cuts = new std::vector<EicCadWizardCut*>;
    std::vector<std::pair<EicCadWizardCut*,bool> > *tried_cuts = new std::vector<std::pair<EicCadWizardCut*,bool> >;

    {
      int ret = ConvertSourceFileToRoot(wfile, tried_cuts);

      if (ret) return ret;
    }
  } //for fl

  return 0;
} // EicCadWizard::ConvertSourceFilesToRoot()

// ---------------------------------------------------------------------------------------

Bool_t EicCadWizard::IsElementaryFace(const TopoDS_Face &face) const
{
  Handle(Geom_Surface) SS = BRep_Tool::Surface(face);

  return (SS->IsKind(STANDARD_TYPE(Geom_Plane)) 
	  || SS->IsKind(STANDARD_TYPE(Geom_CylindricalSurface)) 
	  || SS->IsKind(STANDARD_TYPE(Geom_ConicalSurface)) 
	  || SS->IsKind(STANDARD_TYPE(Geom_ToroidalSurface)) 
	  || SS->IsKind(STANDARD_TYPE(Geom_SphericalSurface)));
} // EicCadWizard::IsElementaryFace()

// ---------------------------------------------------------------------------------------

Bool_t EicCadWizard::IsKnownFace(const TopoDS_Face &face) const
{
  if (IsElementaryFace(face)) return true;

  {
    Handle(Geom_Surface) SS = BRep_Tool::Surface(face);

    return (SS->IsKind(STANDARD_TYPE(Geom_BSplineSurface)) 
	    || SS->IsKind(STANDARD_TYPE(Geom_RectangularTrimmedSurface)));
  }
} // EicCadWizard::IsKnownFace()

// ---------------------------------------------------------------------------------------

//
// FIXME: need to pass the vector of cuts, which were used so far (so there is not need
// to try them again);
//

EicCadWizardCut *EicCadWizard::GetCut(const TopoDS_Face &face, const gp_Pnt *bcenter, double bradius)
{
  Handle(Geom_Surface) SS = BRep_Tool::Surface(face);

  // Well, here I have to resort to a switch;
  if (SS->IsKind(STANDARD_TYPE(Geom_Plane))) {
    Handle(Geom_Plane) es = Handle(Geom_Plane)::DownCast(SS);
    
    return new EicCadWizardPlane(es->Pln());
  } else
  if (SS->IsKind(STANDARD_TYPE(Geom_CylindricalSurface))) {
    Handle(Geom_CylindricalSurface) es = Handle(Geom_CylindricalSurface)::DownCast(SS);
    
    return new EicCadWizardCylinder(es->Cylinder(), bcenter, bradius);
  } 
  else
  if (SS->IsKind(STANDARD_TYPE(Geom_ConicalSurface))) {
    Handle(Geom_ConicalSurface) es = Handle(Geom_ConicalSurface)::DownCast(SS);

    return new EicCadWizardCone(es->Cone(), bcenter, bradius);
  }
  else
  if (SS->IsKind(STANDARD_TYPE(Geom_SphericalSurface))) {
    Handle(Geom_SphericalSurface) es = Handle(Geom_SphericalSurface)::DownCast(SS);

    return new EicCadWizardSphere(es->Sphere());
  }
  else
  if (SS->IsKind(STANDARD_TYPE(Geom_ToroidalSurface))) {
    Handle(Geom_ToroidalSurface) es = Handle(Geom_ToroidalSurface)::DownCast(SS);

    return new EicCadWizardTorus(es->Torus());
  } //if

  // Can not happen;
  return 0;
} // EicCadWizard::GetCut()

// ---------------------------------------------------------------------------------

int EicCadWizard::ConvertSourceDirectoryToRoot(const char *dname, EicCadWizardFileConfig *config, 
					       const std::vector<std::pair<EicCadWizardCut*,bool> > *tried_cuts)
{
  struct stat St;
  unsigned fCounter = 0, stlen = strlen(_DOT_STEP_);
  // NB: stat(), since I want to dereference the link;
  stat(dname, &St);
    
  //printf("%d %d\n", S_ISLNK(St.st_mode), S_ISDIR(St.st_mode));
  DIR *curr_dir = opendir(dname); assert(curr_dir);
  struct dirent *curr_file;

  while((curr_file = readdir(curr_dir))) {
    int len = strlen(curr_file->d_name); 
    //printf("%d\n", len);

    if (len >= stlen && 
	!memcmp(curr_file->d_name + len - stlen, _DOT_STEP_, stlen)) {
      TString fileName = TString(dname) + "/" + curr_file->d_name;
      //printf("--> %s\n", fileName.Data());
      int ret = ConvertSourceFileToRoot(fileName.Data(), config, tried_cuts);
      if (ret) return ret;
      
      fCounter++;
    } //if
  } //while

    //printf("fCounter: %d\n", fCounter);
    // FIXME: do it better; for now assume at least two files should have been there;
  //assert(fCounter >= 2);
  return 0;
} // EicCadWizard::ConvertSourceDirectoryToRoot()

// ---------------------------------------------------------------------------------------

int EicCadWizard::AccountRootFile(const TString &fname)
{
  mRootFiles.push_back(fname);

  return 0;
} // EicCadWizard::AccountRootFile()

// ---------------------------------------------------------------------------------------

#include <TKey.h>
#include <TGeoTube.h>

#if 1//_OFF_
static void SetDefaultMatrixName(TGeoMatrix* matrix)
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
  printf("Here: %s\n", matrix->GetName());
}
#endif

//
// (?) sudo sysctl -w fs.file-max=100000
//

void EicCadWizard::AddRootDirectory(const char *dname) {
  unsigned rtlen = strlen(_DOT_ROOT_), splen = strlen(_DOT_SPLIT_);
    
  printf("--> %s ... \n", dname);
  DIR *curr_dir = opendir(dname); 
  if (!curr_dir) printf("errno: %d\n", errno);
  assert(curr_dir);
  struct dirent *curr_file;

  while((curr_file = readdir(curr_dir))) {
    if (!strcmp(curr_file->d_name, ".") || !strcmp(curr_file->d_name, "..")) continue;

    struct stat St;
    TString fname = TString(dname) + "/" + curr_file->d_name;
#if 1//_LATER_
    // NB: stat(), since I want to dereference the link;
    stat(fname.Data(), &St);
    if (S_ISDIR(St.st_mode) && strlen(fname) > splen && 
	// FIXME: do it better; 
	TString(fname).EndsWith(_DOT_SPLIT_)) 
      AddRootDirectory(fname.Data());
#endif

    int len = strlen(curr_file->d_name);

    if (len >= rtlen && 
	!memcmp(curr_file->d_name + len - rtlen, _DOT_ROOT_, rtlen)) {
      AccountRootFile(fname.Data());
    } //if
  } //while

  closedir(curr_dir);
} // EicCadWizard::AddRootDirectory()

int EicCadWizard::AssembleRootFiles(const char *dname, const char *detname)
{
  // FIXME: do this better later; in fact this suffices I guess;
  EicCadFileConfig *config = new EicCadFileConfig();
  config->CreateMediaHub();//(char*)"air");

  //EicGeoParData *geo = new EicGeoParData("DUMMY", 0, 0);
  EicGeoParData *geo = new EicGeoParData(detname, 0, 0);
  {
    char rname[1024];
    snprintf(rname, 1024-1, "%s.root", geo->GetDetName()->name().Data());//detname);
    geo->SetFileName(rname);
  }

  config->mhub()->Init(); 

  // In this case add all .root files in this directory by hand;
  if (dname) {
    mRootFiles.clear();
    AddRootDirectory(dname);
  } //if

  unsigned vCounter = 0;

  for(unsigned fl=0; fl<mRootFiles.size(); fl++) {
    TString &fname = mRootFiles[fl];

    printf("%3d -> %s\n", fl, fname.Data());

    TFile *f = new TFile(fname.Data()); assert(f);
    TList* l= f->GetListOfKeys();

    TKey* key = (TKey*) l->At(0);  //Get the first key in the list
    TGeoVolumeAssembly *assembly = dynamic_cast<TGeoVolumeAssembly*> (key->ReadObj());
    assert(assembly);

    // NB: this way it works for both boolean and tetrahedra shapes;
    //printf("--> %d\n", assembly->GetNode(0)->GetVolume()->GetNtotal());
    unsigned node_num = assembly->GetNode(0)->GetVolume()->GetNtotal();
    // THINK: why '-1' (this way it does not crash)???;
    for(unsigned nd=0; nd<node_num-1; nd++) {
      //printf("--> %d\n", nd);
      TGeoNode *node = assembly->GetNode(0)->GetVolume()->GetNode(nd);

#if 0
      TGeoMatrix* M = node->GetMatrix();
      SetDefaultMatrixName(M);
      gGeoManager->GetListOfMatrices()->Remove(M);
      TGeoHMatrix* global = gGeoManager->GetHMatrix();
      gGeoManager->GetListOfMatrices()->Remove(global);
#endif

      TGeoVolume *volume = node->GetVolume();

      //printf("-> %s\n", volume->GetName());
      char vname[128];
      const char *oname = volume->GetName();
      //snprintf(vname, 128-1, "C%06d", vCounter++);
      printf("-> %s\n", volume->GetName());
      assert(strlen(oname) >= 4 && oname[0] == '@' && oname[2] == 'V' && oname[3] == '@');
      snprintf(vname, 128-1, "@%cV@%06d", oname[1], vCounter++);
      volume->SetName(vname);

      //volume->RegisterYourself();
      //assert(node->GetMatrix()->IsIdentity());
      // NB: new TGeoCombiTrans() is essential here even if identity;
      //geo->GetTopVolume()->AddNode(volume, 0, new TGeoCombiTrans(10.0*vCounter, 0.0, 0.0, node->GetMatrix()));
      //TGeoMatrix* M = node->GetMatrix(); M->SetDx(10.0*vCounter);
      geo->GetTopVolume()->AddNode(volume, 0, node->GetMatrix());
      volume->RegisterYourself();
    } //for nd

    f->Close();
  } //for fl

#if 0    
  TGeoMatrix* M = n->GetMatrix();
  SetDefaultMatrixName(M);

  /** NOw we can remove the matrix so that the new geomanager will rebuild it properly*/
  gGeoManager->GetListOfMatrices()->Remove(M);
  TGeoHMatrix* global = gGeoManager->GetHMatrix();
  gGeoManager->GetListOfMatrices()->Remove(global); //Remove the Identity matrix
  /**Now we can add the node to the existing cave */
  Cave->AddNode(v1,0, M);
#endif

  {
    //TGeoRotation *rw = new TGeoRotation();
    TGeoHMatrix *rw = new TGeoHMatrix();
    rw->RotateY(90); 
    //rw->SetScale(1.);
    printf("%f\n", rw->GetScale());
    
    //geo->SetTopVolumeTransformation(new TGeoCombiTrans(0.0, 0.0, 0.0, rw));
  }

  // FIXME this, please!;
  geo->GetColorTable()->AddPatternMatch       ("@RV@", kGray);
  geo->GetTransparencyTable()->AddPatternMatch("@RV@", 0);
  //geo->GetColorTable()->AddPatternMatch       ("@GV@", kSpring);
  //geo->GetTransparencyTable()->AddPatternMatch("@GV@", 50);
  geo->GetColorTable()->AddPatternMatch       ("@GV@", kGray);
  geo->GetTransparencyTable()->AddPatternMatch("@GV@", 0);

  geo->FinalizeOutput();

  return 0;
} // EicCadWizard::AssembleRootFiles()

// ---------------------------------------------------------------------------------------

int EicCadWizard::SplitAndRescaleSourceFile(const TString &fname)
{
  // FIXME: do it better later; use std::string or TString;
  assert(fname.EndsWith(_DOT_STEP_));
  char bname[1024];
  snprintf(bname, 1024-1, "%s", fname.Data());
  bname[strlen(bname)-4] = 0;

  // There are >1 solids in this file; create one level down directory and populate it 
  // with individual shapes;
  char dname[1024];
  unsigned csCounter = 0;
  snprintf(dname, 1024-1, "%s.split", bname);
  // FIXME: once debugging is over, use Boost remove_all() to clean up non-empty directory;
  mkdir(dname, 0755); 

  {
    //unsigned sCounter = 0;
    STEPControl_Reader cReader;
    IFSelect_ReturnStatus cstatus = cReader.ReadFile(fname.Data());

    // FIXME: may want to loop through all roots; should be easy fix?;
    //assert(cReader.NbRootsForTransfer() == 1);
    cReader.TransferRoots();//(1);
    printf("%d root(s) in STEP file; %d shape(s)\n", cReader.NbRootsForTransfer(), cReader.NbShapes());

    // FIXME: hardcode 90-degree rotation and scale factor 0.1;
    gp_Trsf scaling;
    {
      gp_Pnt x0(0.0, 0.0, 0.0);
      // FIXME: do it better later;
      //assert(config()->GetRotationAxis() == EicCadFileConfig::_Y_);
      gp_Dir n0(0.0, 1.0, 0.0);
      gp_Ax1 axis(x0, n0);

      scaling.SetRotation(axis, -90.0 * TMath::DegToRad());
    } //if
    // Yes, first rotate, then re-scale (otherwise scaling takes no effect);
    scaling.SetScaleFactor(0.10);
    BRepBuilderAPI_Transform sc(scaling);

    // FIXME: do this check more efficiently later;
    for(unsigned sh=0; sh<cReader.NbShapes(); sh++) {
      TopoDS_Shape shape = cReader.Shape(sh+1);
      
      for (TopExp_Explorer itf(shape,TopAbs_SOLID); itf.More(); itf.Next()) {
	STEPControl_Writer cWriter;
      
	sc.Perform(itf.Current());
	const TopoDS_Shape &solid = sc.ModifiedShape(itf.Current());

	cWriter.Transfer(solid, STEPControl_ManifoldSolidBrep);
	char qname[1024];
	snprintf(qname, 1024-1, "%s/part-%05d.stp", dname, csCounter);
	cWriter.Write(qname);
	
	// FIXME: and need to deal with these files right away here, recursively; later
	// on - in multi-threading mode - this will not interrupt the processing sequence;
	
	csCounter++;
      } //for itf

      //for (TopExp_Explorer itf(shape,TopAbs_SOLID); itf.More(); itf.Next())
      //sCounter++;
    } //for sh

    //printf("-> sCounter %d\n", sCounter);	
  }

  return 0;
} // EicCadWizard::SplitAndRescaleSourceFile()

// ---------------------------------------------------------------------------------------

int EicCadWizard::ConvertSourceFileToRoot(const TString &fname, EicCadWizardFileConfig *config, 
					  const std::vector<std::pair<EicCadWizardCut*,bool> > *tried_cuts)
{
  printf("\n  Entering EicCadWizard::ConvertSourceFileToRoot(): %s ...\n", fname.Data());
  // FIXME: do it better later; use std::string or TString;
  assert(fname.EndsWith(_DOT_STEP_));
  char bname[1024];
  snprintf(bname, 1024-1, "%s", fname.Data());
  bname[strlen(bname)-4] = 0;
  //printf("%s\n", bname);

  // Check #1: if ROOT file exists already, assume there is nothing else to do;
  {
    char rfile[1024];
    
    snprintf(rfile, 1024-1, "%s.root", bname); 
    if (!access(rfile, R_OK)) return AccountRootFile(rfile);
  }

  // Check #2: if split link to a directory exists already, deal with respective 
  // root files in the subdirectory, recursively; then return;
  {
    char slink[1024];

    snprintf(slink, 1024-1, "%s.split", bname); 
    //printf("%s\n", slink);
    if (!access(slink, R_OK)) return ConvertSourceDirectoryToRoot(slink, config, tried_cuts);
  }   

  // Ok, should work on this file; first figure out whether there are more than one solid
  // there (almost certainly indicating it is a primary model file; or a bug :-);
  {
    unsigned sCounter = 0;
    STEPControl_Reader cReader;
    IFSelect_ReturnStatus cstatus = cReader.ReadFile(fname.Data());

    // FIXME: may want to loop through all roots; should be easy fix?;
    //assert(cReader.NbRootsForTransfer() == 1);
    cReader.TransferRoots();//(1);
    printf("%d root(s) in STEP file; %d shape(s)\n", cReader.NbRootsForTransfer(), cReader.NbShapes());

    //gp_Trsf scaling;
    //scaling.SetScaleFactor(10.0);//config()->scale() * config()->units());
    //BRepBuilderAPI_Transform sc(scaling);

    // FIXME: do this check more efficiently later;
    for(unsigned sh=0; sh<cReader.NbShapes(); sh++) {
      TopoDS_Shape shape = cReader.Shape(sh+1);
      
      for (TopExp_Explorer itf(shape,TopAbs_SOLID); itf.More(); itf.Next())
	sCounter++;
    } //for sh

    // FIXME: handle this situation correctly;
    //assert(sCounter);
    if (!sCounter) unlink(fname.Data());
    printf("-> sCounter %d\n", sCounter);

    if (sCounter == 1) {
      // Check if flat cut makes sense; otherwise proceed with the FaceGuidedSplit();
      //+if (!FlatSurfaceSplit(cReader, bname, config, tried_cuts)) return 0;
      
      return FaceGuidedSplit(cReader, bname, config, tried_cuts);
    }
    else 
      // FIXME: should also be some sort of recursion up to the very end of the 
      // dissection procedure;
      return StraightforwardSplit(cReader, bname);
  }

  return 0;
} // EicCadWizard::ConvertSourceFileToRoot()

// ---------------------------------------------------------------------------------------

class ObjectFace {
public:
  ObjectFace(TopoDS_Face *face): mFace(face), xMin(0.0), yMin(0.0), zMin(0.0), 
				 xMax(0.0), yMax(0.0), zMax(0.0) {};
  ~ObjectFace() {};

  const TopoDS_Face *mFace;
  double xMin, yMin, zMin, xMax, yMax, zMax, mMin[3], mMax[3];
};

int EicCadWizard::FlatSurfaceSplit(const STEPControl_Reader &cReader, const char *bname, 
				   EicCadWizardFileConfig *config, 
				   const std::vector<std::pair<EicCadWizardCut*,bool> > *tried_cuts)
{
  // So need to deal with a single solid; perform cuts; FIXME: re-use 'itf' later;
  // for now just re-enter the same loop; 
  for(unsigned sh=0; sh<cReader.NbShapes(); sh++) {
    TopoDS_Shape shape = cReader.Shape(sh+1);
    
    // NB: here I do know there is a single unique shape available;
    for (TopExp_Explorer itf(shape,TopAbs_SOLID); itf.More(); itf.Next()) {
      const TopoDS_Shape &solid = itf.Current();

      // FIXME: avoid duplication here and in EicCadWizard::FaceGuidedSplit();
      if (config->mForcedStlSolidCreation) {
	CreateStlSolidRootFile(solid, bname, config);
	return 0;
      } //if
      //if (config->mForcedGhostSolidCreation) {
      if (config->mMaterial.IsNull()) {
	CreateGhostSolidRootFile(solid, bname, config);
	return 0;
      } //if

      gp_Pnt *bcenter;
      double bradius;
      {
	double xMin, yMin, zMin, xMax, yMax, zMax;
	Bnd_Box Boundary;
	BRepBndLib::Add(solid, Boundary);
	    
	Boundary.Get(xMin, yMin, zMin, xMax, yMax, zMax);
	
	// Be lazy; determine bounding sphere;
	bcenter = new gp_Pnt((xMin+xMax)/2., (yMin+yMax)/2., (zMin+zMax)/2.);
	bradius = sqrt(SQR(xMax-xMin) + SQR(yMax-yMin) + SQR(zMax-zMin))/2.;
      }

      {
	std::vector<ObjectFace*> faces;
	// Get boundary box for the whole object;
	double gxMin, gyMin, gzMin, gxMax, gyMax, gzMax;
	Bnd_Box Gboundary;
	BRepBndLib::Add(solid, Gboundary);
	    
	Gboundary.Get(gxMin, gyMin, gzMin, gxMax, gyMax, gzMax);

	// Allocate all faces (except for those which were tried out already)
	// together with their boundary boxes;
	for (TopExp_Explorer itg(solid,TopAbs_FACE); itg.More(); itg.Next()) {
	  {
	    TopoDS_Face &face = (TopoDS_Face&)itg.Current();
	    
	    Handle(Geom_Surface) SS = BRep_Tool::Surface(face);
	    cout << " " << SS->DynamicType() << endl; 
	    if (IsElementaryFace(face)) {
	      // Elementary face -> get cutting surface;
	      EicCadWizardCut *fcut = GetCut(face, bcenter, bradius);

	      bool tried = false;
	      for(unsigned tct=0; tct<tried_cuts->size(); tct++) {
		EicCadWizardCut *tcut = (*tried_cuts)[tct].first;
		
		if (fcut->IsEqual(tcut)) {
		  tried = true;
		  break;
		} //if
	      } //for tct

	      // No reason to account for this face in the flat cut procedure;
	      if (tried) continue;
	    } //if
	  }

	  // Otherwise proceed with the allocation;
	  ObjectFace *objface = new ObjectFace(&(TopoDS_Face&)itg.Current());

	  Bnd_Box Boundary;
	  BRepBndLib::Add(itg.Current(), Boundary);
	  Boundary.Get(objface->xMin, objface->yMin, objface->zMin, 
		       objface->xMax, objface->yMax, objface->zMax);
	  // FIXME: this looks stupid;
	  objface->mMin[0] = objface->xMin; objface->mMin[1] = objface->yMin; objface->mMin[2] = objface->zMin; 
	  objface->mMax[0] = objface->xMax; objface->mMax[1] = objface->yMax; objface->mMax[2] = objface->zMax; 
	  
	  faces.push_back(objface);
	} //for itg

	printf("%3d non-trivial faces total!\n", faces.size());

	// Well, some sanity must be there; require at least that many non-trivial faces;
	if (faces.size() <= 10) return -1;

	// Now loop through all 3 dimensions, try out N locations of splitting 
	// plane and see whether can separate faces in a better way; FIXME: 
	// something like a binary tree approximation is due here;
	{
	  int best_iq = -1;
	  // FIXME: even in this ugly approach should do this parameter configurable;
	  unsigned ndim = 10, best_st = 0, best_metric = 0;
	  double gMin[3] = {gxMin, gyMin, gzMin}, gMax[3] = {gxMax, gyMax, gzMax}, gStep[3];
	  for(unsigned iq=0; iq<3; iq++) 
	    gStep[iq] = (gMax[iq] - gMin[iq])/ndim;

	  for(unsigned iq=0; iq<3; iq++) {
	    for(unsigned st=1; st<ndim; st++) {
	      double coord = gMin[iq] + st*gStep[iq];

	      // Calculate number of left-side/crossed/right-side faces;
	      int left = 0, crossed = 0, right = 0;
	      for(unsigned fc=0; fc<faces.size(); fc++) {
		ObjectFace *objface = faces[fc];
		
		if (objface->mMin[iq] <= coord && objface->mMax[iq] <= coord)
		  left++;
		else 
		  if(objface->mMin[iq] >= coord && objface->mMax[iq] >= coord)
		    right++;
		  else
		    crossed++;
	      } //for fc

	      // This cut would be useless, nothing to talk about;
	      if (!left || !right) continue;

	      unsigned metric = abs(left - right) + crossed;
	      if (best_iq == -1 || metric < best_metric) {
		best_iq = iq;
		best_st = st;
		best_metric = metric;
	      } //if

	      printf("%3d %3d -> %4d %4d %4d -> %4d\n", iq, st, left, crossed, right, abs(left-right)+crossed);
	    } //for st
	  } //for iq

	  printf("--> %d %d -> %3d\n", best_iq, best_st, best_metric);
	  if (best_iq == -1) return -1;

	  // FIXME: make configurable; anyway, for now take this decision here, make a cut
	  // and create 'split' subdirectory without any further considerations;
	  //if (best_metric + 2 >= faces.size()) return -1;

	  // Define the cutting plane;
	  double pt[3], nn[3];
	  for(unsigned iq=0; iq<3; iq++) {
	    pt[iq] = best_iq == iq ? gMin[iq] + best_st*gStep[iq] : 0.0;
	    nn[iq] = best_iq == iq ? 1.0 : 0.0;
	  } //for iq
	  gp_Pnt x0(pt[0], pt[1], pt[2]);
	  gp_Dir n0(nn[0], nn[1], nn[2]);
	  gp_Pln pl(x0, n0);

	  // This is my best guess flat cut;
	  EicCadWizardCut *fcut = new EicCadWizardPlane(pl);

	  // Check that it was not used already;
	  for(unsigned tct=0; tct<tried_cuts->size(); tct++) {
	    EicCadWizardCut *tcut = (*tried_cuts)[tct].first;
		
	    // Yes, just return failure of this attempt;
	    if (fcut->IsEqual(tcut)) return -1;
	  } //for tct

	  TopoDS_Shape commonHalf = BRepAlgoAPI_Common(solid, fcut->GetSolid());
	  for (TopExp_Explorer itq(commonHalf,TopAbs_SOLID); itq.More(); itq.Next())
	    fcut->AddCommonSolid(new TopoDS_Shape(itq.Current()));
	  
	  TopoDS_Shape cutHalf = BRepAlgoAPI_Cut(solid, fcut->GetSolid());
	  for (TopExp_Explorer itq(cutHalf,TopAbs_SOLID); itq.More(); itq.Next())
	    fcut->AddCutSolid(new TopoDS_Shape(itq.Current()));
	  
	  //@@@fcut->mSubtractionFlag = fcut->GetCutSolidsCount();
	  
	  printf("-> multi %3d; --> %3d %3d\n", 
		 fcut->GetMultiplicity(), fcut->GetCommonSolidsCount(), fcut->GetCutSolidsCount());

	  // Yes, this would be a failure; 
	  if (fcut->GetCommonSolidsCount() + fcut->GetCutSolidsCount() <= 1) return -1;
 		
	  {
	    char dname[1024];
	    unsigned csCounter = 0;
	    std::vector<TopoDS_Shape*> *arr[2] = {&fcut->mCommonSolids, &fcut->mCutSolids};
	    
	    snprintf(dname, 1024-1, "%s.cut-%05d", bname, 0);
	    // FIXME: once debugging is over, use Boost remove_all() to clean up non-empty directory;
	    mkdir(dname, 0755);

	    for(unsigned gr=0; gr<2; gr++) {
	      std::vector<TopoDS_Shape*> *group = arr[gr];
	      std::vector<std::pair<EicCadWizardCut*,bool> > *fwd_cuts = new std::vector<std::pair<EicCadWizardCut*,bool> >;
	      for(unsigned tct=0; tct<tried_cuts->size(); tct++) 
		fwd_cuts->push_back((*tried_cuts)[tct]);
	      fwd_cuts->push_back(std::pair<EicCadWizardCut*,bool>(fcut, gr == 1));
	      
	      for(unsigned pt=0; pt<group->size(); pt++) {
		const TopoDS_Shape &qshape = *(*group)[pt];
				
		STEPControl_Writer cWriter;
		
		cWriter.Transfer(qshape, STEPControl_ManifoldSolidBrep);
		char qname[1024];
		snprintf(qname, 1024-1, "%s/part-%05d.stp", dname, csCounter);
		cWriter.Write(qname);

		{
		  //static unsigned counter;
		  
		  //if (counter == 1) { 
		  int ret = ConvertSourceFileToRoot(qname, config, fwd_cuts);
		  if (ret) return ret;
		  //}
		  //counter++;
		} 		  

		csCounter++;
	      } //for pt
	    } //for gr
	  }

	  {
	    // Create .split link;
	    char dname[1024], lname[1024];
	    // FIXME: this looks like a hack;
	    snprintf(dname, 1024-1, "%s.cut-%05d", strrchr(bname, '/') + 1, 0);
	    snprintf(lname, 1024-1, "%s.split", bname);
	    unlink(lname);
	    symlink(dname, lname);

#if _OLD_
	    {
	      // Copy over tried_cuts[] vector and append it with this selected cut;
	      std::vector<std::pair<EicCadWizardCut*,bool> > *fwd_cuts = new std::vector<std::pair<EicCadWizardCut*,bool> >;
	      for(unsigned tct=0; tct<tried_cuts->size(); tct++) 
		fwd_cuts->push_back((*tried_cuts)[tct]);
	      fwd_cuts->push_back(std::pair<EicCadWizardCut*,bool>(fcut,true));

	      int ret = ConvertSourceDirectoryToRoot(lname, config, fwd_cuts);
	      if (ret) return ret;
	    }
#endif
	  }
	}
      }
    } //for itf
  } //for sh

  return 0;
} // EicCadWizard::FlatSurfaceSplit()

// ---------------------------------------------------------------------------------------

int EicCadWizard::FaceGuidedSplit(const STEPControl_Reader &cReader, const char *bname, 
				  EicCadWizardFileConfig *config, 
				  const std::vector<std::pair<EicCadWizardCut*,bool> > *tried_cuts)
{
  // So need to deal with a single solid; perform cuts; FIXME: re-use 'itf' later;
  // for now just re-enter the same loop; 
  for(unsigned sh=0; sh<cReader.NbShapes(); sh++) {
    TopoDS_Shape shape = cReader.Shape(sh+1);
    
    // NB: here I do know there is a single unique shape available;
    for (TopExp_Explorer itf(shape,TopAbs_SOLID); itf.More(); itf.Next()) {
      const TopoDS_Shape &solid = itf.Current();

      if (config->mForcedStlSolidCreation) {
	CreateStlSolidRootFile(solid, bname, config);
	return 0;
      } //if
      //if (config->mForcedGhostSolidCreation) {
      if (config->mMaterial.IsNull()) {
	CreateGhostSolidRootFile(solid, bname, config);
	return 0;
      } //if

      // THINK: is it really harmless to keep this boundary sphere global in view 
      // of recursion?; NB: will certainly fail in multi-threaded mode;
      gp_Pnt *bcenter;
      double bradius;
      {
	double xMin, yMin, zMin, xMax, yMax, zMax;
	Bnd_Box Boundary;
	BRepBndLib::Add(solid, Boundary);
	    
	Boundary.Get(xMin, yMin, zMin, xMax, yMax, zMax);
	
	// Be lazy; determine bounding sphere;
	bcenter = new gp_Pnt((xMin+xMax)/2., (yMin+yMax)/2., (zMin+zMax)/2.);
	bradius = sqrt(SQR(xMax-xMin) + SQR(yMax-yMin) + SQR(zMax-zMin))/2.;
      }

      {
	bool has_complex_faces = false;
	// FIXME: may want to get rid of uCounter;
	unsigned faceCounter = 0, uCounter = 0;
	for (TopExp_Explorer itg(solid,TopAbs_FACE); itg.More(); itg.Next()) 
	  faceCounter++;
	printf("%3d faces total!\n", faceCounter);

	{
	  // FIXME: some sort of std::map would help;
	  std::vector<std::pair<EicCadWizardCut*,bool> > *cuts = new std::vector<std::pair<EicCadWizardCut*,bool> >;
	  //std::vector<EicCadWizardCut*> *cuts = new std::vector<EicCadWizardCut*>;
	  	  
	  std::vector<std::vector<std::pair<std::string,bool> > > vgdumb;

	  unsigned single_piece_face_count_max_min = 0, ct_best = 0, tried_already = 0;
	  for (TopExp_Explorer itg(solid,TopAbs_FACE); itg.More(); itg.Next()) {
	    TopoDS_Face &face = (TopoDS_Face&)itg.Current();
	    
	    std::vector<std::pair<std::string,bool> > vldumb;

	    Handle(Geom_Surface) SS = BRep_Tool::Surface(face);
	    cout << " " << SS->DynamicType() << endl; 
	    if (!IsElementaryFace(face)) {
	      has_complex_faces = true;
	      continue;
	    } //if

	    // Elementary face -> get cutting surface;
	    EicCadWizardCut *fcut = GetCut(face, bcenter, bradius);

	    // Check whether it was tried out already;
	    {
	      bool exists = false;

	      for(unsigned ct=0; ct<cuts->size(); ct++) {
		EicCadWizardCut *cut = (*cuts)[ct].first;
		
		if (cut->IsEqual(fcut)) {
		  cut->IncrementMultiplicity();

		  exists = true;
		  break;
		} //if
	      } //for ct

	      if (exists) continue;
	    } 

	    // Account this (new) cut in the cutting surface array;
	    cuts->push_back(std::pair<EicCadWizardCut*,bool>(fcut,true));
	    //cuts->push_back(fcut);//,true));

	    // Check whether it was tried before (then there is no chance 
	    // one gets a non-trivial split);
	    {
	      bool tried = false;
	      
	      for(unsigned tct=0; tct<tried_cuts->size(); tct++) {
		EicCadWizardCut *tcut = (*tried_cuts)[tct].first;
		
		if (fcut->IsEqual(tcut)) {
		  // Yes, this suffices;
		  //fcut->mSubtractionFlag = tcut->mSubtractionFlag;
		  (*cuts)[cuts->size()-1].second = (*tried_cuts)[tct].second;

		  tried_already++;
		  tried = true;
		  break;
		} //if
	      } //for tct

	      if (tried) goto _continue;
	    } 
	      
	    // Cut has not been tried yet -> do it now;
	    {
	      TopoDS_Shape commonHalf = BRepAlgoAPI_Common(solid, fcut->GetSolid());
	      for (TopExp_Explorer itq(commonHalf,TopAbs_SOLID); itq.More(); itq.Next())
		fcut->AddCommonSolid(new TopoDS_Shape(itq.Current()));
	    
	      TopoDS_Shape cutHalf = BRepAlgoAPI_Cut(solid, fcut->GetSolid());
	      for (TopExp_Explorer itq(cutHalf,TopAbs_SOLID); itq.More(); itq.Next())
		fcut->AddCutSolid(new TopoDS_Shape(itq.Current()));

	      //fcut->mSubtractionFlag = fcut->GetCutSolidsCount();
	      (*cuts)[cuts->size()-1].second = fcut->GetCutSolidsCount();
	      // Account this (new) cut in the cutting surface array;
	      //cuts->push_back(std::pair<EicCadWizardCut*,bool>(fcut,fcut->GetCutSolidsCount()));

	      printf("%4d -> multi %3d; --> %3d %3d\n", 
		     cuts->size()-1, fcut->GetMultiplicity(), fcut->GetCommonSolidsCount(), fcut->GetCutSolidsCount());
	    } 		
	    
	    // Save all cut options, which keep splitting the file in smaller objects; 
	    if (fcut->GetCommonSolidsCount() + fcut->GetCutSolidsCount() >= 2) {
	      char dname[1024];
	      unsigned csCounter = 0, fCounter[2] = {0, 0}, single_piece_face_count_max = 0;
	      std::vector<TopoDS_Shape*> *arr[2] = {&fcut->mCommonSolids, &fcut->mCutSolids};
	      
	      snprintf(dname, 1024-1, "%s.cut-%05d", bname, cuts->size()-1);
	      // FIXME: once debugging is over, use Boost remove_all() to clean up non-empty directory;
	      mkdir(dname, 0755); 
	      
	      for(unsigned gr=0; gr<2; gr++) {
		std::vector<TopoDS_Shape*> *group = arr[gr];
		
		for(unsigned pt=0; pt<group->size(); pt++) {
		  const TopoDS_Shape &qshape = *(*group)[pt];
		  
		  unsigned single_piece_face_count = 0;
		  for (TopExp_Explorer itx(qshape,TopAbs_FACE); itx.More(); itx.Next()) {
		    fCounter[gr]++;
		    single_piece_face_count++;
		  } //for itx
		  if (single_piece_face_count > single_piece_face_count_max) 
		    single_piece_face_count_max = single_piece_face_count;
		  
		  STEPControl_Writer cWriter;
		  
		  cWriter.Transfer(qshape, STEPControl_ManifoldSolidBrep);
		  char qname[1024];
		  snprintf(qname, 1024-1, "%s/part-%05d.stp", dname, csCounter);
		  cWriter.Write(qname);
		  
		  vldumb.push_back(std::pair<std::string,bool>(qname, gr == 1));

		  csCounter++;
		} //for pt
	      } //for gr
	      
	      printf("    --> %4d & %4d faces total; max per piece: %4d!\n", 
		     fCounter[0], fCounter[1], single_piece_face_count_max);
	      
	      if (!single_piece_face_count_max_min || 
		  single_piece_face_count_max < single_piece_face_count_max_min) {
		ct_best = cuts->size()-1;
		single_piece_face_count_max_min = single_piece_face_count_max;
	      } //if
	      
	      uCounter++;
	    } //if 

	  _continue:
	    vgdumb.push_back(vldumb);
	  } //for itg

	  printf("%d new cuts total (and %d tried out already)!\n", cuts->size(), tried_already);

	  if (uCounter) {
	    // Create .split link to the best option;
	    char dname[1024], lname[1024];
	    // FIXME: this looks like a hack;
	    snprintf(dname, 1024-1, "%s.cut-%05d", strrchr(bname, '/') + 1, ct_best);
	    snprintf(lname, 1024-1, "%s.split", bname);
	    unlink(lname);
	    symlink(dname, lname);

	    {
	      printf("@@@ %d %d\n", vgdumb.size(), ct_best);
	      std::vector<std::pair<std::string,bool> > &vldumb = vgdumb[ct_best];
	      for(unsigned fl=0; fl<vldumb.size(); fl++) {
		std::vector<std::pair<EicCadWizardCut*,bool> > *fwd_cuts = new std::vector<std::pair<EicCadWizardCut*,bool> >;
		for(unsigned tct=0; tct<tried_cuts->size(); tct++) 
		  fwd_cuts->push_back((*tried_cuts)[tct]);
		fwd_cuts->push_back(std::pair<EicCadWizardCut*,bool>((*cuts)[ct_best].first, vldumb[fl].second));
		
		int ret = ConvertSourceFileToRoot(vldumb[fl].first, config, fwd_cuts);
		if (ret) return ret;
	      } //for fl

	      // Copy over tried_cuts[] vector and append it with this selected cut;
	      //std::vector<std::pair<EicCadWizardCut*,bool> > *fwd_cuts = new std::vector<std::pair<EicCadWizardCut*,bool> >;
	      //for(unsigned tct=0; tct<tried_cuts->size(); tct++) 
	      //fwd_cuts->push_back((*tried_cuts)[tct]);
	      //fwd_cuts->push_back((*cuts)[ct_best]);

	      //int ret = ConvertSourceDirectoryToRoot(lname, config, fwd_cuts);
	      //if (ret) return ret;
	    }
	  } else {
	    //{
	    //std::vector<EicCadWizardCut*> *fwd_cuts = new std::vector<EicCadWizardCut*>;
	    //return FaceGuidedSplit(cReader, bname, config, fwd_cuts); 
	    //}

	    // Now, since no more cuts possible, take the decision;
	    if (has_complex_faces) {
	      // FIXME: later on may want to try out non-face cuts in order to isolate 
	      // parts of the solid, which may still be represented as a boolean operation
	      // on basic shapes;
	      //assert(config->mStlSolidCreationAllowed);
	      
	      printf("Creating ROOT file with tetrahedra (shape has a complex face, sorry) ...\n");
	      
	      CreateStlSolidRootFile(solid, bname, config);
	    } else {
	      // FIXME: this is leas efficient than in the original EicCadFile implementation,
	      // but much more straightforward; the loop over 
	      if (config->mBooleanSolidCreationAllowed) {
		printf("Decomposing into basic ROOT shapes ...\n");
		
		CreateBooleanSolidRootFile(solid, bcenter, bradius, cuts, bname, config);
	      } else {
		//assert(config->mStlSolidCreationAllowed);

		printf("Creating ROOT file with tetrahedra (Boolean representation disabled, sorry) ...\n");
		
		CreateStlSolidRootFile(solid, bname, config);
	      } //if
	    } //if
	  } //if
	}
      }
    } //for itf
  } //for sh

  return 0;
} // EicCadWizard::FaceGuidedSplit()

// ---------------------------------------------------------------------------------------

//
// Essentially an adapted EicCadFile::DumpAsStlSolid() call;
//

int EicCadWizard::CreateStlSolidRootFile(const TopoDS_Shape &solid, const char *bname, 
					 EicCadWizardFileConfig *config)
{
  char fname[1024];
  snprintf(fname, 1024-1, "%s.stl", bname);

  // Yes, in newer OCC versions this operation is required; 
  {
    StlAPI_Writer writer;  

    BRepMesh_IncrementalMesh(solid, config->GetStlQualityCoefficient());
    //writer.SetCoefficient(0.03);

    writer.ASCIIMode() = true;
    writer.Write(solid, fname);
  }

  {
    // FIXME: do this better later; in fact this suffices I guess;
    EicCadFileConfig *qconfig = new EicCadFileConfig();
    //qconfig->SetStlVertexMergingTolerance(0.01);
    qconfig->CreateMediaHub(strdup(config->mMaterial.Data()));//(char*)"air");

    EicGeoParData *geo = new EicGeoParData("DUMMY", 0, 0);
    char rname[1024];
    snprintf(rname, 1024-1, "%s.root", bname);
    geo->SetFileName(rname);

    qconfig->mhub()->Init(); 

    // 'false': yes, here I should ignore units and scaling given in mConfig because 
    // they were taken into account when creating the STEP model; THINK!
    EicStlFactory stl_factory(0, fname, qconfig, false);

    stl_factory.CreateRootFile(geo->GetTopVolume(), bname);

    // FIXME;
    geo->GetColorTable()->AddPatternMatch       ("-Ass0", kBlue);
    geo->GetTransparencyTable()->AddPatternMatch("-Ass0", 0);

    geo->FinalizeOutput();

    AccountRootFile(rname);
  }

  unlink(fname);

  return 0;
} // EicCadWizard::CreateStlSolidRootFile()

// ---------------------------------------------------------------------------------------

TGeoCombiTrans *Ax3ToCombiTrans(const char *name, const gp_Ax3 &ax3, double offset)
{
  gp_Pnt x0 = ax3.Location();
  gp_Dir xDir = ax3.XDirection(), yDir = ax3.YDirection(), zDir = ax3.Axis().Direction(); 

  double data[9] = {xDir.X(), yDir.X(), zDir.X(),
		    xDir.Y(), yDir.Y(), zDir.Y(),
		    xDir.Z(), yDir.Z(), zDir.Z()};
  TGeoRotation *grr = new TGeoRotation();
  grr->SetMatrix(data); 

  return new TGeoCombiTrans(name, 
			    x0.X() + offset * zDir.X(), 
			    x0.Y() + offset * zDir.Y(), 
			    x0.Z() + offset * zDir.Z(), grr);
} // Ax3ToCombiTrans()

// ---------------------------------------------------------------------------------------

int EicCadWizard::CreateBooleanSolidRootFile(const TopoDS_Shape &solid, const gp_Pnt *bcenter, double bradius,
					     const std::vector<std::pair<EicCadWizardCut*,bool> > *cuts,
					     const char *bname, EicCadWizardFileConfig *config)
{
  // FIXME: unify this code in both places;
  EicCadFileConfig *qconfig = new EicCadFileConfig();
  //qconfig->CreateMediaHub((char*)"air");
  // FIXME: do it better;
  qconfig->CreateMediaHub(strdup(config->mMaterial.Data()));

  EicGeoParData *geo = new EicGeoParData("DUMMY", 0, 0);
  char rname[1024];
  snprintf(rname, 1024-1, "%s.root", bname);
  geo->SetFileName(rname);
  
  qconfig->mhub()->Init();

  TString cmd(":");

  {
    static unsigned scounter;
    char vname[128], tname[128];

    //printf("counter=%d\n", counter);

    snprintf(vname, 128-1, "VS%04d", scounter);
    snprintf(tname, 128-1, "TS%04d", scounter++);

    cmd = vname + cmd + tname;

    // Define big enough ROOT sphere;
    //TGeoSphere *shell = new TGeoSphere(vname, 0.0, mBoundarySphereRadius + 1E-10*counter);
    //TGeoCombiTrans *ts = new TGeoCombiTrans(tname, mBoundarySphereCenter->X(), 
    //					    mBoundarySphereCenter->Y(), 
    //					    mBoundarySphereCenter->Z(), 0);
    TGeoSphere *shell = new TGeoSphere(vname, 0.0, bradius + 1E-10*scounter);
    TGeoCombiTrans *ts = new TGeoCombiTrans(tname, bcenter->X(), 
					    bcenter->Y(), 
					    bcenter->Z(), 0);
    ts->RegisterYourself();
  }

  for(unsigned ct=0; ct<cuts->size(); ct++) {
    //EicCadWizardCut *cut = (*cuts)[ct].first;
    std::pair<EicCadWizardCut*,bool> cut = (*cuts)[ct];

    // FIXME: multi-threading!!!;
    static unsigned counter;
    char vname[128], tname[128];

    snprintf(vname, 128-1, "V%06d", counter);
    snprintf(tname, 128-1, "T%06d", counter++);

    TGeoCombiTrans *ts = cut.first->BuildRootVolume(vname, tname);

    if (ts) ts->RegisterYourself();
    
    //cmd = "(" + cmd + ")" + (cut->GetCutSolidsCount() ? "-" : "*") + vname + (ts ? TString(":") + tname : "");
    //cmd = "(" + cmd + ")" + (cut->mSubtractionFlag ? "-" : "*") + vname + (ts ? TString(":") + tname : "");
    //cmd = "(" + cmd + ")" + (cut.first->mSubtractionFlag ? "-" : "*") + vname + (ts ? TString(":") + tname : "");
    cmd = "(" + cmd + ")" + (cut.second ? "-" : "*") + vname + (ts ? TString(":") + tname : "");
  } //for ct

  printf("%s\n", cmd.Data()); 

  {
    static unsigned ccounter;
    char cname[128];

    snprintf(cname,  128-1, "@RV@%06d", ccounter++);

    TGeoVolume *vcomp;
    if (mCreateEicCompositeShape) {
      EicCompositeShape *comp = new EicCompositeShape(cname, cmd.Data(), &solid);
      //EicCompositeShape *comp = new EicCompositeShape(cname, "", &solid);
      comp->LocalFillBuffer3D(config->GetStlQualityCoefficient());
      vcomp = new TGeoVolume(cname, comp, qconfig->mhub()->fSingleMedium);
    } else {
      TGeoCompositeShape *comp = new TGeoCompositeShape(cname, cmd.Data());
      vcomp = new TGeoVolume(cname, comp, qconfig->mhub()->fSingleMedium);
    } //if
    //TGeoVolume *vcomp = new TGeoVolume(cname, comp, qconfig->mhub()->fSingleMedium);

    geo->GetTopVolume()->AddNode(vcomp, 0, new TGeoCombiTrans(0, 0, 0, 0));
    vcomp->RegisterYourself();

    // FIXME: please;
    geo->GetColorTable()->AddPatternMatch       ("@RV@", kGray);
    geo->GetTransparencyTable()->AddPatternMatch("@RV@", 0);
    
    geo->FinalizeOutput();

    AccountRootFile(rname);
  }

  return 0;
} // EicCadWizard::CreateBooleanSolidRootFile()

// ---------------------------------------------------------------------------------------

int EicCadWizard::CreateGhostSolidRootFile(const TopoDS_Shape &solid, 
					   const char *bname, EicCadWizardFileConfig *config)
{
  // FIXME: unify this code in both places;
  EicCadFileConfig *qconfig = new EicCadFileConfig();
  // FIXME: this works only as long as material is the same as of CAVE volume;
  qconfig->CreateMediaHub((char*)"air");

  EicGeoParData *geo = new EicGeoParData("GHOST", 0, 0);
  char rname[1024];
  snprintf(rname, 1024-1, "%s.root", bname);
  geo->SetFileName(rname);
  
  qconfig->mhub()->Init();

  {
    static unsigned ccounter;
    char cname[128];

    snprintf(cname,  128-1, "@GV@%06d", ccounter++);

    // FIXME: this is indeed a hack;
    EicCompositeShape *comp = new EicCompositeShape(cname, "", &solid);
    comp->LocalFillBuffer3D(config->GetStlQualityCoefficient());
    TGeoVolume *vcomp = new TGeoVolume(cname, comp, qconfig->mhub()->fSingleMedium);

    geo->GetTopVolume()->AddNode(vcomp, 0, new TGeoCombiTrans(0, 0, 0, 0));
    vcomp->RegisterYourself();

    // FIXME: please;
    //geo->GetColorTable()->AddPatternMatch       ("@GV@", kSpring);
    //geo->GetTransparencyTable()->AddPatternMatch("@GV@", 50);
    geo->GetColorTable()->AddPatternMatch       ("@GV@", kGray);
    geo->GetTransparencyTable()->AddPatternMatch("@GV@", 0);
    
    geo->FinalizeOutput();

    AccountRootFile(rname);
  }

  return 0;
} // EicCadWizard::CreateGhostSolidRootFile()

// ---------------------------------------------------------------------------------------

int EicCadWizard::StraightforwardSplit(const STEPControl_Reader &cReader, const char *bname)
{
  // There are >1 solids in this file; create one level down directory and populate it 
  // with individual shapes;
  char dname[1024];
  unsigned csCounter = 0;
  snprintf(dname, 1024-1, "%s.split", bname);
  // FIXME: once debugging is over, use Boost remove_all() to clean up non-empty directory;
  mkdir(dname, 0755); 
      
  for(unsigned sh=0; sh<cReader.NbShapes(); sh++) {
    TopoDS_Shape shape = cReader.Shape(sh+1);
	
    for (TopExp_Explorer itf(shape,TopAbs_SOLID); itf.More(); itf.Next()) {
      STEPControl_Writer cWriter;
      
      cWriter.Transfer(itf.Current(), STEPControl_ManifoldSolidBrep);
      char qname[1024];
      snprintf(qname, 1024-1, "%s/part-%05d.stp", dname, csCounter);
      cWriter.Write(qname);
      
      // FIXME: and need to deal with these files right away here, recursively; later
      // on - in multi-threading mode - this will not interrupt the processing sequence;
      
      csCounter++;
    } //for itf
  } //for sh

  return 0;
} // EicCadWizard::StraightforwardSplit()

// ---------------------------------------------------------------------------------------

int EicCadWizard::ConvertSourceFileToRoot(EicCadWizardFile *wfile, 
					  const std::vector<std::pair<EicCadWizardCut*,bool> > *tried_cuts)
{
  return ConvertSourceFileToRoot(wfile->GetFileName(), wfile->config(), tried_cuts);
} // EicCadWizard::ConvertSourceFileToRoot()

// =======================================================================================

ClassImp(EicCadWizard)
ClassImp(EicCadWizardFile)
ClassImp(EicCadWizardFileConfig)


#if _OFF_
	  Handle(Geom_Surface) SS = BRep_Tool::Surface(face);
	  if (!IsElementaryFace(face)) {
	    cout << setw(3) << rfCounter << " " << SS->DynamicType() << endl; 
	    unsigned dim = 1000;
	    double u1, u2, v1, v2;
	    SS->Bounds(u1, u2, v1, v2);
	    double du = (u2 - u1)/dim, dv = (v2 - v1)/dim;
	    //printf("%f %f %f %f\n", u1, u2, v1, v2);
	    for(unsigned iu=0; iu<=dim; iu++)
	      for(unsigned iv=0; iv<=dim; iv++) {
		double u = u1 + du*iu, v = v1 + dv*iv;
		gp_Pnt pt = SS->Value(u, v);

		//printf("%3d %3d -> %f %f %f\n", iu, iv, pt.X(), pt.Y(), pt.Z());
	      } //for iu ..iv
#endif

		    //Handle(Message_PrinterOStream) filePrinter = 
		    //new Message_PrinterOStream("export.log", Standard_False, Message_Info);
		    //Handle(Message_Messenger) msgr = Message::DefaultMessenger();
		    //msgr->AddPrinter(filePrinter);
