//
// AYK (ayk@bnl.gov), 2014/09/09
//
//  A trivial (for now) extension of EicRunAna class which allows one 
// to import real data into the digitization chain and substitute MC 
// hits preserving all the structure of simulation->digitization->reconstruction 
// data exchange files; in fact used for FLYSUB data analysis only;
//

#include <EicRunAna.h>

#include <EicGeoParData.h>

#ifndef _EIC_RUN_DIGI_
#define _EIC_RUN_DIGI_

class ColumnOrBranchDescriptor
{
 public:
 ColumnOrBranchDescriptor(const char *detName, unsigned group, EicGeoParData::IDXYZ what, 
			  double scale = 1.0,  
			  float *dataPtr = 0, float *errorPtr = 0):
  mDetName(detName), mGroup(group), mWhat(what), mHaveFreshData(false), 
    mData(0.0), mError(0.0), mDataPtr(dataPtr), mErrorPtr(errorPtr), mScale(scale) {};
  ~ColumnOrBranchDescriptor() {};

  // Column description;
  TString mDetName;
  unsigned mGroup;
  EicGeoParData::IDXYZ mWhat;

  // Branch entry offset (assume float[] array);
  float *mDataPtr, *mErrorPtr;

  // Want to give data away only once; 2-d request will fail; 
  bool mHaveFreshData;
  // The actual data and error values;
  double mData, mError;

  // Scaling factor needed to convert data to [cm] (or [rad] for radial 
  // detectors);
  double mScale;
};

#define _FLOAT_INVALID_ (-919191.0)
// Do not mind to have it static;
#define _ARRAY_DIM_MAX_ 100

class BranchBuffer {
 public:
 BranchBuffer(const char *dataBranchName, const char *errorBranchName = 0): 
  mDataBranchName(dataBranchName), mErrorBranchName(errorBranchName) { 
    for(unsigned iq=0; iq<_ARRAY_DIM_MAX_; iq++)
      mDataArray[iq] = mErrorArray[iq] = _FLOAT_INVALID_;

    mDataVector  = new std::vector<float>;
    mErrorVector = errorBranchName ? new std::vector<float> : 0;
  };
  ~BranchBuffer() {};

  TString mDataBranchName, mErrorBranchName;
  std::vector<float> *mDataVector, *mErrorVector;
  float mDataArray[_ARRAY_DIM_MAX_], mErrorArray[_ARRAY_DIM_MAX_];
};

class EicRunDigi : public EicRunAna
{
 public:
 EicRunDigi(/*const char *fileName = 0, const char *treeName = 0*/): //FairRunAna(), 
    mInputHitAsciiFile(0), mInputHitRootFile(0), mLastImportedEventID(-1), mHitTree(0), 
    mVectorInputMode(false) {
    //if (fileName && treeName) ImportRealHits(fileName, treeName);
  };
  ~EicRunDigi() {};

  // For now it's an ASCII file with FLYSUB hits;
  void ImportRealHits(const char *fileName, const char *treeName);
  void SetVectorInputMode() { mVectorInputMode = true; };

  // FIXME: no double-counting check;
  void SetColumnDescriptor(const char *detName, unsigned group, EicGeoParData::IDXYZ what, 
			  double scale = 1.0) {
    mDescriptors.push_back(new ColumnOrBranchDescriptor(detName, group, what, scale));
  };
  void SetBranchDescriptor(const char *detName, unsigned group, EicGeoParData::IDXYZ what,
			  const char *dataBranchName, const char *errorBranchName, unsigned offset, 
			  double scale = 1.0) {
    float *dataPtr = 0, *errorPtr = 0;

    // Loop through already defined branches and see whether this one exists already;
    for(unsigned br=0; br<mBranchBuffers.size(); br++) {
      BranchBuffer *buffer = mBranchBuffers[br];

      // FIXME: think on this, may not be exactly clean; 
      if (buffer->mDataBranchName.EqualTo(dataBranchName) && 
	  (!errorBranchName || buffer->mErrorBranchName.EqualTo(errorBranchName))) {
	dataPtr  = buffer->mDataArray  + offset;
	errorPtr = buffer->mErrorArray + offset;
	break;
      } //if
    } //for 

    // New branch -> have some work to do;
    if (!dataPtr) {
      BranchBuffer *buffer = new BranchBuffer(dataBranchName, errorBranchName);

      // FIXME: check branch existence; well, and float[3] size as well; 
      if (mVectorInputMode) {
	mHitTree->SetBranchAddress(dataBranchName,  &buffer->mDataVector);
	if (errorBranchName)
	  mHitTree->SetBranchAddress(errorBranchName, &buffer->mErrorVector);
      }
      else {
	mHitTree->SetBranchAddress(dataBranchName,  &buffer->mDataArray);
	if (errorBranchName)
	  mHitTree->SetBranchAddress(errorBranchName, &buffer->mErrorArray);
      } //if
      mBranchBuffers.push_back(buffer);

      dataPtr  = buffer->mDataArray  + offset;
      if (errorBranchName) errorPtr = buffer->mErrorArray + offset;
    } //if

    mDescriptors.push_back(new ColumnOrBranchDescriptor(detName, group, what, scale, dataPtr, errorPtr));
  };

  // No matter ASCII (Aiwu) or ROOT (Kondo); just let the caller know whether MC hits need 
  // to be substituted or not;
  bool HitImportMode() const { return !mInputHitFileName.IsNull(); };

  int GetDetectorHits(const char *name, unsigned group, unsigned offset, 
		      unsigned mdim, double local[], double sigma[] = 0);

 private:
  TString mInputHitFileName;                           // input file name

  FILE *mInputHitAsciiFile;                            //! ASCII file descriptor

  TFile *mInputHitRootFile;                            //! ROOT  file descriptor
  TTree *mHitTree;                                     //! ROOT tree with hits
  Bool_t mVectorInputMode;                             //! If true, assume branch with hits is a vector
  std::vector<BranchBuffer*> mBranchBuffers;           //! ROOT branch buffer vector

  // Do not want to disturb FairRunAna() activity at all; just keep track
  // on my own imported line counter and check it against FairRunAna() one;
  // once there is a mismatch, import new record;
  int mLastImportedEventID;                            //! identifier of the last imported event

  std::vector<ColumnOrBranchDescriptor*> mDescriptors; //!

  ClassDef(EicRunDigi,5)
};

#endif
