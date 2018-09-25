#ifndef PNDGENFITADAPTERS_H
#define PNDGENFITADAPTERS_H

class PndTrack;
class PndTrackCand;
class GFTrack;
class GFAbsTrackRep;
class GFTrackCand;

PndTrackCand* GenfitTrackCand2PndTrackCand(const GFTrackCand*);
GFTrackCand* PndTrackCand2GenfitTrackCand(PndTrackCand*);
PndTrack* GenfitTrack2PndTrack(const GFTrack*);

#endif
