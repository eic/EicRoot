//
// AYK (ayk@bnl.gov), 2014/03/11; revamped in Oct'2017;
//
//  EicRoot STL/SLP file manipulation routines;
//

#include <assert.h>
#include <iostream>
#include <asm/types.h>

#ifdef TETLIBRARY
#include "tetgen.h"
#endif

#define _BINARY_STL_HEADER_SIZE_ 80

#include <TGeoManager.h>
#include <TGeoMedium.h>
#include <TRotation.h>
#include <TMath.h>
#include <TGeoVolume.h>
#include <TGeoArb8.h>
#include <TGeoMatrix.h>

#include <FairLogger.h>

#include <EicCadFile.h>
#include <EicStlFactory.h>

// =======================================================================================

void EicStlFactory::PreAllocateFacet(unsigned solidId, const TGeoMedium *medium, 
				     const double vCoord1[], const double vCoord2[], 
				     const double vCoord3[])
{
  // Start with vertices rather than facets; 
  EicStlVertex *v1 = new EicStlVertex(vCoord1);
  EicStlVertex *v2 = new EicStlVertex(vCoord2);
  EicStlVertex *v3 = new EicStlVertex(vCoord3);

  // Obtain medium group pointer;
  EicStlMediaGroup *mgroup = &mGroups[std::pair<unsigned, const TGeoMedium*>(solidId, medium)];

  EicStlVertex *vv[3] = {v1, v2, v3};
  for(unsigned iq=0; iq<3; iq++)
  {
    EicStlVertex *vtx = vv[iq];

    // Looks stupid, but allows to unify two code branches;
    if (mgroup->vertices()->find(vtx->key()) == mgroup->vertices()->end()) {
      // Well, vertices can allocate in the ordered map right away; if any of them is not 
      // needed any longer, it will simply be erased rather than modified (like in case of facets);
      //(*mgroup->vertices())[vtx->key()] = vtx;
      mgroup->AddVertex(vtx);//ices())[vtx->key()] = vtx;

      // FIXME: may want to apply a small fixed rotation in order to avoid a fake 1D coordinate
      // degeneracy often found in CAD models (consider a cube with its 8 vertices sitting at 
      // identical X(YZ)-coordinates, etc);
      mgroup->xCoord.insert(std::pair<double, EicStlVertex*>(vtx->key()->GetData()[0], vtx)); 
      mgroup->yCoord.insert(std::pair<double, EicStlVertex*>(vtx->key()->GetData()[1], vtx)); 
      mgroup->zCoord.insert(std::pair<double, EicStlVertex*>(vtx->key()->GetData()[2], vtx)); 
    } else {
      //vv[iq] = (*mgroup->vertices())[vtx->key()];
      vv[iq] = (*mgroup->vertices()).at(vtx->key());
      delete vtx; 
    } //if
  } //for iq

  // Allocate facet and store in the intermediate array; the rest will happen in 
  // the EicStlMediaGroup::SplitIntoAssemblies() call during geometry construction;
  EicStlFacet *facet = new EicStlFacet(vv[0], vv[1], vv[2]);
  mgroup->mFbuffer.push_back(facet);
} // EicStlFactory::PreAllocateFacet()

// ---------------------------------------------------------------------------------------

