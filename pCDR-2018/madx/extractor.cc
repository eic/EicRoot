
//
//  c++ -o extractor extractor.cc 
//

//
// FIXME: for now for outgoing electron side it is easier just to 
// change rotation angles by hand in the output file;
// 

#include <vector>
#include <map>
#include <string>

#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

// Give some small default value, which would allow on-axis particles 
// to pass through, but block everything else and immediately trigger 
// questions if some of the magnet bores are not present in the database; 
#define _APERTURE_RADIUS_DEFAULT_   (0.01)

// FIXME: command-line parameter, please;
//#define _ZMIN_                     ( 0.00)
#define _ZMIN_                    (-40.00)
//#define _ZMAX_                     ( 0.00)
#define _ZMAX_                     (40.00)

class Bore {
public:
  Bore(double rZin, double rZout, double dOut): 
    mApertureRadiusZin(rZin), mApertureRadiusZout(rZout), mOutsideDiameter(dOut) {};
  ~Bore() {};

  double mApertureRadiusZin, mApertureRadiusZout, mOutsideDiameter;
};

static std::map<std::string, Bore*> bores;

class MagElement {
public:
  MagElement(std::string name): mName(name), mApertureRadiusZin(_APERTURE_RADIUS_DEFAULT_), 
				mApertureRadiusZout(_APERTURE_RADIUS_DEFAULT_), mOutsideDiameter(0.0),
				mTHETAstart(0.0), mTHETAend(0.0),
				mSstart(0.0), mSend(0.0), mXstart(0.0), mYstart(0.0), mZstart(0.0), mXend(0.0), mYend(0.0), mZend(0.0), 
				mLength(0.0), mX(0.0), mY(0.0), mZ(0.0), mTHETA(0.0) {
    if (bores.find(name) != bores.end()) {
      Bore *bore = bores[name];

      // Convert [cm] -> [m];
      mApertureRadiusZin  = 0.01 * bore->mApertureRadiusZin;
      mApertureRadiusZout = 0.01 * bore->mApertureRadiusZout;
      mOutsideDiameter    = 0.01 * bore->mOutsideDiameter;
    } //if
  };
  ~MagElement() {};

  virtual double GetField( void )    const { return 0.0; };
  virtual double GetGradient( void ) const { return 0.0; };

  // NB: need at least one virtual method, otherwise dynamic_cast fails
  // claiming base class is not polymorphic;
  virtual void Calculate(double Brho) = 0;
  virtual void Print( void ) { printf("%10s -> L = %7.3f; X = %7.3f, Z = %8.3f, TH = %7.2f mrad", 
				      mName.c_str(), mLength, mX, mZ, mTHETA * 1000); };
  virtual void WriteOut(FILE *fout) { 
    // I'm changing format of this file anyway (Apr'2018), so change coordinate order to XYZ too;
    fprintf(fout, "%10s %8.4f  %8.4f %8.4f %7.4f    %7.3f  %7.4f %7.3f  %7.2f  %7.3f  %8.3f\n", 
	    mName.c_str(), mX, mY, mZ, mApertureRadiusZin, mApertureRadiusZout, mOutsideDiameter, mLength, mTHETA * 1000., GetField(), GetGradient()); };

  std::string mName;
  // mOutsideDiameter has a meaning of full width (height) for dipoles;
  double mApertureRadiusZin, mApertureRadiusZout, mOutsideDiameter;

  // Will be used for length calculation;
  double mSstart, mSend, mTHETAstart, mTHETAend;
  // Will be used for XYZ calculation;
  double mXstart, mYstart, mZstart, mXend, mYend, mZend;

  // Ultimate numbers, which will go into the tables;
  double mLength, mX, mY, mZ, mTHETA;
};

class Dipole: public MagElement {
public:
  Dipole(std::string name, double angle): MagElement(name), mANGLE(angle), mField(0.0) {};
  ~Dipole() {};

  double GetField( void ) const { return mField; };
  void Calculate(double Brho) { mField = Brho * mANGLE/mLength; };
  void Print( void ) { MagElement::Print(); printf(" (D), ANGLE = %9.6f -> %8.3f  [T]\n", mANGLE, mField); };
  double mANGLE;

  // (Constant) field value;
  double mField;
};

class Quadrupole: public MagElement {
public:
  Quadrupole(std::string name, double k1l): MagElement(name), mK1L(k1l), mGradient(0.0) {};
  ~Quadrupole() {};

