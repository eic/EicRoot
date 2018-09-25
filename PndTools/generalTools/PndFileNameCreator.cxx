#include "PndFileNameCreator.h"
#include "PndStringSeparator.h"
#include <sstream>
#include <iostream>


PndFileNameCreator::PndFileNameCreator():
	fFileName(), fExtPar("par"), fExtSim("sim"), fExtDigi("digi"), fExtReco("reco"),
	fExtTrackF("trackF"), fExtIdealTrackF("idealTrackF"), fExtKalman("kalman"),
	fExtRiemann("riemann"),fExtCombinedRiemann("combRiemann"),fExtVertex("vertex"),
	fVerbose(0)
{
}

PndFileNameCreator::PndFileNameCreator(std::string fileName):
	fFileName(fileName), fExtPar("par"), fExtSim("sim"), fExtDigi("digi"), fExtReco("reco"),
	fExtTrackF("trackF"), fExtIdealTrackF("idealTrackF"), fExtKalman("kalman"),
	fExtRiemann("riemann"),fExtCombinedRiemann("combRiemann"),fExtVertex("vertex"),
	fVerbose(0)
{
}




std::string PndFileNameCreator::GetParFileName(std::string addon, bool cut)
{
  return GetCustomFileName(fExtPar, addon, cut);
}

std::string PndFileNameCreator::GetSimFileName(std::string addon, bool cut)
{
  return GetCustomFileName(fExtSim, addon, cut);
}

std::string PndFileNameCreator::GetDigiFileName(std::string addon, bool cut)
{
	return GetCustomFileName(fExtDigi, addon, cut);
}

std::string PndFileNameCreator::GetRecoFileName(std::string addon, bool cut)
{
	return GetCustomFileName(fExtReco, addon, cut);
}

std::string PndFileNameCreator::GetTrackFindingFileName(std::string addon, bool cut)
{
	return GetCustomFileName(fExtTrackF, addon, cut);
}

std::string PndFileNameCreator::GetIdealTrackFindingFileName(std::string addon, bool cut)
{
	return GetCustomFileName(fExtIdealTrackF, addon, cut);
}

std::string PndFileNameCreator::GetRiemannFileName(std::string addon, bool cut)
{
	return GetCustomFileName(fExtRiemann, addon, cut);
}

std::string PndFileNameCreator::GetCombinedRiemannFileName(std::string addon, bool cut)
{
	return GetCustomFileName(fExtCombinedRiemann, addon, cut);
}


std::string PndFileNameCreator::GetKalmanFileName(std::string addon, bool cut)
{
	return GetCustomFileName(fExtKalman, addon, cut);
}

std::string PndFileNameCreator::GetVertexFileName(std::string addon, bool cut)
{
	return GetCustomFileName(fExtVertex, addon, cut);
}
 
// -----------------

//std::string PndFileNameCreator::GetParFileName(std::string inputFileName, std::string addon, bool cut)
//{
//	fFileName = inputFileName;
//	return GetParFileName(addon, cut);
//}
//
//std::string PndFileNameCreator::GetSimFileName(std::string inputFileName, std::string addon, bool cut)
//{
//	fFileName = inputFileName;
//	return GetSimFileName(addon, cut);
//}
//
//std::string PndFileNameCreator::GetDigiFileName(std::string inputFileName, std::string addon, bool cut)
//{
//	fFileName = inputFileName;
//	return GetDigiFileName(addon, cut);
//}
//
//std::string PndFileNameCreator::GetRecoFileName(std::string inputFileName, std::string addon, bool cut)
//{
//	fFileName = inputFileName;
//	return GetRecoFileName(addon, cut);
//}
//
//std::string PndFileNameCreator::GetTrackFindingFileName(std::string inputFileName, std::string addon, bool cut)
//{
//	fFileName = inputFileName;
//	return GetTrackFindingFileName(addon, cut);
//}
//
//std::string PndFileNameCreator::GetIdealTrackFindingFileName(std::string inputFileName, std::string addon, bool cut)
//{
//	fFileName = inputFileName;
//	return GetIdealTrackFindingFileName(addon, cut);
//}
//
//std::string PndFileNameCreator::GetKalmanFileName(std::string inputFileName, std::string addon, bool cut)
//{
//	fFileName = inputFileName;
//	return GetKalmanFileName(addon, cut);
//}
//
//std::string PndFileNameCreator::GetVertexFileName(std::string inputFileName, std::string addon, bool cut)
//{
//	fFileName = inputFileName;
//	return GetVertexFileName(addon, cut);
//}
//
//std::string PndFileNameCreator::GetRiemannFileName(std::string inputFileName, std::string addon, bool cut)
//{
//	fFileName = inputFileName;
//	return GetRiemannFileName(addon, cut);
//}
//
//std::string PndFileNameCreator::GetCombinedRiemannFileName(std::string inputFileName, std::string addon, bool cut)
//{
//	fFileName = inputFileName;
//	return GetCombinedRiemannFileName(addon, cut);
//}