void EicStlFactory::ImportBinaryStlFile(FILE *fin, unsigned trCount, double scale)
{
  __u16 attr;
  float fnn[3], fvCoord1[3], fvCoord2[3], fvCoord3[3];
	  
  printf("trCount: %d\n", trCount);

  // Assume I'm right past 80-byte header and 4-byte triangle count;
  for(unsigned itr=0; itr<trCount; itr++)
  {
    // Assume these fread() calls can not fail;
    fread(fnn,      sizeof(float), 3, fin);
    fread(fvCoord1, sizeof(float), 3, fin);
    fread(fvCoord2, sizeof(float), 3, fin);
    fread(fvCoord3, sizeof(float), 3, fin);
    fread(&attr,    sizeof(__u16), 1, fin);
    //printf("%7.3f %7.3f %7.3f; %7.3f %7.3f %7.3f; %7.3f %7.3f %7.3f; %7.3f %7.3f %7.3f; %8d\n", 
    //	   fnn[0], fnn[1], fnn[2], 
    //	   fvCoord1[0], fvCoord1[1], fvCoord1[2],
    //	   fvCoord2[0], fvCoord2[1], fvCoord2[2],
    //	   fvCoord3[0], fvCoord3[1], fvCoord3[2], attr); 
    
    // Prefer to switch to double precision right away at the import stage;
    {
      double dnn[3], dvCoord1[3], dvCoord2[3], dvCoord3[3];
      
      for(unsigned iq=0; iq<3; iq++) {
	dnn     [iq] =         double(fnn     [iq]);
	dvCoord1[iq] = scale * double(fvCoord1[iq]);
	dvCoord2[iq] = scale * double(fvCoord2[iq]);
	dvCoord3[iq] = scale * double(fvCoord3[iq]);
      } //for iq

      // Add color attribute to the parameter list later;
      //PreAllocateFacet(0, fMediaHub ? fMediaHub->fSingleMedium : 0, dvCoord1, dvCoord2, dvCoord3);
      PreAllocateFacet(0, (mConfig && mConfig->mhub()) ? mConfig->mhub()->fSingleMedium : 0, 
		       dvCoord1, dvCoord2, dvCoord3);
    }
  } //for itr 
} // EicStlFactory::ImportBinaryStlFile()

// ---------------------------------------------------------------------------------------

