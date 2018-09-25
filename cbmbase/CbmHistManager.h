/**
 * \file CbmHistManager.h
 * \brief Histogram manager.
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2011
 */

#ifndef CBMHISTMANAGER_H_
#define CBMHISTMANAGER_H_

#include "TObject.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <cassert>

class TFile;
class TNamed;
class TH1;
class TH2;
class TGraph;
class TGraph2D;
class TProfile;
class TProfile2D;

using std::map;
using std::make_pair;
using std::string;
using std::vector;

/**
 * \class CbmHistManager
 * \brief Histogram manager.
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2011
 */
class CbmHistManager : public TObject
{
public:

   /**
    * \brief Constructor.
    */
   CbmHistManager();

   /**
    * \brief Destructor.
    */
   virtual ~CbmHistManager();

   /**
    * \brief Add new named object to manager.
    * \param[in] name Name of the object.
    * \param[in] object Pointer to object.
    */
   void Add(
         const string& name,
         TNamed* object) {
     //fMap.insert(std::make_pair<string, TNamed*>(name, object));
   }

   /**
    * \brief Helper function for creation of 1-dimensional histograms and profiles.
    * Template argument is a real object type that has to be created, for example,
    * Create1<TH1F>("name", "title", 100, 0, 100);
    * \param[in] name Object name.
    * \param[in] title Object title.
    * \param[in] nofBins Number of bins.
    * \param[in] minBin Low axis limit.
    * \param[in] maxBin Upper axis limit.
    */
   template<class T> void Create1(
         const string& name,
         const string& title,
         Int_t nofBins,
         Double_t minBin,
         Double_t maxBin) {
		T* h = new T(name.c_str(), title.c_str(), nofBins, minBin, maxBin);
		Add(name, h);
	}

   /**
    * \brief Helper function for creation of 2-dimensional histograms and profiles.
    * Template argument is a real object type that has to be created, for example,
    * Create2<TH2F>("name", "title", 100, 0, 100, 200, 0, 200);
    * \param[in] name Object name.
    * \param[in] title Object title.
    * \param[in] nofBinsX Number of bins for X axis.
    * \param[in] minBinX Low X axis limit.
    * \param[in] maxBinX Upper X axis limit.
    * \param[in] nofBinsY Number of bins for Y axis.
    * \param[in] minBinY Low Y axis limit.
    * \param[in] maxBinY Upper Y axis limit.
    */
   template<class T> void Create2(
         const string& name,
         const string& title,
         Int_t nofBinsX,
         Double_t minBinX,
         Double_t maxBinX,
         Int_t nofBinsY,
         Double_t minBinY,
         Double_t maxBinY) {
   	T* h = new T(name.c_str(), title.c_str(), nofBinsX, minBinX, maxBinX, nofBinsY, minBinY, maxBinY);
   	Add(name, h);
   }

   /**
    * \brief Return pointer to TH1 histogram.
    * \param[in] name Name of histogram.
    * \return pointer to TH1 histogram.
    */
   TH1* H1(
         const string& name) const {
      if (fMap.count(name) == 0) { // Temporarily used for debugging
    	  std::cout << "Error: CbmHistManager::H1(name): name=" << name << std::endl;
      }
      assert(fMap.count(name) != 0);
      return (TH1*) fMap.find(name)->second;
   }

   /**
    * \brief Return vector of pointers to TH1 histogram.
    * \param[in] pattern Regex for histogram name.
    * \return Vector of pointers to TH1 histogram.
    */
   vector<TH1*> H1Vector(
         const string& pattern) const;

   /**
    * \brief Return pointer to TH2 histogram.
    * \param[in] name Name of histogram.
    * \return pointer to TH1 histogram.
    */
   TH2* H2(
         const string& name) const {
      if (fMap.count(name) == 0) { // Temporarily used for debugging
    	  std::cout << "Error: CbmHistManager::H2(name): name=" << name << std::endl;
      }
      assert(fMap.count(name) != 0);
      return (TH2*) fMap.find(name)->second;
   }

   /**
    * \brief Return vector of pointers to TH2 histogram.
    * \param[in] pattern Regex for histogram name.
    * \return Vector of pointers to TH2 histogram.
    */
   vector<TH2*> H2Vector(
         const string& pattern) const;

   /**
    * \brief Return pointer to TGraph.
    * \param[in] name Name of graph.
    * \return pointer to TGraph.
    */
   TGraph* G1(
         const string& name) const {
      if (fMap.count(name) == 0) { // Temporarily used for debugging
    	  std::cout << "Error: CbmHistManager::G1(name): name=" << name << std::endl;
      }
      assert(fMap.count(name) != 0);
      return (TGraph*) fMap.find(name)->second;
   }

