//
// AYK (ayk@bnl.gov), 2014/08/13
//
//  Beam pipe geometry description (around IP, so that a PCON structure will work);
//
//  NB: internal storage is ROOT TGeoPcon-oriented; it does not make sense to 
//      arrange inherited classes for pure cylindric and pure conical pieces; 
//      user however can define cylindric and conical pieces as well (interface 
//      functions are provided); representation in ROOT will be done via either 
//      TGeoPcon or (if polycone has one section only) TGeoTube/TGeoCone shapes;
//  
//  NB: it is assumed, that single piece has a constant thickness; rationale: to 
//      simplify user interface; so if beam pipe with the same material changes 
//      wall thickness at some Z along the beam line, a separate piece should be 
//      arranged in beampipe.C; these pieces will be represented as different 
//      TGeoPcon volumes (see beampipe.C code); if this ever becomes a 
//      performance problem, one can modify code and merge neighboring pieces
//      in a single TGeoPcon volume;
//
//  NB: elliptic pieces are not implemented as of yet; rationale: TGeoEltu with 
//      variable profile (which would allow to arrange a smooth transition to 
//      an asimuthally-symmetric pieces) is not available in ROOT (and I do not
//      want to hack hermetic transition by hand); then, it is not clear 
//      yet, whether elliptic tube profile is needed at EIC at all; once it 
//      becomes needed, the hope is that CAD drawings will become available;
//      and: in worst case just extend this set of classes by whatever crap needed;
//

#include <cmath>

#include <EicGeoParData.h>

#ifndef _BEAM_PIPE_GEO_PAR_DATA_
#define _BEAM_PIPE_GEO_PAR_DATA_

class BeamPipeSection: public TObject
{
  // Really no reason to hide anything from the "master" classes;
  friend class BeamPipeElement;
  friend class BeamPipeGeoParData;

 public:
  BeamPipeSection() { ResetVars(); };
 BeamPipeSection(double offset, double outerDiameter): mOffset(offset), 
    mOuterDiameter(outerDiameter) {};
  ~BeamPipeSection() {};
  
 private:
  void ResetVars() {
    mOffset = mOuterDiameter = 0.0;
  };

  Double_t mOffset;           // offset along the beam line
  Double_t mOuterDiameter;    // outer diameter 
  
  ClassDef(BeamPipeSection,2);
};

class BeamPipeElement: public TObject
{
  friend class BeamPipeGeoParData;

 public:
  enum SwapSign {NoSwap, Swap};

  BeamPipeElement() { ResetVars(); };
 BeamPipeElement(TString &material, double thickness): 
  mMaterial(material), mThickness(thickness), mIpElement(false), mSwapped(NoSwap), mAccuOffset(0.0) {};
  BeamPipeElement(BeamPipeElement *source, Bool_t ipElement = false, SwapSign swapped = NoSwap) { 
    *this      = *source;
    mIpElement = ipElement;
    mSwapped   = swapped;
  };
  ~BeamPipeElement() {};
  
  void AddSection(double offset, double outerDiameter) {
    mSections.push_back(new BeamPipeSection(offset, outerDiameter));
  };

 private:
  void ResetVars() {
    mThickness = mAccuOffset = 0.0;
    mSwapped = NoSwap;

    mIpElement = false;
  };

  const BeamPipeSection* GetFirstSection() const {
    if (!mSections.size()) return 0;

    return /*mSwapped == Swap ? mSections[mSections.size()-1] :*/ mSections[0];
  };
  const BeamPipeSection* GetFirstOrientedSection() const {
    if (!mSections.size()) return 0;

    return mSwapped == Swap ? mSections[mSections.size()-1] : mSections[0];
  };
  const BeamPipeSection* GetLastSection() const {
    if (!mSections.size()) return 0;

    return /*mSwapped == Swap ? mSections[0]                  :*/ mSections[mSections.size()-1];
  };
  const BeamPipeSection* GetLastOrientedSection() const {
    if (!mSections.size()) return 0;

    return mSwapped == Swap ? mSections[0]                  : mSections[mSections.size()-1];
  };
#if 0
  const BeamPipeSection* GetOrderedSection(unsigned id) const {
    if (id >= mSections.size()) return 0;

    return mSwapped == Swap ? mSections[mSections.size()-id-1] : mSections[id];
  };
#endif

  double GetLength() const {
    return fabs(GetFirstSection()->mOffset - GetLastSection()->mOffset);
  }; 

  TString mMaterial;                        // material in geometry/media.geo
  Double_t mThickness;                      // thickness
  
  // It may be more convenient to define local section coordinates 
  // in Z+ sequence, even for beam pipe pieces at negative Z;
  SwapSign mSwapped;                        // natural of Z-swapped section order 
  
  Bool_t mIpElement;                        // element at the IP location

  // Sections which define the ROOT TGeoPcon polycone;
  std::vector<BeamPipeSection*> mSections;  // sections to define TGeoPcon

  // Transient variable: accumulated offset; 
  Double_t mAccuOffset;                     //!
  
  ClassDef(BeamPipeElement,6);
};

class BeamPipeGeoParData: public EicGeoParData
{
 private:

 public:
 BeamPipeGeoParData(int version = -1, int subVersion = 0): 
  EicGeoParData("BEAMPIPE", version, subVersion), mIpElementID(-1) {};
  ~BeamPipeGeoParData() {};

  // Pieces will go along the beam line just in the order of AddElement() 
  // calls in beampipe.C;
  void AddElement(BeamPipeElement *element, 
		  BeamPipeElement::SwapSign swapped = BeamPipeElement::NoSwap) {
    // Yes, want to create an independent copy (and prevent further access);
    mElements.push_back(new BeamPipeElement(element, BeamPipeElement::NoSwap, swapped));
  };
  void AddIpElement(BeamPipeElement *element) {
    mElements.push_back(new BeamPipeElement(element, true));
  };

  //void Print(const char *option = 0) const {};
  int ConstructGeometry();

 private:
  bool CheckGeometry();

  Int_t mIpElementID;                       //! IP element ID in mElements[]

  // Ordered along the hadron beam direction; element '0' is centered 
  // around the IP accoring to its section offsets; others are attached with proper 
  // offsets which are calculated on-the-fly (so that pieces can for convenience be 
  // defined with local offsets of 0.0 or whatever else); units are [mm];
  std::vector<BeamPipeElement*> mElements;  // beam pipe elements in this order 
  
  ClassDef(BeamPipeGeoParData,8);
};

#endif