void EicStlFactory::ImportAsciiStlSlpFile(double scale)
{
  // Use single medium defined in EicStlFile() constructor call as default; it can be 0 
  // here and then be overriden by STL syntax extention (see below);
  const TGeoMedium *currentMedium = mConfig->mhub()->fSingleMedium;
  unsigned solidId = 0, vtxId = 0;
  Bool_t inSolid = kFALSE, inFacet = kFALSE, inLoop = kFALSE;
  double vCoord[3][3];
  TString buffer;

  std::ifstream is (mGeometryName);
    
  while (!is.eof())
  {
    buffer.ReadLine(is, kTRUE); 

    // Sanity checks included; require exact case match; require no indent for 
    // both "solid" and "nosolid"; this way object names will not hurt even if
    // they contain other keywords; 
    if (buffer.BeginsWith("solid"))
    {
      assert (inSolid == kFALSE && inFacet == kFALSE && inLoop == kFALSE);
      inSolid = kTRUE;
    }  
    else
    if (buffer.BeginsWith("endsolid"))
    {
      assert (inSolid == kTRUE && inFacet == kFALSE && inLoop == kFALSE);
      solidId++;
      inSolid = kFALSE;
    } 
    else
      // Allow user to define media by hand as many times as needed in STL ASCII 
      // file; this key is searched after "solid", so if solid name contains "medium"
      // it will not hurt; also, it is searched before all other keys, so may contain
      // medium name like "siliconvertex" and this also will not hurt; 
    if (mType == AsciiStl && buffer.Contains("medium"))
    {
      // Well, let it be only outside of 'facet - endfacet' clause; 
      assert (inFacet == kFALSE && inLoop == kFALSE);

      // Take it into account only if media name was not given 
      // in EicStlFactory() constructor;
      if (!mConfig->mhub()->fSingleMedium)
      {
	char key[128], medium[128];

	sscanf(buffer.Data(),"%s %s", key, medium);

	currentMedium = mConfig->mhub()->GetMedium(medium);
	assert(currentMedium);
      } //if
    } 
    else
    if (mType == AsciiSlp && buffer.Contains("color"))
    {
      assert (inSolid == kTRUE && inFacet == kFALSE && inLoop == kFALSE);

      if (!mConfig->mhub()->fSingleMedium)
      {
	// Parse string, build color key and try to remap;
	char color[128];
	// Is it really RGB encoded here?; does not matter;
	float rgb[3];
	sscanf(buffer.Data(),"%s %f %f %f", color, rgb + 0, rgb + 1, rgb + 2);
	//+EicStlKey key(3, rgb);

	//printf("%f %f %f\n", rgb[0], rgb[1], rgb[2]);
	assert(0);
#if _TODAY_
	assert(fMediaHub->fMediaMap->find(&key) != fMediaHub->fMediaMap->end());
	currentMedium = (*fMediaHub->fMediaMap)[&key];
#endif
      } //if
    } 
    else
    if ((mType == AsciiStl && buffer.Contains("facet normal")) ||
	(mType == AsciiSlp && buffer.Contains("facet") && !buffer.Contains("endfacet")))
    {
      // Do smart exit later; ignore actual normal data; in case of SLP file 
      // do not pay attention to "normal" lines at all;
      assert (inSolid == kTRUE && inFacet == kFALSE && inLoop == kFALSE);
      inFacet = kTRUE;
    } 
    else
    if (buffer.Contains("endfacet"))
    {
      assert (inSolid == kTRUE && inFacet == kTRUE && inLoop == kFALSE);
      inFacet = kFALSE;
    }
    else
    if (buffer.Contains("outer loop"))
    {
      assert (inSolid == kTRUE && inFacet == kTRUE && inLoop == kFALSE);
      inLoop = kTRUE;
    }
    else
    if (buffer.Contains("endloop"))
    {
      assert (inSolid == kTRUE && inFacet == kTRUE && inLoop == kTRUE);
      inLoop = kFALSE;
      vtxId = 0;
    }
    else
    if (buffer.Contains("vertex"))
    {
      assert (inSolid == kTRUE && inFacet == kTRUE && inLoop == kTRUE && vtxId <= 2);

      char vertex[128];
      sscanf(buffer.Data(),"%s %lf %lf %lf", vertex, 
	     vCoord[vtxId] + 0, vCoord[vtxId] + 1, vCoord[vtxId] + 2);
      
      // The essential part;
      if (vtxId == 2) 
      {
	//printf("%7.3f %7.3f %7.3f; %7.3f %7.3f %7.3f; %7.3f %7.3f %7.3f\n", 
	//     vCoord[0][0], vCoord[0][1], vCoord[0][2],
	//     vCoord[1][0], vCoord[1][1], vCoord[1][2],
	//     vCoord[2][0], vCoord[2][1], vCoord[2][2]);

	// Well, by this point medium should be defined, in whatever way;
	assert(currentMedium);
	//printf("%s\n", currentMedium->GetName());
	for(unsigned ip=0; ip<3; ip++) 
	  for(unsigned iq=0; iq<3; iq++) 
	    vCoord[ip][iq] *= scale;
	PreAllocateFacet(solidId, currentMedium, vCoord[0], vCoord[1], vCoord[2]);
      } //if      

      vtxId++;
    } //if
  } // for inf
  
  is.close();
} // EicStlFactory::ImportAsciiStlSlpFile()

// ---------------------------------------------------------------------------------------

