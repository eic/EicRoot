// -------------------------------------------------------------------------
// -----                  PndDetectorList.header file                  -----
// -----                 Created 11/02/09  by M. Al-Turany                -----
// -------------------------------------------------------------------------


/** Unique identifier for all Panda detector systems **/

#ifndef PNDDETECTORLIST_H
#define PNDDETECTORLIST_H 1

enum DetectorId {
/** kRICH must be the 1st id, and kHYP must be the last one. Please put new detectors in between!! **/
  kRICH,kDRC,kDSK,kEMC,kGEM,kLUMI,kMDT,kMVD,kRPC,kSTT,kFTOF,kTOF,kFTS,kHYPG,kHYP};
/** Beware! each new detector should be added also in PndMCTrack **/
    
/** Unique identifier for all Panda Point and Hit types **/

enum fDetectorType {  
  // A newer ROOT versions have kUnknown=1 defined somewhere; be creative;
   kCompletelyUnknown, kMCTrack,
   kMVDPoint, kMVDDigiStrip, kMVDDigiPixel, kMVDClusterPixel, kMVDClusterStrip, kMVDHitsStrip, kMVDHitsPixel,
   kSttPoint, kSttHit, kSttHelixHit, kSttTrackCand, kSttTrack,
   kGemPoint, kGemDigi, kGemHit,
   kFtsPoint, kFtsDigi, kFtsHit,
   kMdtPoint, kMdtHit, kMdtTrack,
   kEmcPoint, kEmcHit, kEmcDigi, kEmcCluster, kEmcBump, kEmcRecoHit,
   kLheTrack,
   kTrackCand, kTrack,
   kPidChargedCandidate, kPidNeutralCandidate
};



enum SensorSide { kTOP, kBOTTOM };

#endif
