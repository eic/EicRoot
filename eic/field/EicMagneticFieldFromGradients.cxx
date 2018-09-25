//
// RMP (rpetti@bnl.gov), 2016/02/22
//
//  EIC magnetic field map handler;
//   imports from gradients
//

#include <stdlib.h>
#include <assert.h>
#include <dirent.h>      
#include <unistd.h>

#include <TFile.h>

#include <FairRunSim.h>

#include <EicFieldGradDetector.h>
#include <EicLibrary.h>
#include <EicMagneticFieldFromGradients.h>


// =======================================================================================

EicMagneticFieldFromGradients::EicMagneticFieldFromGradients(const char *fileName) 
{
  // If file name is given, import it as a whole; in principle may want to add more
  // fields to existing structure; should work I guess;
  if (fileName) {
    TFile fin(ExpandedFileName(fileName));

    if (!fin.IsOpen()) 
      fLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Failed to open '%s' field!  \033[0m", 
		     fileName);

    EicMagneticFieldFromGradients *fptr = 0;
    fin.GetObject(_EIC_MAGNETIC_FIELD_FROM_GRADIENTS_, fptr);
    fin.Close();

    if (!fptr)
      fLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Failed to import '%s' field!  \033[0m", 
		     fileName);

    *this = *fptr;
  } //if

  // Is it really needed?; just set whatever index >2;
  fType = 3;

  mInitialized = false;
} // EicMagneticFieldFromGradients::EicMagneticFieldFromGradients()

// ---------------------------------------------------------------------------------------

int EicMagneticFieldFromGradients::AddBeamLineElementGrads(const char *directory, float fieldScaler, int color)
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
	  
	  // FIXME: this crap should be fixed one day;
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
		  AddFieldGradient(map);//new EicBeamLineElementMap(fileName.Data(), 0, 0, color));
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
} // EicMagneticFieldFromGradients::AddBeamLineElementMaps()

// ---------------------------------------------------------------------------------------

int EicMagneticFieldFromGradients::CreateYokeVolumes(Bool_t Active)
{
  FairRunSim *fRun = FairRunSim::Instance();

  // Loop through all the maps and call detector construction routines if available; at present only 
  // beam line elements can do (and perhaps even there this functionality is rather artificial);
  for(unsigned mm=0; mm<mMaps.size(); mm++) {
    EicMagneticFieldGrad *fmap = mMaps[mm];

    if (fmap->CapableToBuildYoke()) {
      //printf("here: %s\n", fmap->GetDetectorName().Data()); exit(0);

      fRun->AddModule(new EicFieldGradDetector(fmap, Active));
    } //if
  } //for mm

  return 0;
} // EicMagneticFieldFromGradients::CreateYokeVolumes()

// ---------------------------------------------------------------------------------------

int EicMagneticFieldFromGradients::InitializeFieldGradients()
{
  for(unsigned mm=0; mm<mMaps.size(); mm++) {
    EicMagneticFieldGrad *fmap = mMaps[mm];

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
  /*
  {
    FairRun *fRun = FairRun::Instance();

    // I hope there is no need to save/restore current directory here?;
    fRun->GetOutputFile()->cd();

    Write(_EIC_MAGNETIC_FIELD_);
  }
  */
  return 0;
} // EicMagneticFieldFromGradients::InitializeFieldGradients()

// ---------------------------------------------------------------------------------------

int EicMagneticFieldFromGradients::GetFieldSumValue(const double xx[], double B[]) 
{
  // Reset field to 0.0 for clarity;
  for(unsigned iq=0; iq<3; iq++)
    B[iq] = 0.0;

  if (!mInitialized && InitializeFieldGradients()) return -1;
  
  // FIXME: need an error printout here I guess;
  if (!xx || !B || !mMaps.size()) return -1;
  
  // Failure per default;
  int ret = -1;
 

  // For now a trivial implementation: just a sum of all maps which can provide
  // field at this location xx[];
  for(unsigned mm=0; mm<mMaps.size(); mm++) {
    EicMagneticFieldGrad *fmap = mMaps[mm];
    
    double BMap[3];
    
    if (!fmap->GetFieldValue(xx, BMap)) {
      for(unsigned iq=0; iq<3; iq++) 
	B[iq] += BMap[iq];
      
      ret = 0;
    } //if
  } //for mm
  
  return ret;
} // EicMagneticFieldFromGradients::GetFieldSumValue()

// -----------------------------------------------------------------------------------------------

/*
int EicMagneticFieldFromGradients::Export(const char *fileName) const
{
  //if (!mInitialied) return -1;

  // Yes, export always happens precisely to the path given via 'fileName' (no VMCWORKDIR
  // expansion like in importTpcDigiParameters());
  TFile fout(fileName, "RECREATE");

  if (!fout.IsOpen())
  {    
    printf("-E- EicMagneticFieldFromGradients::Export() -> failed to open '%s' for writing!\n", fileName);
    return -1;
  } //if

  fout.WriteObject(this, _EIC_MAGNETIC_FIELD_);
  fout.Close();

  return 0;
} // EicMagneticFieldFromGradients::Export()
*/

// =======================================================================================

 //ClassImp(EicPndFieldMap)
ClassImp(EicMagneticFieldFromGradients)
