//
// AYK (ayk@bnl.gov), 2014/08/29
//
//  EIC magnetic field map handler;
//

#include <stdlib.h>
#include <assert.h>
#include <dirent.h>      
#include <unistd.h>

#include <TFile.h>

#include <FairRunSim.h>

#include <EicFieldMapDetector.h>
#include <EicLibrary.h>
#include <EicMagneticField.h>

// =======================================================================================

EicMagneticField::EicMagneticField(const char *fileName) 
{
  // If file name is given, import it as a whole; in principle may want to add more
  // fields to existing structure; should work I guess;
  if (fileName) {
    TFile fin(ExpandedFileName(fileName));

    if (!fin.IsOpen()) 
      fLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Failed to open '%s' field!  \033[0m", 
		     fileName);

    EicMagneticField *fptr = 0;
    fin.GetObject(_EIC_MAGNETIC_FIELD_, fptr);
    fin.Close();

    if (!fptr)
      fLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Failed to import '%s' field!  \033[0m", 
		     fileName);

    *this = *fptr;
  } //if

  // Is it really needed?; just set whatever index >2;
  fType = 3;

  mInitialized = false;
} // EicMagneticField::EicMagneticField()

// ---------------------------------------------------------------------------------------

int EicMagneticField::AddBeamLineElementMaps(const char *directory, float fieldScaler, int color)
{         
  TString dirName = ExpandedFileName("input/", directory);

  DIR *curr_dir = opendir(dirName.Data());
  if (!curr_dir)
    fLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Directory '%s' does not exist!  \033[0m", 
		   dirName.Data());

  // Loop through all ".csv" file in this directory and add them one by one;
  {
    struct dirent *curr_file;
    int extention_len = strlen(_CSV_EXTENSION_);

    while((curr_file = readdir(curr_dir))) {
      int len = strlen(curr_file->d_name);    

        if (len >= extention_len && 
            !memcmp(curr_file->d_name + len - extention_len, 
                    _CSV_EXTENSION_, extention_len))
        {
	  TString fileName = TString(directory) + "/" + curr_file->d_name;

	  printf("Adding beam line element map '%s'\n", fileName.Data());

	  // No transformation and no shape (will be taken from the source file);
	  EicBeamLineElementMap *map = new EicBeamLineElementMap(fileName.Data());
	  map->SetYokeColor(color);
	  map->SetFieldScale(fieldScaler);
	  AddFieldMap(map);
        } /*if*/
    } //while
  }

  return 0;
} // EicMagneticField::AddBeamLineElementMaps()

// ---------------------------------------------------------------------------------------