   /**
    * \brief Return vector of pointers to TGraph.
    * \param[in] pattern Regex for object name.
    * \return Vector of pointers to TGraph.
    */
   vector<TGraph*> G1Vector(
         const string& pattern) const;

   /**
    * \brief Return pointer to TGraph2D.
    * \param[in] name Name of graph.
    * \return pointer to TGraph.
    */
   TGraph2D* G2(
         const string& name) const {
      if (fMap.count(name) == 0) { // Temporarily used for debugging
    	  std::cout << "Error: CbmHistManager::G2(name): name=" << name << std::endl;
      }
      assert(fMap.count(name) != 0);
      return (TGraph2D*) fMap.find(name)->second;
   }

   /**
    * \brief Return vector of pointers to TGraph2D.
    * \param[in] pattern Regex for object name.
    * \return Vector of pointers to TGraph2D.
    */
   vector<TGraph2D*> G2Vector(
         const string& pattern) const;

   /**
    * \brief Return pointer to TProfile.
    * \param[in] name Name of profile.
    * \return pointer to TProfile.
    */
   TProfile* P1(
         const string& name) const {
      if (fMap.count(name) == 0) { // Temporarily used for debugging
        std::cout << "Error: CbmHistManager::P1(name): name=" << name << std::endl;
      }
      assert(fMap.count(name) != 0);
      return (TProfile*) fMap.find(name)->second;
   }

   /**
    * \brief Return vector of pointers to TProfile.
    * \param[in] pattern Regex for profile name.
    * \return Vector of pointers to TProfile.
    */
   vector<TProfile*> P1Vector(
         const string& pattern) const;

   /**
    * \brief Return pointer to TH2 histogram.
    * \param[in] name Name of histogram.
    * \return pointer to TH1 histogram.
    */
   TProfile2D* P2(
         const string& name) const {
      if (fMap.count(name) == 0) { // Temporarily used for debugging
        std::cout << "Error: CbmHistManager::P2(name): name=" << name << std::endl;
      }
      assert(fMap.count(name) != 0);
      return (TProfile2D*) fMap.find(name)->second;
   }

   /**
    * \brief Return vector of pointers to TProfile2D.
    * \param[in] pattern Regex for profile name.
    * \return Vector of pointers to TProfile2D.
    */
   vector<TProfile2D*> P2Vector(
         const string& pattern) const;

   /**
    * \brief Check existence of histogram in manager.
    * \param[in] name Name of histogram.
    * \return True if histogram exists in manager.
    */
   Bool_t Exists(
         const string& name) const {
      return (fMap.count(name) == 0) ? false : true;
   }

   /**
    * \brief Write all histograms to current opened file.
    */
   void WriteToFile();

   /**
    * \brief Read histograms from file.
    * \param[in] file Pointer to file with histograms.
    */
   void ReadFromFile(
         TFile* file);

   /**
    * \brief Clear memory. Remove all histograms.
    */
   void Clear();

   /**
    * \brief Shrink empty bins in histogram for Xaxis from right side.
    * \param[in] histName Name of histogram.
    */
   void ShrinkEmptyBins(
         const string& histName);

   /**
    * \brief Shrink empty bins in histograms for Xaxis from right side.
    * \param[in] histPatternName Regular expression for histogram name.
    */
   void ShrinkEmptyBinsByPattern(
         const string& pattern);

   /**
    * \brief Scale histogram.
    * \param[in] histName Name of histogram.
    * \param[in] scale Scaling factor.
    */
   void Scale(
         const string& histName,
         Double_t scale);

   /**
    * \brief Scale histograms which name matches specified pattern.
    * \param[in] histPatternName Regular expression for histogram name.
    * \param[in] scale Scaling factor.
    */
   void ScaleByPattern(
         const string& pattern,
         Double_t scale);

   /**
    * \brief Rebin histogram.
    * \param[in] histName Name of histogram.
    * \param[in] ngroup Rebining factor.
    */
   void Rebin(
         const string& histName,
         Int_t ngroup);

   /**
    * \brief Rebin histograms which name matches specified pattern.
    * \param[in] histPatternName Regular expression for histogram name.
    * \param[in] ngroup Rebining factor.
    */
   void RebinByPattern(
         const string& pattern,
         Int_t ngroup);

   /**
    * \brief Return string representation of class.
    * \return string representation of class.
    */
   string ToString() const;

   /**
    * \brief Operator << for convenient output to std::ostream.
    * \return Insertion stream in order to be able to call a succession of insertion operations.
    */
   friend std::ostream& operator<<(std::ostream& strm, const CbmHistManager& histManager) {
      strm << histManager.ToString();
      return strm;
   }

private:
   template<class T> vector<T> ObjectVector(
         const string& pattern) const;

   // Map of histogram (graph) name to its pointer
   map<string, TNamed*> fMap;

   ClassDef(CbmHistManager, 1)
};

#endif /* CBMHISTMANAGER_H_ */
