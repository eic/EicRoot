/** CbmStsTrackMatch.h
 *@author V.Friese <v.friese@gsi.de>
 *@since 07.05.2009
 **
 ** Data structure describing the matching of a reconstructed track
 ** with a Monte Carlo track on the base of corresponding hits/points.
 ** This requires matching of hits to MC points.
 **/


#ifndef CBMTRACKMATCH_H
#define CBMTRACKMATCH_H 1


#include "TObject.h"



class CbmTrackMatch : public TObject
{

 public:

  /** Default constructor **/
  CbmTrackMatch();


  /** Standard constructor 
  *@param mcTrackID   Index of matched MCTrack
  *@param nTrue       Number of true hits (belonging to matched MCTrack)
  *@param nWrong      Number of wrong Hits (from other MCTracks)
  *@param nFake       Number of fake hits (not belonging to any MCTrack)
  *@param nTracks     Number of MCTracks with common hits
  **/
  CbmTrackMatch(Int_t mcTrackID, 
		Int_t nTrue, 
		Int_t nWrong, 
		Int_t nFake, 
		Int_t nTracks);


  /** Destructor **/
  virtual ~CbmTrackMatch();


  /** Index of matched MC track **/
  Int_t GetMCTrackId() const { return fMCTrackId;    };

  /** Number of true hits on track (from matched MC track) **/
  Int_t GetNofTrueHits() const { return fNofTrueHits;  };

  /** Number of wrong hits on track (from other MC tracks) **/
  Int_t GetNofWrongHits() const { return fNofWrongHits; };

  /** Number of fake hits on track (from no MC track) **/
  Int_t GetNofFakeHits() const { return fNofFakeHits;  };

  /** Number of MCTracks with common hits **/
  Int_t GetNofMCTracks() const { return fNofMCTracks;  };


 private:

  /** Index of matched CbmMCTrack  **/
  Int_t fMCTrackId;

  /** Number of true hits (belonging to the matched MCTrack) **/
  Int_t fNofTrueHits;

  /** Number of wrong hits (belonging to other MCTracks) **/
  Int_t fNofWrongHits;

  /** Number of fake hits (belonging to no MCTrack) **/
  Int_t fNofFakeHits;

  /** Number of MCTrackx with common hits **/
  Int_t fNofMCTracks;


  ClassDef(CbmTrackMatch,1);

};


#endif
				 
