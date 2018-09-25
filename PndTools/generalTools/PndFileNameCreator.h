/// PndFileNameCreator.h
/// @author Tobias Stockmanns <t.stockmanns@fz-juelich.de>
/// @brief A simple class which adds the corresponding file extensions to a given base class
///
/// PndFileNameCreator takes the simulation filename and creates all the other filenames of a simulation chain
/// by adding an extension to the simulation filename. Example: <SimulationFileName>.root --> <SimulationFileName>_<extension>.root.
/// The Extension is predefined for the different simulation stages but a custom extension can be given in the method:
/// GetCustomFileName.
/// If the cut parameter is set to true an existing extension of the given fileName is replaced by a new extension.
/// Example: <SimulationFileName>_<OldExtension>.root --> <SimulationFileName>_<NewExtension>.root. An extension is defined
/// as the last part of a FileName which is separated by a "_" from the rest of the FileName.

#ifndef PNDFILENAMECREATOR_H
#define PNDFILENAMECREATOR_H

#include <vector>
#include <string>

#include "TObject.h"

class PndFileNameCreator : public TObject 
  {
  public:
    PndFileNameCreator();
    PndFileNameCreator(std::string fileName);
    
    void SetFileName(std::string fileName){fFileName = fileName;};
    void SetVerbose(Int_t v) {fVerbose = v;};
    std::string GetFileName() const {return fFileName;};
    
    std::string GetParFileName(std::string addon = "", bool cut = false);
    std::string GetSimFileName(std::string addon = "", bool cut = false);
    std::string GetDigiFileName(std::string addon = "", bool cut = false);
    std::string GetRecoFileName(std::string addon = "", bool cut = false);
    std::string GetTrackFindingFileName(std::string addon = "", bool cut = false);
    std::string GetRiemannFileName(std::string addon = "", bool cut = false);
    std::string GetCombinedRiemannFileName(std::string addon = "", bool cut = false);
    
    std::string GetIdealTrackFindingFileName(std::string addon = "", bool cut = false);
    std::string GetKalmanFileName(std::string addon = "", bool cut = false);
    std::string GetVertexFileName(std::string addon = "", bool cut = false);
    
//    std::string GetParFileName(std::string inputFileName, std::string addon = "", bool cut = false);
//    std::string GetSimFileName(std::string inputFileName, std::string addon = "", bool cut = false);
//    std::string GetDigiFileName(std::string inputFileName, std::string addon = "", bool cut = false);
//    std::string GetRecoFileName(std::string inputFileName, std::string addon = "", bool cut = false);
//    std::string GetTrackFindingFileName(std::string inputFileName, std::string addon = "", bool cut = false);
//    std::string GetRiemannFileName(std::string inputFileName, std::string addon = "", bool cut = false);
//    std::string GetCombinedRiemannFileName(std::string inputFileName, std::string addon = "", bool cut = false);
//    std::string GetIdealTrackFindingFileName(std::string inputFileName, std::string addon = "", bool cut = false);
//    std::string GetKalmanFileName(std::string inputFileName, std::string addon = "", bool cut = false);
//    std::string GetVertexFileName(std::string inputFileName, std::string addon = "", bool cut = false);
    
    std::string GetCustomFileName(std::string ext, std::string addon = "", bool cut = false);
//    std::string GetCustomFileName(std::string inputFileName, std::string ext, std::string addon = "", bool cut = false);
    
    std::string GetCustomFileNameInitial(std::string ext);

    std::string GetPath();
    std::string GetFileName();

  private:
    std::string fFileName;
    std::string fExtPar;
    std::string fExtSim;
    std::string fExtDigi;
    std::string fExtReco;
    std::string fExtTrackF;
    std::string fExtIdealTrackF;
    std::string fExtRiemann;
    std::string fExtCombinedRiemann;
    std::string fExtKalman;
    std::string fExtVertex;
    
    std::string TruncateFileName(bool cut);
    std::string TruncateInitial();
    Int_t fVerbose;
    ClassDef(PndFileNameCreator, 2);
  };

#endif