EicStlFactory::EicStlFactory(const char *vname, const char *geometryName, const EicCadFileConfig *config, 
			     bool acknowledge_config_file_scaling): //mWireframeMode(false), mWireframeFeatureSize(0.0),
  mGeometryName(geometryName), mVolumeName(vname), mConfig(config)
{
  double scale = (config && acknowledge_config_file_scaling) ? config->scale() * config->units() : 1.0;

  mLogger = FairLogger::GetLogger();

  // If file extension is ".slp", it is Pro-E SLP format, no tricks; let it be the default;
  mType = EicStlFactory::AsciiSlp;

  // Well, I need to handle at least 3 different formats (ASCII and binary STL, 
  // as well as ASCII SLP); makes sense to unify these 3 branches early enough;
  // however vertex & facet allocation is needed anyway; consider to put this
  // allocation in a separate call and do an easy switch of input parser
  // depending on the file type;
  FILE *fin = fopen(geometryName, "r");
  if (!fin) 
    mLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Failed to open geometry file %s!!  \033[0m", 
		   geometryName);

  // Used in binary STL case only?;
  __u32 trCount;
  if (mGeometryName.EndsWith(".stl")) {
    // Pro-E binary STL files happily start with the same "solid" keyword as ASCII 
    // files (idiots!), so one can not check on that; assume, that if number of encoded 
    // triangles matches file size, it is a binary file; so first calculate file size in the 
    // most stupid way possible;
    fseek(fin, 0, SEEK_END);
    long fSize = ftell(fin);
      
    rewind(fin);
    unsigned char header[_BINARY_STL_HEADER_SIZE_];
    if (fread(header, 1, _BINARY_STL_HEADER_SIZE_, fin) == _BINARY_STL_HEADER_SIZE_ &&
	// Well, in fact I guess I know size of 32-bit integer, or?; 
	fread(&trCount, 1, sizeof(__u32), fin) == sizeof(__u32) &&
	// Calculate expected file size if it is binary;
	fSize == _BINARY_STL_HEADER_SIZE_ + sizeof(__u32) + trCount*(sizeof(float)*3*4 + sizeof(__u16)))
      mType = EicStlFactory::BinaryStl;
    else
      mType = EicStlFactory::AsciiStl;
  } // if (mGeometryName.EndsWith(".stl"))

  // And now arrange an import switch;
  switch (mType) {
  case EicStlFactory::BinaryStl:
    // Yes, there is no other easy way to encode media in Pro/E STL files; other packages
    // may allow this (either in the header or in attribute field); once needed, extend this part;
    if (mConfig && mConfig->mhub()) assert(mConfig->mhub()->fSingleMedium);
    ImportBinaryStlFile(fin, trCount, scale);
    fclose(fin);
    break;
  case EicStlFactory::AsciiStl:;
  case EicStlFactory::AsciiSlp:
    fclose(fin);
    ImportAsciiStlSlpFile(scale);
    break;
  default:
    assert(0);
  } // switch(mType)
} // EicStlFactory::EicStlFactory()

// =======================================================================================

#ifdef TETLIBRARY
static TVector3 GetTetPoint(tetgenio &io, unsigned index_offset)
{
  // FIXME: think about units;
  return TVector3( io.pointlist[(io.tetrahedronlist[index_offset]-1)*3+0],
		   io.pointlist[(io.tetrahedronlist[index_offset]-1)*3+1],
		   io.pointlist[(io.tetrahedronlist[index_offset]-1)*3+2]);
} // GetTetPoint()
#endif

// ---------------------------------------------------------------------------------------

void EicStlFactory::DumpTmpStlFile(EicStlAssembly *assembly, TString &stlActualFileName)
{
  // Do this better later (Mac OS compatible at least);
  FILE *fout = fopen(stlActualFileName, "w"); 
  if (!fout)
    mLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Failed to open file %s!!  \033[0m", 
		   stlActualFileName.Data());
    
  // Dump "solid ..." clause;
  fprintf(fout, "solid MySolid\n");

  for(unsigned fc=0; fc<assembly->facets().size(); fc++) {
    EicStlFacet *facet = assembly->facets()[fc].first;
    double sign = assembly->facets()[fc].second ? 1.0 : -1.0;
    
    // Dump respective facet section; assume %16.8f format suffices for all
    // practical situations;
    fprintf(fout, "  facet normal %16.8f %16.8f %16.8f\n", 
	    sign*facet->GetNormal()[0], sign*facet->GetNormal()[1], sign*facet->GetNormal()[2]);
    fprintf(fout, "    outer loop\n");
    {
      // FIXME: does vertex order matter here (it is indeed arbitrary) or normal direction 
      // sufiices to instruct Tetgen (netgen) software about outward-pointing direction?;
      for(vEntry::const_iterator vt=facet->vertices()->begin(); vt!=facet->vertices()->end() ; vt++) {
	const double *coord = vt->second->key()->GetData();
	fprintf(fout, "      vertex %16.8f %16.8f %16.8f\n", coord[0], coord[1], coord[2]);
      } //for vt
      fprintf(fout, "    endloop\n");
      fprintf(fout, "  endfacet\n");
    }
  } //for fc
      
  // Dump "endsolid ..." clause;
  fprintf(fout, "endsolid MySolid\n");
  fclose(fout);
} // EicStlFactory::DumpTmpStlFile()

