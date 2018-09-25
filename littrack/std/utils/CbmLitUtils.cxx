#include "CbmLitUtils.h"

#include "TCanvas.h"
#include "TSystem.h"

namespace lit
{

void SaveCanvasAsImage(
   TCanvas* c,
   const std::string& dir)
{
   if (dir == "") return;
   gSystem->mkdir(dir.c_str(), true); // create directory if it does not exist
   c->SaveAs(std::string(dir + std::string(c->GetTitle()) + ".eps").c_str());
   c->SaveAs(std::string(dir + std::string(c->GetTitle()) + ".png").c_str());
   c->SaveAs(std::string(dir + std::string(c->GetTitle()) + ".gif").c_str());
}

string FindAndReplace(
		const string& name,
		const string& oldSubstr,
		const string& newSubstr)
{
	string newName = name;
	Int_t startPos = name.find(oldSubstr);
	newName.replace(startPos, oldSubstr.size(), newSubstr);
	return newName;
}

vector<string> Split(
		const string& name,
		char delimiter)
{
	vector<string> result;
	Int_t begin = 0;
	Int_t end = name.find_first_of(delimiter);
	while (end != string::npos) {
		string str = name.substr(begin, end - begin);
		if (str[0] == delimiter) str.erase(0, 1);
		result.push_back(str);
		begin = end;
		end = name.find_first_of(delimiter, end + 1);
	}
	result.push_back(name.substr(begin + 1));
	return result;
}

}
