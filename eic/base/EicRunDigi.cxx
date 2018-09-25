//
// AYK (ayk@bnl.gov), 2014/09/09
//
//  A trivial (for now) extension of FairRunAna class;
//

#include <assert.h>
#include <math.h>

#include "TMath.h"

#include <FairEventHeader.h>

#include <EicRunDigi.h>

// ---------------------------------------------------------------------------------------

void EicRunDigi::ImportRealHits(const char *fileName, const char *treeName)
{
  // This function can also be called from the constructor, so prefer 
  // to fall out in case of problems;
  if (!fileName || !treeName)
    fLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Zero input file pointer!  \033[0m"); 

  mInputHitFileName = TString(fileName);

  // Do not mind to open the file right here; assume exact path given; distinguish
  // betwee ROOT file and any other extension (assuming ASCII file then);
  if (mInputHitFileName.EndsWith(".root")) {
    mInputHitRootFile = new TFile(fileName);
    if (!mInputHitRootFile->IsOpen())
      fLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Failed to open '%s' ROOT input file with hits!  \033[0m", 
		     fileName);

    // And get access to the tree with hits; FIXME: #define, please;
    mHitTree = (TTree*)mInputHitRootFile->Get(/*"TTracking"*/treeName);
    if (!mHitTree)
      fLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m No '%s' tree found in the ROOT file '%s'!  \033[0m", 
		     treeName, fileName);
  }
  else {
    mInputHitAsciiFile = fopen(fileName, "r");
    if (!mInputHitAsciiFile) 
      fLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Failed to open '%s' ASCII input file with hits!  \033[0m", 
		     fileName);
  } //if
} // EicRunDigi::ImportRealHits()

// ---------------------------------------------------------------------------------------

//
// NB: do not care about any efficiency here; code is only useful 
// for test run data analysis, etc; 
//

int EicRunDigi::GetDetectorHits(const char *name, unsigned group, unsigned offset, 
				unsigned mdim, double local[], double sigma[])
{ 
  // Identified component counter (see check at the end);
  unsigned counter = 0;

  // Sanity check;
  if (!name || !local) return -1;

  // Reset all components to 0.0 (sane default state);
  for(unsigned iq=0; iq<mdim; iq++)
    local[iq] = 0.0;

  // Check, that event ID has not changed since last import;
  FairEventHeader *evHeader = GetEventHeader();
  //printf("entry %d\n", evHeader->GetMCEntryNumber());
  if (evHeader->GetMCEntryNumber() != mLastImportedEventID) {
    // Import next track;
    if (mInputHitRootFile) {
      if (evHeader->GetMCEntryNumber() < mHitTree->GetEntries()) {
	mHitTree->GetEntry(evHeader->GetMCEntryNumber());

	if (mVectorInputMode)
	  for(unsigned br=0; br<mBranchBuffers.size(); br++) {
	    BranchBuffer *buffer = mBranchBuffers[br];

	    //printf("%d -> %f %f\n", buffer->mDataVector->size(), 
	    //	   (*buffer->mDataVector)[0], (*buffer->mErrorVector)[0]);
	    for(unsigned iq=0; iq<_ARRAY_DIM_MAX_; iq++) {
	      buffer->mDataArray[iq] = (*buffer->mDataVector)[iq] + iq*0.38 + 26.60;
	      //buffer->mDataArray[iq] = (*buffer->mDataVector)[iq];// + iq*0.38;// + 26.60;
	      if (buffer->mDataVector) {
		buffer->mErrorArray[iq] = (*buffer->mErrorVector)[iq];
		//printf("%f\n", buffer->mErrorArray[iq]);
		//if (buffer->mErrorArray[iq] < 0.2) buffer->mErrorArray[iq] = 0.2;
		//if (buffer->mErrorArray[iq] > 0.1) buffer->mErrorArray[iq] = 2.0;
	      }
	    } //for iq
	  } //if .. for br

#if _LATER_
	for(unsigned br=0; br<mBranchBuffers.size(); br++) {
	  BranchBuffer *buffer = mBranchBuffers[br];
	  
	  if (buffer->mDataBranchName.EqualTo("UVAEIC")) {
	    double x = buffer->mArr[0], y = buffer->mArr[1], ra = 6.067*TMath::Pi()/180;

	    if (x == _FLOAT_INVALID_ || y == _FLOAT_INVALID_)
	      buffer->mArr[0] = buffer->mArr[1] = _FLOAT_INVALID_;
	    else {
	      double qx = 2*x*tan(ra), qy = 2*y;

	      double clustY1 = (qx+qy)/2;
	      double clustY0 = (qy-qx)/2;

	      //buffer->mArr[0] =  clustY0*0.98;
	      //buffer->mArr[1] =  clustY1*0.98;
	      buffer->mArr[0] =  clustY0;//*1.015;
	      buffer->mArr[1] =  clustY1;//*1.015;
	    } //if
	  } //if
	}
#endif
	for(unsigned col=0; col<mDescriptors.size(); col++) {
	  ColumnOrBranchDescriptor *descriptor = mDescriptors[col];

	  // Yes, check entry validity;
	  if (*descriptor->mDataPtr != _FLOAT_INVALID_) {
	    descriptor->mData = *descriptor->mDataPtr;
	    //printf("%s -> %d/%d -> %f\n", descriptor->mDetName.Data(), 
	    //	   descriptor->mGroup, descriptor->mWhat, descriptor->mData);

	    if (descriptor->mErrorPtr)
	      descriptor->mError = *descriptor->mErrorPtr;

	    descriptor->mHaveFreshData = true;
	  } //if
	} //for col
      } //if
    }
    else {
      // Read in the next line from ASCII input hit file;
      for(unsigned col=0; col<mDescriptors.size(); col++) {
	ColumnOrBranchDescriptor *descriptor = mDescriptors[col];

	if (fscanf(mInputHitAsciiFile, "%lf", &descriptor->mData) != 1) return -1;
	//#if 1
	//if (col == 8) descriptor->mData *= 1860;
	//if (col == 8) descriptor->mData *= 10;
	//#endif
	descriptor->mHaveFreshData = true;
      } //for col
    } //if

    mLastImportedEventID = evHeader->GetMCEntryNumber();
  } //if

  //
  // This stuff is the same for ASCII and ROOT input;
  //

    // Loop through all column descriptors and fill out matching components
    // of the output array; if at least one match found, return success;
  for(unsigned col=0; col<mDescriptors.size(); col++) {
    ColumnOrBranchDescriptor *descriptor = mDescriptors[col];
      
    if (descriptor->mHaveFreshData && descriptor->mDetName.EqualTo(name) && 
	descriptor->mGroup == group && 
	descriptor->mWhat >= offset && descriptor->mWhat < offset+mdim) {
      unsigned index = descriptor->mWhat - offset;
      // Convert [mm] (Aiwu's - and also Kondo's & Bob's? - format) to [cm]; 
      // FIXME: make configurable later?;
      //local[index] = 0.1 * descriptor->mData;
      //if (sigma) sigma[index] = 0.1 * descriptor->mError;
      local[index] = descriptor->mScale * descriptor->mData;
      if (sigma) sigma[index] = descriptor->mScale * descriptor->mError;
      
      // Give data away once -> make a usage note;
      descriptor->mHaveFreshData = false;
      
      // Component found -> increment counter;
      counter++;
    } //if
  } //for col

  return (counter == mdim ? 0 : -1);
} // EicRunDigi::GetDetectorHits()

// ---------------------------------------------------------------------------------------

ClassImp(EicRunDigi)