// ---------------------------------------------------------------------------------------

TGeoVolume *CreateTetrahedron(TVector3 &p1, TVector3 &p2, TVector3 &p3, TVector3 &p4, 
		       char name[], TGeoVolume *fMother, const TGeoMedium *medium, TVector3 shift)
{
  // And now the whole mess comes; no G4Tet or similar in ROOT geometry primitives, 
  // so have to calculate TGeoArb8 parameters, including rotation matrix;
  // Assume, that {p1, p2, p3, p3} are base at -DZ and {p4,p4,p4,p4} is an apex at +DZ;
	  
  // Figure out translation; start with normal to the triangular base;
  TVector3 p21 = p2 - p1, p31 = p3 - p1, nn = p31.Cross(p21).Unit();
  // Base points should be in clockwise direction in TGeoArb8 => need to check 
  // the below scalar product sign (should be positive) and if not, swap p2 & p3;
  TVector3 p41 = p4 - p1;
  double projection = p41 * nn;
  //printf("pro: %f\n", projection);
  if (projection < 0)
  {
    TVector3 bff(p2);
    p2 = p3;
    p3 = bff;
	    
    p21 = p2 - p1;
    p31 = p3 - p1;
    
    nn = -1. * nn;
  } //if
  projection = fabs(projection);
  TVector3 p41perp = projection * nn, translation = p4 - 0.5 * p41perp;
	  
  // Then calculate rotation matrix;
  TVector3 zz(0., 0., 1.);
  double angle =  nn.Angle(zz);
  TVector3 axis = (nn.Cross(zz)).Unit();
	  
  TRotation rr, qrr; rr.SetToIdentity(); 
  // Requires a bit of massaging;
  if (!angle)
    qrr.SetToIdentity();
  else 
  if (angle == TMath::Pi())
    // Or rr.RotateX(pi), does not matter;
    qrr = rr.RotateY(TMath::Pi());
  else
    qrr = rr.Rotate(angle, axis);
  TVector3 q1, q2, q3, q4; 
  q1 = qrr * (p1 - translation);
  q2 = qrr * (p2 - translation);
  q3 = qrr * (p3 - translation);
  q4 = qrr * (p4 - translation);
  //q1.Print(); q2.Print(); q3.Print(); q4.Print();  cout << endl; //exit(0);
  
  double vtx[8][2] = {
    {q1.x(), q1.y()},
    {q2.x(), q2.y()},
    {q3.x(), q3.y()},
    {q3.x(), q3.y()},
    {q4.x(), q4.y()},
    {q4.x(), q4.y()},
    {q4.x(), q4.y()},
    {q4.x(), q4.y()}};

  // Is there an easier way to do this?;
  qrr.Invert();
  double data[9] = {qrr.XX(), qrr.XY(), qrr.XZ(),
		    qrr.YX(), qrr.YY(), qrr.YZ(),
		    qrr.ZX(), qrr.ZY(), qrr.ZZ()};

  TGeoRotation *grr = new TGeoRotation();
  grr->SetMatrix(data); 
  
  TGeoArb8 *tet    = new TGeoArb8  (name, projection/2., (double*)vtx);
  TGeoVolume *vtet = new TGeoVolume(name, tet, medium);
  fMother->AddNode(vtet, 0, new TGeoCombiTrans(translation.x() + shift.x(), 
					       translation.y() + shift.y(), translation.z() + shift.z(), grr));
  //vtet->SetFillColor(kBlue);
  //printf("Here!\n");
  // THINK: really needed?;
  //grr->RegisterYourself();
  vtet->RegisterYourself();

  return vtet;
} // CreateTetrahedron()