#if 1
int EicMagneticField::AddBeamLineElementGrads(const char *directory, float fieldScaler, int color)
{         
  TString dirName = ExpandedFileName("input/", directory);
  std::cout << "dirName = " << dirName << std::endl;

  DIR *curr_dir = opendir(dirName.Data());
  if (!curr_dir)
    fLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Directory '%s' does not exist!  \033[0m", 
		   dirName.Data());

  // Loop through all ".dat" file in this directory and add their magnetic 
  // field elements (dipoles & quads with constant field or gradient) line by line;
  {
    struct dirent *curr_file;
    int extention_len = strlen(".dat");

    while((curr_file = readdir(curr_dir))) {
      int len = strlen(curr_file->d_name);    

      if (len >= extention_len && 
	  !memcmp(curr_file->d_name + len - extention_len, 
		  ".dat", extention_len)) {
	TString fileName = dirName + "/" + curr_file->d_name;
	
	std::cout << "Getting ready to read in the text file with the field information" << std::endl;
	  
	FILE *fin = fopen(fileName.Data(), "r");
	if (!fin) {
	  printf("-E- EicBeamLineElementGrad::Initialize() -> fail to open '%s' file!\n", fileName.Data());
	  return -1;
	} //if

	char buffer[1024];
	while (fgets(buffer, 1024, fin)) {
	  char name[256];
	  double centerX, centerY, centerZ, rZin, rZout, dOut, length, angle, b, gradient;

	  if (!strlen(buffer) || buffer[0] == '#') continue;

	  // NB: as of Apr'2018 changed coordinate order to XYZ here (same as in extractor.cc 
	  // used to produce these files out of madx ones);
	  int ret = sscanf(buffer, "%s %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %s %lf", 
			   name, &centerX, &centerY, &centerZ, &rZin, &rZout, &dOut, &length, &angle, &b, &gradient);
	  if (ret != 11) continue;

	  printf("%10s -> %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f\n", 
		 name, centerX, centerY, centerZ, rZin, rZout, dOut, length, angle, b, gradient);
		
	  // No transformation and no shape (will be taken from the source file);
	  EicBeamLineElementGrad *map = new EicBeamLineElementGrad(name, centerX, centerY, centerZ, rZin, rZout, dOut, length, angle, b, gradient);
	  map->SetYokeColor(color);
	  map->SetFieldScale(fieldScaler);
	  AddFieldMap(map);
	} //while

	fclose(fin);
      } //if
    } //while
  }

  return 0;
} // EicMagneticField::AddBeamLineElementGrads()
#else
int EicMagneticField::AddBeamLineElementGrads(const char *directory, float fieldScaler, int color)
{         
  TString dirName = ExpandedFileName("input/", directory);
  std::cout << "dirName = " << dirName << std::endl;

  DIR *curr_dir = opendir(dirName.Data());
  if (!curr_dir)
    fLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Directory '%s' does not exist!  \033[0m", 
		   dirName.Data());

  // Loop through all ".csv" file in this directory and add them one by one;
  {
    struct dirent *curr_file;
    int extention_len = strlen(".dat");

    while((curr_file = readdir(curr_dir))) {
      int len = strlen(curr_file->d_name);    

        if (len >= extention_len && 
            !memcmp(curr_file->d_name + len - extention_len, 
                    ".dat", extention_len))
        {
	  //TString fileName = TString(directory) + "/" + curr_file->d_name;
	  TString fileName = dirName + "/" + curr_file->d_name;

	  // Figure out the actual file name;
	  //TString fileName = ExpandedFileName("input/", GetFileName());

	  std::cout << "Getting ready to read in the text file with the field information" << std::endl;
	  
	  //FILE *fin = fopen(fileName.Data(), "r");
	  std::ifstream fin;
	  fin.open(fileName.Data());
	  if (!fin) {
	    printf("-E- EicBeamLineElementGrad::Initialize() -> fail to open '%s' file!\n", fileName.Data());
	    return -1;
	  } //if
	  
	  std::cout << "its open..." << std::endl;
	  
	  char buffer[1024];
	  char mName[256];
	  double mCenterX, mCenterY, mCenterZ, mRadius, mLength, mAngle, mB, mGradient;
	  
	  //fgets(buffer, 1024-1, fin);
	  // read in the header comments and discard it
	  for(int i=0; i<6; i++)
	    {
	      fin.getline(buffer, 1024);
	    }
	  
	  // format for files
	  //-------------------------------
	  // element name (header to be ingnored)
	  // magnet_name center_x[m] center_y[m] center_z[m] aperture_radius[m] length[m] angle[mrad] B[T] gradient[T/m]

	  std::cout << "extract the information in the file" << std::endl;
	  while(1)
	    {
	      if(fin >> mName >> mCenterZ >> mCenterX >> mCenterY >> mRadius >> mLength >> mAngle >> mB >> mGradient)
		{
	      
		  printf("Adding beam line element gradient '%s'\n", mName);
		  printf("%15.10f %15.10f %15.10f %15.10f %15.10f %15.10f %15.10f %15.10f\n", 
			 mCenterX, mCenterY, mCenterZ, mRadius, mLength, mAngle, mB, mGradient);
		  
		  // No transformation and no shape (will be taken from the source file);
		  EicBeamLineElementGrad *map = new EicBeamLineElementGrad(mName, mCenterX, mCenterY, mCenterZ, mRadius, mLength, mAngle, mB, mGradient);
		  map->SetYokeColor(color);
		  map->SetFieldScale(fieldScaler);
		  //AddFieldGradient(map);
		  AddFieldMap(map);
		}  // if
	      else{
		break;
	      }
	    }  // while(1)
	  fin.close();
        } /*if*/
    } //while
  }

  return 0;
} // EicMagneticField::AddBeamLineElementGrads()
#endif

