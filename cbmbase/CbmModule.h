#include "FairModule.h"

#include "TString.h"

#include <map>

class TGeoNode;
class TGeoMatrix;

class CbmModule : public FairModule
{
public:
	CbmModule();
	CbmModule(const char*, const char*);
	~CbmModule();

	void ConstructGDMLGeometry(TGeoMatrix*);
	void ExpandNodeForGDML(TGeoNode*);

        void ConstructGeometry();

private:
	static std::map<TString, Int_t> fixedMats; //!
	static Bool_t isFirstGDML;                 //!

	ClassDef(CbmModule,1);	//CbmModule
};