std::string PndFileNameCreator::GetCustomFileName(std::string ext, std::string addon, bool cut)
{
	std::cout << "Ext: " << ext << " addon: " << addon << " cut: " << cut;
	std::string result = TruncateFileName(cut);
	std::cout << " Truncated: " << result << std::endl;
	if (addon.size() > 0){
		result += "_";
		result += addon;
	}
	if (ext.size() > 0){
		result += "_";
		result += ext;
	}
	result += ".root";
  if(fVerbose>0) std::cout<<" -I- PndFileNameCreator file: "<<result.c_str()<<std::endl;
	return result;
}

//std::string PndFileNameCreator::GetCustomFileName(std::string inputFileName, std::string ext, std::string addon, bool cut)
//{
//	fFileName = inputFileName;
//	return GetCustomFileName(ext, addon, cut);
//}

std::string PndFileNameCreator::GetCustomFileNameInitial(std::string ext)
{
	std::string result = ext;
	result += TruncateInitial();

	if (fVerbose>0) std::cout << " -I- PndFileNameCreator::GetCustomFileNameInitial: " << result.c_str() << std::endl;
	return result;
}

std::string PndFileNameCreator::GetPath()
{
	std::stringstream result;
	std::vector<std::string> resString;

	PndStringSeparator pathAna(fFileName,"/");
	resString = pathAna.GetStringVector();
	if(fVerbose>1) pathAna.Print();
	if (fFileName.find("/") == 0)
		result << "./";
	for (UInt_t i = 0; i < resString.size()-1; i++){
		result << resString[i] << "/";
	}
	return result.str();
}

std::string PndFileNameCreator::GetFileName()
{
	std::stringstream result;
	std::vector<std::string> resString;

	PndStringSeparator pathAna(fFileName,"/");
	resString = pathAna.GetStringVector();

	return resString[resString.size()-1];
}

std::string PndFileNameCreator::TruncateFileName(bool cut)
{
	std::vector<std::string> resString;
	std::stringstream result;
	std::string path, name;
	Int_t cutLast = 0;
  

	path = GetPath();
	name = GetFileName();
	//if(fVerbose>1) std::cout << "Path: " << path << " FileName: " << name << std::endl;
	result.str("");
  
	PndStringSeparator stringAna(name, "._");
	resString = stringAna.GetStringVector();
	//if(fVerbose>1) stringAna.Print();
  
	if (cut == true)
		cutLast = 2;
	else cutLast = 1;
  
	if (resString[resString.size()-1] != "root")
		cutLast--;
  
	if (resString.size() - cutLast <= 0)
		return "";
  
	result << path;
	for (UInt_t i = 0; i < resString.size()-1 - cutLast; i++){
		result << resString[i] << "_";
	}
	result << resString[resString.size()-1 - cutLast];
	return result.str();
}

std::string PndFileNameCreator::TruncateInitial()
{
	std::vector<std::string> resString;
	std::stringstream result;
	std::string path, name;

	path = GetPath();
	name = GetFileName();

	PndStringSeparator stringAna(name, "._");
	resString = stringAna.GetStringVector();

	for (UInt_t i = 1; i < resString.size() - 1; i++){
		result << "_" << resString[i];
	}
	result << ".root";
	return result.str();
}
ClassImp(PndFileNameCreator);