// ---------------------------------------------------------------------------------------

int EicMagneticField::CreateYokeVolumes(Bool_t Active)
{
  FairRunSim *fRun = FairRunSim::Instance();

  // Loop through all the maps and call detector construction routines if available; at present only 
  // beam line elements can do (and perhaps even there this functionality is rather artificial);
  for(unsigned mm=0; mm<mMaps.size(); mm++) {
    EicMagneticFieldMap *fmap = mMaps[mm];

    if (fmap->CapableToBuildYoke() && 
	mSuppressedYokes.find(fmap->GetDetectorName().Data()) == mSuppressedYokes.end()) {
      //printf("here: %s\n", fmap->GetDetectorName().Data()); exit(0);

      //printf("%s\n", fmap->GetDetectorName().Data());

      fRun->AddModule(new EicFieldMapDetector(fmap, Active));
    } //if
  } //for mm

  return 0;
} // EicMagneticField::CreateYokeVolumes()

// ---------------------------------------------------------------------------------------

int EicMagneticField::InitializeFieldMaps()
{
  for(unsigned mm=0; mm<mMaps.size(); mm++) {
    EicMagneticFieldMap *fmap = mMaps[mm];

    // Map was initialized already -> skip here;
    if (fmap->Initialized()) continue;

    // Well, this call is used in FairRoot::Init() implementation which has no return code;
    // so if anything goes wrong here, just exit with a clear red message;
    if (fmap->Initialize()) 
      fLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Failed to initialize '%s' field map!  \033[0m", 
		     fmap->GetFileName().IsNull() ? "NO NAME" : fmap->GetFileName().Data());
  } //for mm

  mInitialized = true;

  // FIXME: arrange a queue or a separate task for such purposes;
  {
    FairRun *fRun = FairRun::Instance();

    // I hope there is no need to save/restore current directory here?;
    fRun->GetOutputFile()->cd();

    Write(_EIC_MAGNETIC_FIELD_);
  }

  return 0;
} // EicMagneticField::InitializeFieldMaps()

// ---------------------------------------------------------------------------------------

int EicMagneticField::GetFieldSumValue(const double xx[], double B[]) 
{
  //printf("%f %f %f\n", xx[0], xx[1], xx[2]);
  // Reset field to 0.0 for clarity;
  for(unsigned iq=0; iq<3; iq++)
    B[iq] = 0.0;

  if (!mInitialized && InitializeFieldMaps()) return -1;
  
  // FIXME: need an error printout here I guess;
  if (!xx || !B || !mMaps.size()) return -1;
  
  // Failure per default;
  int ret = -1;

  // For now a trivial implementation: just a sum of all maps which can provide
  // field at this location xx[];
  for(unsigned mm=0; mm<mMaps.size(); mm++) {
    EicMagneticFieldMap *fmap = mMaps[mm];
    
    double BMap[3];
    
    if (!fmap->GetFieldValue(xx, BMap)) {
      for(unsigned iq=0; iq<3; iq++) 
	B[iq] += BMap[iq];
      
      ret = 0;
    } //if
  } //for mm
  
  return ret;
} // EicMagneticField::GetFieldSumValue()

// -----------------------------------------------------------------------------------------------

int EicMagneticField::Export(const char *fileName) const
{
  //if (!mInitialied) return -1;

  // Yes, export always happens precisely to the path given via 'fileName' (no VMCWORKDIR
  // expansion like in importTpcDigiParameters());
  TFile fout(fileName, "RECREATE");

  if (!fout.IsOpen())
  {    
    printf("-E- EicMagneticField::Export() -> failed to open '%s' for writing!\n", fileName);
    return -1;
  } //if

  fout.WriteObject(this, _EIC_MAGNETIC_FIELD_);
  fout.Close();

  return 0;
} // EicMagneticField::Export()

// =======================================================================================

ClassImp(EicPndFieldMap)
ClassImp(EicMagneticField)
