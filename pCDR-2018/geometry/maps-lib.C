
//
//  Crappy, but perhaps the easiest;
//

void DefinitionParser(void *ptr)
{
  MapsGeoParData *maps = (MapsGeoParData*)ptr;

  // Simpliest case wins;
#ifdef _BEAM_LINE_UNIFORM_GEOMETRY_
  maps->SetGeometryType(EicGeoParData::SimpleStructure);
#endif
#ifdef _NO_STRUCTURE_GEOMETRY_
  maps->SetGeometryType(EicGeoParData::NoStructure);
#endif

#ifdef _USE_TRIANGULAR_ASSEMBLIES_
  maps->UseTriangularAssemblies(true);
#endif

#ifdef _TEST_VERSION_
  maps->SetTestGeometryFlag();
#endif

  // Let MapsGeoParData base class instance know, which elements are wanted;
  // this set of #define and respective call looks strange, but visually it 
  // is more convenient to check #define statements at the beginning of the 
  // script rather than to check several lines with functional calls in the text;
#ifdef _WITH_MOUNTING_RINGS_
  maps->WithMountingRings(true);
#endif
#ifdef _WITH_ENFORCEMENT_BRACKETS_
  maps->WithEnforcementBrackets(true);
#endif
#ifdef _WITH_EXTERNAL_PIPES_
  maps->WithExternalPipes(true);
#endif
} // DefinitionParser()


void SetMapsColors(void *ptr)
{
  EicNamePatternHub<Color_t> *ctable = ((EicGeoParData*)ptr)->GetColorTable();

  // Specify color preferences; NB: order of calls matters!;
  ctable->AddPatternMatch("WaterPipe",      kYellow);
  ctable->AddPatternMatch("Water",          kBlue);
  ctable->AddPatternMatch("StaveBracket",   kOrange);
  ctable->AddPatternMatch("Beam",           kBlack);
  ctable->AddPatternMatch("ColdPlate",      kYellow);
  ctable->AddPatternMatch("MimosaCore",     kYellow);
  ctable->AddPatternMatch("CellFlexLayer",  kGreen+2);
  ctable->AddPatternMatch("AluStrips",      kGray);
  ctable->AddPatternMatch("MountingRing",   kMagenta+1);
} // SetMapsColors()


void *ConfigureAliceCell()
{
  // First want to cook the basic building block - Mimosa chip assembly; the
  // rest is composed out of these blocks in a LEGO fashion;
  MapsMimosaAssembly *ibcell = new MapsMimosaAssembly();
  
  //  !!!!!!
  //
  // NB: these parameters were carefully tuned to match ALICE ITS upgrade design;
  //     do NOT touch unless you understand what you are doing;
  //
  //  !!!!!!

  // Air container volume parameters sufficient to pack all the stuff;
  ibcell->mAssemblyBaseWidth           =   17.500;
  ibcell->mAssemblySideSlope           =   30.000;
  ibcell->mChipToChipGap               =    0.100;

  // Space structure;
  ibcell->mApexEnforcementBeamDiameter =    0.400;
  ibcell->mEnforcementStripWidth       =    0.500;
  ibcell->mEnforcementStripThickness   =    0.200;
  ibcell->mBaseEnforcementBeamWidth    =    1.200;
  ibcell->mSideWallThickness           =    0.050;

  // Basic Mimosa 34 chip parameters; pixel size does not matter here (will play 
  // a role during digitization only);
  ibcell->mChipLength                  =   30.000;
  ibcell->mChipWidth                   =   15.000;
  // Well, ALICE rad.length scan plot indicates, that it is equal to the chip width;
  // NB: do NOT try to set this value to be larger than mChipWidth (unless modify
  // assembly and stove width calculation):
  ibcell->mAssemblyDeadMaterialWidth   =   15.000;
  ibcell->mChipThickness               =    0.050;
  ibcell->mChipActiveZoneThickness     =    0.018;
  ibcell->mChipDeadAreaWidth           =    2.000;

  // Layers at the base of the assembly; kapton and effective alu thinckness;
  ibcell->mFlexCableKaptonThickness    =    0.100;
  // Based on the ALICE rad.length scan one can conclude, that 50um is too much;
  // I guess these strips occupy only a fraction of the assembly width;
  ibcell->mFlexCableAluThickness       =    0.020;
  // Assume just 'Carbon fiber + paper' on p.55 of ALICE ITS TDR) -> 30+70um;
  ibcell->mColdPlateThickness          =    0.100;

  // Water pipes; assume 2 parallel pipes; 25um thick walls;
  ibcell->mWaterPipeInnerDiameter      =    1.024;
  ibcell->mWaterPipeWallThickness      =    0.025;

  return ibcell;
} // ConfigureAliceCell()