// ---------------------------------------------------------------------------------------

static TVector3 swapXYfun(TVector3 src)
{
  TVector3 ret;
  ret.SetX(-src.z()); 
  ret.SetY( src.y()); 
  ret.SetZ( src.x()); 

  return ret;
} // swapXYfun()

//#include <TGeoSphere.h>
//#include <TGeoCompositeShape.h>

void EicStlFactory::_ConstructGeometry(TGeoVolume *mother, TVector3 shift, bool swapXY, 
				       /*bool wireframe,*/ std::vector<TGeoVolume*> *volumes)
{
  for (gEntry::iterator it=mGroups.begin(); it!=mGroups.end(); it++) {
    EicStlMediaGroup *mgroup = &it->second;

    printf("vtx count: %d; buffered facet count: %d\n",
    	   mgroup->vertices()->size(), mgroup->mFbuffer.size()); 

#if _DOES_NOT_WORK_
    // May want to build just a wireframe model for vizualization purposes;
    //if (mWireframeMode) {
    if (wireframe) {
      double fsize = 5.0 * eic::mm;
      static unsigned counter;

      //char sname[128];//, tname[128];
      //snprintf(sname, 128-1, "Pt%05d", counter);
      //snprintf(tname, 128-1, "Tr%05d", counter);
      TGeoSphere *sphere = new TGeoSphere("SPHERE", 0.0, fsize/2);
      TGeoVolume *vsphere = new TGeoVolume("SPHERE", sphere, it->first.second);
      vsphere->RegisterYourself();
      //TGeoCombiTrans *trans = new TGeoCombiTrans(tname, src[0], src[1], src[2], 0);
      //mother->AddNode(vsphere, 0, new TGeoCombiTrans(src[0], src[1], src[2], 0));

      char cmd[16384] = "";
      for (vEntry::const_iterator vt=mgroup->vertices()->begin(); vt != mgroup->vertices()->end(); vt++) {
	EicStlVertex *vtx = vt->second;
	//vtx->Print();

	TVector3 src(vtx->key()->GetData());
	if (swapXY) src = swapXYfun(src);

	//char sname[128], tname[128];
	//snprintf(sname, 128-1, "Pt%05d", counter);
	//snprintf(tname, 128-1, "Tr%05d", counter);
	//TGeoSphere *sphere = new TGeoSphere(sname, 0.0, fsize/2);
	//TGeoVolume *vsphere = new TGeoVolume(sname, sphere, it->first.second);
	//TGeoCombiTrans *trans = new TGeoCombiTrans(tname, src[0], src[1], src[2], 0);
	mother->AddNode(vsphere, counter, new TGeoCombiTrans(src[0], src[1], src[2], 0));
	//trans->RegisterYourself();
	//vsphere->RegisterYourself();

	//unsigned len = strlen(cmd);
	//if (counter) sprintf(cmd + strlen(cmd), "+");
	//sprintf(cmd + strlen(cmd), "%s:%s", sname, tname);
	//char cmd[1024], yokeCompName[128];
	//snprintf(cmd, 1024-1, "%s-%s", yokeIronName, yokeVacuumName);
	//snprintf(yokeCompName, 128-1, "%sComp", GetDetectorName().Data());

	//TGeoCompositeShape *comp = new TGeoCompositeShape(yokeCompName/*GetDetectorName().Data()*/, cmd);
	//mYoke = new TGeoVolume(yokeCompName/*GetDetectorName().Data()*/, comp, mediaHub->GetMedium(_IRON_));

	//break;

	counter++;
      } //for vt

      //printf("%s\n", cmd);
      //TGeoCompositeShape *comp = new TGeoCompositeShape("SPHERE", cmd);
      //TGeoVolume *vcomp = new TGeoVolume("SPHERE", comp, it->first.second);
      //mother->AddNode(vcomp, 0, new TGeoCombiTrans(0, 0, 0, 0));//src[0], src[1], src[2], 0));
      //vcomp->RegisterYourself();

      continue;
    } //if
#endif

    if (mgroup->SplitIntoAssemblies(mConfig->GetStlVertexMergingTolerance()))
      mLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m EicStlFactory::ConstructGeometry() "
		     "failed on %s!!  \033[0m", mGeometryName.Data());

    // Assume, that by this point all the remaining issues are resolved; just loop through 
    // assemblies and their facets, dump respective ASCII files, call TETGEN and ROOT Geo Manager;
    for(unsigned ass=0; ass<mgroup->assemblies().size(); ass++)
    {
      EicStlAssembly *assembly = mgroup->assemblies()[ass];

      // FIXME: should put medium name here as well;
      char buffer[128];
      snprintf(buffer, 128-1, "./%05d-%06d-%s.stl", getpid(), ass, it->first.second->GetName());
      TString stlActualFileName = TString(buffer);

      DumpTmpStlFile(assembly, stlActualFileName);

      // NB: yes, comment out like this since eventually may want to add complementary netgen 
      // support as well (so that the routine itself can still be a valid piece of code);
#ifdef TETLIBRARY
      {
	tetgenio in, out;
	unsigned ret = in.load_stl((char*)stlActualFileName.Data());
	std::cout << in.numberofpoints     << " points" << std::endl; 

	//printf("Here-1!\n");
	// The essential part - convertion to tetrahedra; "Y" key is VERY important here 
	// for a multi-piece object!; how about checking return code?;
	tetrahedralize((char*)"YpQ", &in, &out);
	//tetrahedralize((char*)"Yp", &in, &out);
	//printf("Here-2!\n");
	std::cout << out.numberofpoints     << " points" << std::endl; 
	std::cout << mVolumeName.Data() << " @@@ " << out.numberoftetrahedra << " thetrahedra" << std::endl;     
	
	for (int iq=0; iq<out.numberoftetrahedra; iq++)
	{
	  char name[128];
	  //snprintf(name, 128-1, "%s-Ass%04d-Tet%06d-%s", mVolumeName.Data(), ass, iq, 
	  //	   it->first.second->GetName());
	  snprintf(name, 128-1, "@RV@%s-Ass%04d-Tet%06d-%s", mVolumeName.Data(), ass, iq, 
		   it->first.second->GetName());
	  
	  // Tetrahedron has 4 corners, right?;
	  int index_offset = iq * 4; 

	  // Use p1..p4 instead of array p[4] for a better readability;
	  TVector3 p1 = GetTetPoint(out, index_offset    );
	  TVector3 p2 = GetTetPoint(out, index_offset + 1);
	  TVector3 p3 = GetTetPoint(out, index_offset + 2);
	  TVector3 p4 = GetTetPoint(out, index_offset + 3);
	  //p1.Print(); p2.Print(); p3.Print(); p4.Print(); cout << endl; //exit(0);

	  //createTetrahedron(p1, p2, p3, p4, name, fSingleMedium);
	  //+CreateTetrahedron(p1, p2, p3, p4, name, mother, it->first.second, shift);
	  {
	    TGeoVolume *vtet;

	    if (swapXY) {
	      TVector3 swpp1 = swapXYfun(p1), swpp2 = swapXYfun(p2), swpp3 = swapXYfun(p3), swpp4 = swapXYfun(p4);

	      vtet = CreateTetrahedron(swpp1, swpp2, swpp3, swpp4, name, mother, it->first.second, shift);
	    } else
	      vtet = CreateTetrahedron(p1, p2, p3, p4, name, mother, it->first.second, shift);

	    if (volumes) volumes->push_back(vtet);
	    //printf("Here!\n");
	    //vtet->SetFillColor(kBlue);
	    //vtet->RegisterYourself();
	  }
	}
      }
#else
      mLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m STL file meshing required but Tetgen "
		     "support is not compiled in! \033[0m"); 
      
#endif
      //unlink(stlActualFileName.Data());
    } //for ass
  } //for it
} // EicStlFactory::ConstructGeometry()

