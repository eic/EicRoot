//
// AYK (ayk@bnl.gov)
//
//  Hough transform track finder for STAR-specific forward tracker;
//
//  Initial port from OLYMPUS sources: Oct'2015;
//

#include <ayk.h>
#include <3d.h>
#include <KalmanNode.h>

#include <HoughTree.h>
#include <FwdMatchCandidate.h>
#include <SensitiveVolume.h>
#include <FwdHoughNodeGroup.h>

#ifndef _FWD_HOUGH_TREE_
#define _FWD_HOUGH_TREE_

class TrKalmanNode;
class FwdTrackFinder;

class FwdHoughTree : public HoughTree
{
 public:
 FwdHoughTree(FwdTrackFinder *tf): mBzAtIP(0.0), mTrackFinder(tf), mAmbiguityResolutionViaWorstHit(true) {};

  // Get (presumably solenoidal) field Z-component at the IP;
  double GetBzAtIP();

  void MappingCall(const double par[], t_hough_range id[]);

  // Yes, want some extra fields, as well as fitting method;
  MatchCandidate *AllocateMatchCandidate() {
    return new FwdMatchCandidate(this);
  };
  //void SeparateSiamTracks(MatchCandidate *match, unsigned minHitCount, std::vector<MatchCandidate*> *newMatches);
  //void ResolveAmbiguities(MatchCandidate *match);
  void ResolveAmbiguitiesNg(MatchCandidate *match);
  void FinalFit(MatchCandidate *match);
  TrKalmanNode *GetKfNode(MatchCandidate *match, unsigned gr, unsigned mm);

  HoughNodeGroup *AllocateNodeGroup(unsigned id) { return new FwdHoughNodeGroup(id); };

  FwdHoughNodeGroup *AddNodeGroup(TrKalmanNodeLocation *location, 
			       unsigned id, unsigned cdim, const double min[], const double max[],
			       const double gra[]);
  FwdHoughNodeGroup *AddNodeGroup(TrKalmanNodeLocation *location, unsigned tmpl, bool cylindricalPreference,
				  const std::set<double> &xMin, const std::set<double> &xMax, 
				  const std::set<double> &yMin, const std::set<double> &yMax, 
				  const std::set<double> &rMin, const std::set<double> &rMax);

  FwdHoughNodeGroup *GetNodeGroup(unsigned gr) { 
    return (gr < mGroups.size() ? dynamic_cast<FwdHoughNodeGroup*>(mGroups[gr]) : 0);
  };

  void SetupTrackQualityIteration(unsigned qua);

  //void ResolveAmbiguityViaWorstHit() { mAmbiguityResolutionViaWorstHit = true; };

 private:
  // Well, STAR magnetic field is virtually constant in the [RZ] range
  // [0 .. 20cm, 0 .. 150cm] -> may clearly use global helix fit for 
  // preliminary track chi^2 evaluation and simplified fast KF fit for
  // precise track parameter determination at the end; just need to know
  // Z-component of the field; 
  double mBzAtIP;              // solenoidal field Z-component at Z=0.0 

  // Back-door pointer; FIXME: do it better;
  FwdTrackFinder *mTrackFinder; //!

  bool mAmbiguityResolutionViaWorstHit;

  // This part is shared between ResolveAmbiguity() and FinalFit();
  bool SetupKalmanFilter(MatchCandidate *match);

  ClassDef(FwdHoughTree,3);
};

#endif
