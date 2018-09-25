
#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;


#pragma link C++ class EicNamePatternHub<unsigned>+;
#pragma link C++ class EicNamePatternHub<double>+;
#pragma link C++ class EicNamePatternHub<SteppingType>+;
#pragma link C++ class EicNamePatternHub<Color_t>+;
#pragma link C++ class EicNamePatternHub<Char_t>+;

#pragma link C++ class EicDetector+;


#if _OFF_
#pragma link C++ class EicRunSim+;
#pragma link C++ class EicMCApplication+;
#pragma link C++ class FluxMonitorGrid+;
#pragma link C++ class EicFluxMonitorTask+;
#pragma link C++ class FluxMonitorParticleType+;
#pragma link C++ class EicRunAna+;
#pragma link C++ class EicRunDigi+;
//#pragma link C++ class EicRunReco+;

#pragma link C++ class EicDummyDetector+;
#pragma link C++ class EicFieldMapDetector+;
#pragma link C++ class EicFieldGradDetector+;

//#pragma link C++ class EicNamePatternHub+;
#pragma link C++ class std::pair<TString,SteppingType>+;
#pragma link C++ class std::pair<TString,unsigned>+;

#pragma link C++ class std::pair<TString,Color_t>+;
#pragma link C++ class std::pair<TString,Char_t>+;

#pragma link C++ class EicMoCaPoint+;
#pragma link C++ class EicFakeMoCaPoint+;
#pragma link C++ class EicFakeMoCaPointGenerator+;
#pragma link C++ class EicFakeMoCaPointDatabase+;
#pragma link C++ class EicFakeMoCaPointDbEntry+;
#pragma link C++ class EicFakeMoCaPointDbHit+;
#pragma link C++ class EicGeo+;
#pragma link C++ class EicContFact+;
#pragma link C++ class EicGeoPar+;

#pragma link C++ class EicDigiParData+;

#pragma link C++ class EicFileNameExpansion+;
#endif


#pragma link C++ class EicDetName+;

#pragma link C++ class EicBitMask<UGeantIndex_t>+;
#pragma link C++ class EicBitMask<ULogicalIndex_t>+;
// This should in principle work as well?;
//#pragma link C++ class EicBitMask*+;
#pragma link C++ class GeantVolumeLevel+;
#pragma link C++ class EicGeoMap+;

#pragma link C++ class EicGeoParData+;

#pragma link C++ class SourceFile+;

#pragma link C++ class LogicalVolumeGroupProjection+;
#pragma link C++ class LogicalVolumeLookupTableEntry+;
#pragma link C++ class LogicalVolumeGroup+;

#pragma link C++ class std::pair<TString, Int_t>+;

#if _OFF_
#pragma link C++ class std::pair<ULong64_t,ULong64_t>+;

#pragma link C++ class EicDigiHitProducer+;

//#pragma link C++ class EicFlatDisEventGenerator+;

#pragma link C++ class EicEventManager+;

#pragma link C++ class EicBoxGenerator+;
#pragma link C++ class EicAsciiBoxGenerator+;

#pragma link C++ enum EicDetectorId;
#endif

#pragma link C++ enum SteppingType;

#if _OFF_
#pragma link C++ class EicEnergyMonitor+;

#pragma link C++ class std::vector< std::vector<int> >+;
#pragma link C++ class std::vector<int>+;
#pragma link C++ class std::vector< std::vector<float> >+;
#endif

#pragma link C++ namespace eic;

#endif