  double GetGradient( void ) const { return mGradient; };
  void Calculate(double Brho) { mGradient = Brho * mK1L/mLength; };
  void Print( void ) { MagElement::Print(); printf(" (Q), K1L   = %9.6f ->%9.3f [T/m] \n", mK1L, mGradient); };
  double mK1L;

  // (Constant) field gradient value;
  double mGradient;
};

class BeamLine {
public:
  BeamLine(unsigned data_par_num, unsigned survey_par_num, bool rear = false): 
    mBrho(0.0), mDataParNum(data_par_num), mSurveyParNum(survey_par_num)/*, mZcoordFlip(rear)*/ {};
  ~BeamLine() {};

  void AddDataFile(const char *fname)   { mDataFiles.push_back(fname); };
  void AddSurveyFile(const char *fname) { mSurveyFiles.push_back(fname); };

  double mBrho;

  // ALFY is missing in the electron data files; extra columns exist in electron survey file;
  unsigned mDataParNum, mSurveyParNum;
  //bool mZcoordFlip;
  std::vector<std::string> mDataFiles, mSurveyFiles, mAllFiles;

  std::map<std::string, MagElement*> mMagElements;
  std::map<double, MagElement*> mOrderedMagElements;
};

static char *GetTrueElementName(const char *name)
{
  static char rname[1024];
  assert(strlen(name) < 1024);

  strcpy(rname, name + (name[0] == '\"' ? 1 : 0));
  {
    unsigned slen = strlen(rname);

    if (rname[slen-1] == '\"') rname[slen-1] = 0;
  }
  {
    unsigned slen = strlen(rname);
    if (!strcmp(rname+slen-2, "_E")  || !strcmp(rname+slen-2, "_F"))  rname[slen-2] = 0;
    if (!strcmp(rname+slen-2, "_6"))                                  rname[slen-2] = 0;
    if (!strcmp(rname+slen-3, "_E1") || !strcmp(rname+slen-3, "_E2")) rname[slen-3] = 0;
  }

  return rname;
} // GetTrueElementName()

static bool IsElementStartLine(const char *name)
{
  unsigned slen = strlen(name);
  
  // NB: yes, E1 & E2 seem to be swapped; also rear files need special treatment;
  return (!strcmp(name+slen-2, "_F")   || !strcmp(name+slen-3, "_E2") ||
	  !strcmp(name+slen-3, "_F\"") || !strcmp(name+slen-4, "_E2\""));
} // IsElementStartLine()

static bool IsElementEndLine(const char *name)
{
  unsigned slen = strlen(name);
  
  return (!strcmp(name+slen-2, "_E")   || !strcmp(name+slen-3, "_E1") || 
	  !strcmp(name+slen-3, "_E\"") || !strcmp(name+slen-4, "_E1\"") || !strcmp(name+slen-3, "_6\""));
} // IsElementEndLine()

