//
// AYK (ayk@bnl.gov), 2014/07/24
//
//  A simple class to store and retrieve information about name pattern matching;
//

#include <TObject.h>
#include <TString.h>

#ifndef _EIC_NAME_PATTERN_HUB_
#define _EIC_NAME_PATTERN_HUB_

template <typename T>
class EicNamePatternHub: public TObject {
 public:
  EicNamePatternHub() {};
  ~EicNamePatternHub() {
    fExactMatch.clear();
    fPrefixMatch.clear();
    fSuffixMatch.clear();
    fPatternMatch.clear();
  };

  // Well, really want to have default value for the 2-d parameter; 
  // let it be 0, type-matched to T;
  void AddExactMatch (const char *match, T value = (T)0) { 
    fExactMatch.push_back (std::pair<TString, T>(TString(match), value)); 
  };

  void AddPrefixMatch(const char *match, T value = (T)0) { 
    fPrefixMatch.push_back(std::pair<TString, T>(TString(match), value)); 
  };

  void AddSuffixMatch(const char *match, T value = (T)0) { 
    fSuffixMatch.push_back(std::pair<TString, T>(TString(match), value)); 
  };

  void AddPatternMatch(const char *match, T value = (T)0) { 
    fPatternMatch.push_back(std::pair<TString, T>(TString(match), value)); 
  };

  const std::pair<TString, T> *ExactMatch(const char *name) const {
    for(unsigned iq=0; iq<fExactMatch.size(); iq++) 
      if (fExactMatch[iq].first.EqualTo(name))
	return &fExactMatch[iq];

    return 0;
  };

  const std::pair<TString, T> *PrefixMatch(const char *name) const {
    for(unsigned iq=0; iq<fPrefixMatch.size(); iq++) 
      if (TString(name).BeginsWith(fPrefixMatch[iq].first))
	return &fPrefixMatch[iq];

    return 0;
  };

  const std::pair<TString, T> *SuffixMatch(const char *name) const {
    for(unsigned iq=0; iq<fSuffixMatch.size(); iq++) 
      if (TString(name).EndsWith(fSuffixMatch[iq].first))
	return &fSuffixMatch[iq];

    return 0;
  };

  const std::pair<TString, T> *PatternMatch(const char *name) const {
    for(unsigned iq=0; iq<fPatternMatch.size(); iq++) 
      if (TString(name).Contains(fPatternMatch[iq].first))
	return &fPatternMatch[iq];

    return 0;
  };

  const std::pair<TString, T> *AnyMatch(const char *name) const { 
    // Try exact match first;
    {
      const std::pair<TString, T> *ret = ExactMatch(name);
      if (ret) return ret;
    }

    // If failed, try prefix match;
    {
      const std::pair<TString, T> *ret = PrefixMatch(name);
      if (ret) return ret;
    }

    // If failed, try suffix match;
    {
      const std::pair<TString, T> *ret = SuffixMatch(name);
      if (ret) return ret;
    }

    // Otherwise return pattern match check result;
    return PatternMatch(name); 
  };

  const bool IsEmpty() const { 
    return !fExactMatch.size() && !fPrefixMatch.size() && !fSuffixMatch.size() && !fPatternMatch.size(); 
  };

 private:
  // FIXME: think about efficiency later; for my purposes overhead is negligible 
  // though (called during init phase only) -> std::vector is fine; 
  // NB: it is sometimes handy to store extra info associated with the pattern;
  std::vector< std::pair<TString, T> > fExactMatch;
  std::vector< std::pair<TString, T> > fPrefixMatch;
  std::vector< std::pair<TString, T> > fSuffixMatch;
  std::vector< std::pair<TString, T> > fPatternMatch;
    
  ClassDef(EicNamePatternHub<T>,7) 
};

#endif
