//
// AYK (ayk@bnl.gov), 2014/03/11; revamped in Oct'2017;
//
//  EicRoot STL/SLP file manipulation routines; the idea behind keeping this 
//  class separate from EicCadFile is that local istances are invoked in a
//  couple of places in order to handle triangulation of geometry subcomponents;
//  therefore is it not really practical to merge the two classes inot one; on
//  the other hand I definitely want to see only EicCadFile calls in simulation.C
//  rather than diverse group of classes handling each particular type of input;  
//

#include <TObject.h>
#include <TString.h>

#include <FairLogger.h>

#include <EicMediaHub.h>
#include <EicStlMediaGroup.h>

#ifndef _EIC_STL_FACTORY_
#define _EIC_STL_FACTORY_

class TGeoVolume;
class EicCadFileConfig;

class EicStlFactory: public TObject {
 public:
  EicStlFactory(const char *volume_name, const char *geometry_file_name, const EicCadFileConfig *config, 
		bool acknowledge_config_file_scaling = true);
  // Dummy constructor;
 EicStlFactory(): mType(Undefined), /*mWireframeMode(false), mWireframeFeatureSize(0.0),*/ mConfig(NULL), mLogger(NULL) {};
  // Destructor;
  ~EicStlFactory() {};

  /*! Either binary or ASCII STL, as well as ASCII SLP files are allowed as input; 
   STL/SLP is decided based on file extension, binary/ASCII based on file length */
  enum StlFileType {Undefined, BinaryStl, AsciiStl, AsciiSlp};

  gEntry &mgroups( void ) { return mGroups; }; 

  void _ConstructGeometry(TGeoVolume *mother, TVector3 shift, bool swapXY = false, //bool wireframe = false, 
			 std::vector<TGeoVolume*> *volumes = 0);
  int CreateRootFile(TGeoVolume *mother, const char *bname);

 private:
  FairLogger *mLogger;

  /*! Input STL/SLP file type */
  StlFileType mType;

  //bool mWireframeMode;
  //double mWireframeFeatureSize;

  const EicCadFileConfig *mConfig;

  /*! Ordered map of medium-separated objects */
  gEntry mGroups; //!

  TString mVolumeName;
  TString mGeometryName;

  void ImportBinaryStlFile(FILE *fin, unsigned trCount, double scale = 1.0);
  void ImportAsciiStlSlpFile(double scale = 1.0);
  
  /// Facet allocation call during STL file input stage
  ///
  /// @param solidId  0-based index of "solid - endsolid" block in the ASCII STL/SLP files; 
  ///        0 for binary STL
  /// @param medium   medium pointer
  /// @param vCoord1  1-st vertex coordinates
  /// @param vCoord2  2-d vertex coordinates
  /// @param vCoord3  3-d vertex coordinates 
  void PreAllocateFacet(unsigned solidId, const TGeoMedium *medium, 
			const double vCoord1[], const double vCoord2[], const double vCoord3[]);

  void DumpTmpStlFile(EicStlAssembly *assembly, TString &stlActualFileName);

  ClassDef(EicStlFactory,4) 
};

// Yes, this routine can happily live outside of any class definition (used 
// independently in EicCadFile and EicStlFactory);
TGeoVolume *CreateTetrahedron(TVector3 &p1, TVector3 &p2, TVector3 &p3, TVector3 &p4, 
			      char name[], TGeoVolume *mother, const TGeoMedium *medium, TVector3 shift);

#endif