int main(int argc, char **argv)
{
  // Make it easy -> hardcode damn file names;
  BeamLine *electron = new BeamLine(14, 15), *hadron = new BeamLine(15, 8), *beamlines[2] = {hadron, electron};
  electron->AddDataFile("rr-data-norad-ver3-10GeV-elke.tfs.no-dash");
  electron->AddSurveyFile("rr-survey-ver3-10GeV-elke.tfs");
  hadron->AddDataFile("Hadron-275GeV.dataForward");
  hadron->AddSurveyFile("Hadron-275GeV.surveyForward");
  hadron->AddDataFile("Hadron-275GeV.dataRear");
  hadron->AddSurveyFile("Hadron-275GeV.surveyRear");

  // Read bore information file;
  {
    FILE *fbore = fopen("bores.txt", "r");
    assert(fbore);

    char buffer[1024];

    while (fgets(buffer, 1024, fbore)) {
      if (!strlen(buffer) || buffer[0] == '#') continue;

      char name[256];
      double rZin, rZout, rOut; 
      int ret = sscanf(buffer, "%s %lf %lf %lf", name, &rZin, &rZout, &rOut);

      if (ret == 4) bores[name] = new Bore(rZin, rZout, 2*rOut);
    } //while

    fclose(fbore);
  } 

  // Loop through both beamlines independently;
  for(unsigned bl=0; bl<2; bl++) {
    BeamLine *beamline = beamlines[bl];

    // This looks dumb, but it works; data files will be parsed first;
    for(unsigned fl=0; fl<beamline->mDataFiles.size(); fl++) 
      beamline->mAllFiles.push_back(beamline->mDataFiles[fl]);
    for(unsigned fl=0; fl<beamline->mSurveyFiles.size(); fl++) 
      beamline->mAllFiles.push_back(beamline->mSurveyFiles[fl]);

    // Parse data files first;
    for(unsigned fl=0; fl<beamline->mAllFiles.size(); fl++) {
      std::string &fname = beamline->mAllFiles[fl];
      // FIXME: tired of this *hit, sorry;
      bool zflip = fname.find("Rear") != std::string::npos;
      if (beamline == electron) zflip = true;

      printf("\n   --> %s\n\n", fname.c_str());
      {
	FILE *fin = fopen(fname.c_str(), "r");
	assert(fin);
      
	int ret;
	// NB: need a buffer string, since may want to extract header variables as well;
	char buffer[1024];

	while (fgets(buffer, 1024, fin)) {
	  // Try to interpret the string as header data first;
	  char str1[128], str2[128], str3[128], str4[128], str5[128], *vptr = 0;
	  int hret = sscanf(buffer, "%s %s %s %s %s", str1, str2, str3, str4, str5);
	  if (hret == 3) 
	    vptr = str3;
	  else if (hret == 4) 
	    vptr = str4;

	  if (vptr) {
	    // Header string; fish out the ones I may need; if assigned more than once,
	    // (in forward and rearward data files, survey files, etc) check consistency;
	    if (!strcmp(str2, "PC")) {
	      // FIXME: hardcode this coefficient in a more intelligent way later;
	      double value = atof(vptr) * 3.335640951;
	      if (beamline->mBrho) 
		assert(value == beamline->mBrho);
	      else
		beamline->mBrho = value;
	    } //if
	  } else {
	    char name[256];

	    if (fl < beamline->mDataFiles.size()) {
	      double S, L, BETX, ALFX, BETY, ALFY, MUX, MUY, DX, DPX, ANGLE, K0L, K1L, K2L; 

	      // Try this same string as data entry;
	      switch (beamline->mDataParNum) {
	      case 14:
		ALFY = 0.0; 
		{
		  char qname[256] = "";
		  // NB: K1L column has dash symbols rather than '-' and fscanf() will terminate 
		  // conversion at this point if %lf and &K1L were used; 
		  ret = sscanf(buffer, "%s %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %s %lf", 
			       name, &S, &L, &BETX, &ALFX, &BETY, &MUX, &MUY, &DX, &DPX, &ANGLE, &K0L, qname, &K2L);
		  // NB: this will fail for values starting with the dash; FIXME later;
		  K1L = atof(qname);
		}
		break;
	      case 15:
		ret = sscanf(buffer, "%s %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", 
			     name, &S, &L, &BETX, &ALFX, &BETY, &ALFY, &MUX, &MUY, &DX, &DPX, &ANGLE, &K0L, &K1L, &K2L);
		break;
	      default:
		assert(0);
	      } //switch
	      
	      //printf("%d\n", ret);
	      if (ret != beamline->mDataParNum) continue;
	      
	      // For now I'm only interested in the field strength in these files; ignore all other lines;
	      // FIXME: YI6_HB1 magnet with 0.0 field at 275 GeV is ignored in this picture;
	      if (!ANGLE && !K1L) continue;
	      char *rname = GetTrueElementName(name);
	      
	      // If K1L is non-zero, it is a quad; NB: ANGLE can be 0.0 for the dipole; 
	      // For now I'm only interested in the field strength in the 'data' files -> 
	      // it looks like I can ignore the _E and _F lines;
	      if (K1L)
		beamline->mMagElements[rname] = new Quadrupole(rname, /*L,*/ K1L);
	      else
		beamline->mMagElements[rname] = new Dipole    (rname, /*L,*/ ANGLE);
	      
	      //printf("%10s -> %10s -> %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf (%s)\n", 
	      //     name, rname, S, L, BETX, ALFX, BETY, ALFY, MUX, MUY, DX, DPX, ANGLE, K0L, K1L, K2L, qname);
	    } else {
	      double S, L, ANGLE, X, Y, Z, THETA, dummy;

	      // Try this same string as survey entry; at most 15 columns;
	      int ret = sscanf(buffer, "%s %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", 
			       name, &S, &L, &ANGLE, &X, &Y, &Z, &THETA, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy);
	      if (ret != beamline->mSurveyParNum) continue;

	      if (zflip) Z *= -1.0;
	      if (THETA < -6.0) THETA += 2*M_PI;

	      char *rname = GetTrueElementName(name);
	      MagElement *element = beamline->mMagElements[rname]; 
	      if (element) {
		// Well, center point is missing in hadron files -> base XYZ calculation of 
		// front/end entries (which can come irn reversed order like in electron file);
		if (IsElementStartLine(name)) {
		  element->mSstart = S;
		  element->mTHETAstart  = THETA;
		  
		  element->mXstart = X;
		  element->mYstart = Y;
		  element->mZstart = Z;

		} else if (IsElementEndLine(name)) {
		  element->mSend   = S;
		  element->mTHETAend  = THETA;

		  element->mXend   = X;
		  element->mYend   = Y;
		  element->mZend   = Z;
		} //if
	      } else {
		printf("Orhpan beam line element: %s\n", rname);
	      } //if
	    } //if
	  }
	} //while

	fclose(fin);
      } 
    } //for fl

    printf(" Brho --> %f\n", beamline->mBrho);
    for(std::map<std::string, MagElement*>::iterator it = beamline->mMagElements.begin(); 
	it != beamline->mMagElements.end(); it++) {
      MagElement *element = it->second;

      if (element) {
	element->Print();
	printf("  %10s -> Sstart = %8.4f, Send = %8.4f\n", it->first.c_str(), element->mSstart, element->mSend); 
	assert(element->mSstart && element->mSend);

	// Calculate element length; fabs() is the easiest hack for hadron rear elements;
	element->mLength = fabs(element->mSend - element->mSstart); assert(element->mLength);

	element->mTHETA = (element->mTHETAstart + element->mTHETAend)/2;

	// Calculate element center point coordinates;
	element->mX = (element->mXstart + element->mXend)/2;
	element->mY = (element->mYstart + element->mYend)/2;
	element->mZ = (element->mZstart + element->mZend)/2; 

	element->Calculate(beamline->mBrho);

	beamline->mOrderedMagElements[element->mZ] = element;
      } //if
    } //for it
#if 0
    printf(" \n bl#%d: Dipoles:\n", bl);
    for(std::map<std::string, MagElement*>::iterator it = beamline->mMagElements.begin(); 
	it != beamline->mMagElements.end(); it++) {
      Dipole *dipole = dynamic_cast<Dipole*>(it->second);
      if (dipole) dipole->Print();
    } //for it
    printf(" \n bl#%d: Quads:\n", bl);
    for(std::map<std::string, MagElement*>::iterator it = beamline->mMagElements.begin(); 
	it != beamline->mMagElements.end(); it++) {
      Quadrupole *quad = dynamic_cast<Quadrupole*>(it->second);
      if (quad) quad->Print();
    } //for it
#endif
#if 1
    printf(" \n bl#%d: Ordered element sequence:\n", bl);
    for(std::map<double, MagElement*>::iterator it = beamline->mOrderedMagElements.begin(); 
	it != beamline->mOrderedMagElements.end(); it++) 
      it->second->Print();
#endif

    // Produce output file;
    {
      char fname[1024];
      snprintf(fname, 1024-1, "%s/ir-magnets-2018-04-04-%s.dat", bl ? "E" : "H", bl ? "electrons" : "hadrons");
      FILE *fout = fopen(fname, "w"); assert(fout);

      // Copy over the "header";
      {
	FILE *fheader = fopen("header.txt", "r");
	assert(fheader);
	{
	  char buffer[1024];

	  while (fgets(buffer, 1024-1, fheader)) fprintf(fout, "%s", buffer);
	} 
	fclose(fheader);
      }

      // The magnetic elements;
      for(std::map<double, MagElement*>::iterator it = beamline->mOrderedMagElements.begin(); 
	  it != beamline->mOrderedMagElements.end(); it++) 
	if (it->second->mZ >= _ZMIN_ && it->second->mZ <= _ZMAX_) 
	  it->second->WriteOut(fout);

      fclose(fout);
    }
  } //for bl

  exit(0);
} // main()
