//
// AYK (ayk@bnl.gov), 2014/03/11
//
//  EicRoot media interface to TGeo, related FairRoot stuff & Co;
//

#include <assert.h>
#include <stdlib.h>

#include <TGeoManager.h>

#include <FairLogger.h>

#include <EicLibrary.h>
#include <EicMediaHub.h>

// =======================================================================================

EicMediaHub::EicMediaHub(char *mediaName):  geoLoad(0), geoFace(0), Media(0), geobuild(0), 
					    fSingleMedium(0)
#if _LATER_
					 , fMediaMap(0)
#endif
{
  // Figure out later was it a single medium or media file name;
  fMediaName = TString(mediaName);
} // EicMediaHub::EicMediaHub()

// ---------------------------------------------------------------------------------------

int EicMediaHub::importMediaMapFile(TString &mediaMapFileName)
{
#if _LATER_
  TString buffer;
  std::ifstream is(mediaMapFileName);
    
  // File does not exist; Ok;
  if (!is.is_open()) return 0;

  fMediaMap = new mEntry(EicStlKeyCompare);

  while (!is.eof())
  {
    buffer.ReadLine(is, kTRUE); 
    //std::cout << buffer << std::endl;

    // Allow for some readability in this file; skip comments and empty lines;
    if (buffer.BeginsWith("#") || buffer.IsNull()) 
      continue;
    else
    if (buffer.BeginsWith("color"))
    {
      float rgb[3];
      char color[128], medium[128];
      // Is it really RGB encoded here?; does not matter;
      sscanf(buffer.Data(),"%s %f %f %f %s", color, rgb + 0, rgb + 1, rgb + 2, medium);
      const EicStlKey *key = new EicStlKey(3, rgb);

      // Well, duplicate entries should not happen;
      if (fMediaMap->find(key) != fMediaMap->end()) return -1;
      
      // Get medium pointer and add one more mapping entry;
      //printf("%s\n", medium);
      TGeoMedium *mptr = GetMedium(medium);
      assert(mptr);
      (*fMediaMap)[key] = mptr;
    }
    else
      return -1;
  } // for inf

  is.close();
#endif

  return 0;
} // EicMediaHub::importMediaMapFile()

// ---------------------------------------------------------------------------------------

void EicMediaHub::Init()
{
  // Initialize these pointers once; 
  //if (!geoLoad)
  //{
  // Need it only once, here;
  FairLogger *fLogger =  FairLogger::GetLogger();

  geoLoad = FairGeoLoader::Instance();
  geoFace = geoLoad->getGeoInterface();
  
  Media =  geoFace->getMedia();
  geobuild = geoLoad->getGeoBuilder();

  // Also try to resolve single medium pointer once; right here;
  {
#if _OLD_
    TString mediaMapFileName(fMediaName);

    // Consider absolute path case;
    if (!fMediaName.BeginsWith("/") && !fMediaName.BeginsWith("./"))
      mediaMapFileName = TString(getenv("VMCWORKDIR")) + fMediaName;
#else
    TString mediaMapFileName(ExpandedFileName(fMediaName));
#endif

    // Well, first try to see whether a medium with such a name exists; ambiguity whether 
    // it's a medium map file name in STL/SLP case or a single medium name is rather 
    // unlikely, yet check that (if single medium exists), there is no readable file 
    // with this name;
    if (!fMediaName.IsNull()) {
      fSingleMedium = GetMedium(fMediaName.Data());

      // Is there a better check on read access?;
      std::ifstream is(mediaMapFileName);
      
      if (fSingleMedium) {
	if (is.is_open())
	  fLogger->Fatal(MESSAGE_ORIGIN, 
			 "\033[5m\033[31m Ambiguity: both single medium (%s) "
			 "and media file with this name (%s) exist! \033[0m", 
			 fMediaName.Data(), mediaMapFileName.Data());
      } 
      else {
	// This is in fact needed for STL/SLP file import; do not mind to try anyway;
	if (importMediaMapFile(mediaMapFileName))
	  fLogger->Fatal(MESSAGE_ORIGIN, 
			 "\033[5m\033[31m Failed to import media map file (%s)! \033[0m", 
			 mediaMapFileName.Data());
      } //if
    } //if
  }
  //} //if

  //return 0;  
} // EicMediaHub::Init()

// ---------------------------------------------------------------------------------------

TGeoMedium *EicMediaHub::GetMedium(const char *medium)
{
  //printf(" -> getting %s\n", medium);
  // If media is known to the current gGeoManager, just return respective pointer;
  TGeoMedium *gmedium = gGeoManager->GetMedium(medium);
  if (gmedium) return gmedium;
  //printf("  ... does not exist\n");

  // Make gGeoManager about this medium;
  {
    FairGeoMedium *fmedium  = Media->getMedium(medium);

    // Basically means no such medium in media.geo file;
    if (!fmedium) return 0;

    geobuild->createMedium(fmedium);

    // Should not happen;
    assert(gGeoManager->GetMedium(medium));
  } //if

  // NB: this can still return 0 pointer if no such media existed in media.geo file;
  return gGeoManager->GetMedium(medium);
} // EicMediaHub::GetMedium()

// =======================================================================================

ClassImp(EicMediaHub)