// ---------------------------------------------------------------------------------------

int EicStlFactory::CreateRootFile(TGeoVolume *mother, const char *bname)
{
  for (gEntry::iterator it=mGroups.begin(); it!=mGroups.end(); it++) {
    EicStlMediaGroup *mgroup = &it->second;

    printf("vtx count: %d; buffered facet count: %d\n",
    	   mgroup->vertices()->size(), mgroup->mFbuffer.size()); 

    if (mgroup->SplitIntoAssemblies(mConfig->GetStlVertexMergingTolerance())) assert(0);

    // Assume, that by this point all the remaining issues are resolved; just loop through 
    // assemblies and their facets, dump respective ASCII files, call TETGEN and ROOT Geo Manager;
    for(unsigned ass=0; ass<mgroup->assemblies().size(); ass++) {
      EicStlAssembly *assembly = mgroup->assemblies()[ass];

      // FIXME: should put medium name here as well;
      char buffer[128];
      snprintf(buffer, 128-1, "./%05d-%06d-%s.stl", getpid(), ass, it->first.second->GetName());
      TString stlActualFileName = TString(buffer);

      DumpTmpStlFile(assembly, stlActualFileName);

      // NB: yes, comment out like this since eventually may want to add complementary netgen 
      // support as well (so that the routine itself can still be a valid piece of code);
#ifdef TETLIBRARY
      {
	tetgenio in, out;
	unsigned ret = in.load_stl((char*)stlActualFileName.Data());
	std::cout << in.numberofpoints     << " points" << std::endl; 

	// The essential part - convertion to tetrahedra; "Y" key is VERY important here 
	// for a multi-piece object!; how about checking return code?;
	//+tetrahedralize((char*)"YpQ", &in, &out);
	tetrahedralize((char*)"Yp", &in, &out);
	std::cout << out.numberofpoints     << " points" << std::endl; 
	std::cout << mVolumeName.Data() << " @@@ " << out.numberoftetrahedra << " thetrahedra" << std::endl;     
	
	for (int iq=0; iq<out.numberoftetrahedra; iq++)	{
	  char name[128];
	  snprintf(name, 128-1, "@RV@%s-Ass%04d-Tet%06d-%s", mVolumeName.Data(), ass, iq, 
		   it->first.second->GetName());
	  
	  // Tetrahedron has 4 corners, right?;
	  int index_offset = iq * 4; 

	  // Use p1..p4 instead of array p[4] for a better readability;
	  TVector3 p1 = GetTetPoint(out, index_offset    );
	  TVector3 p2 = GetTetPoint(out, index_offset + 1);
	  TVector3 p3 = GetTetPoint(out, index_offset + 2);
	  TVector3 p4 = GetTetPoint(out, index_offset + 3);

	  //CreateTetrahedron(p1, p2, p3, p4, name, cave, it->first.second, TVector3());
	  CreateTetrahedron(p1, p2, p3, p4, name, mother, mConfig->mhub()->fSingleMedium, TVector3());
	}
      }
#else
      printf("STL file meshing required but Tetgen support is not compiled in!\n");
      assert(0);
#endif

      // No need in these intermediate files;
      unlink(stlActualFileName.Data());
    } //for ass
  } //for it

  return 0;
} // EicStlFactory::CreateRootFile()

// =======================================================================================

ClassImp(EicStlFactory)
