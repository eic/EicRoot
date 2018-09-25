//
//  Implementation of a low Q^2 tagger
//    using the new LQST detector classes
//
//  This script constructs the detector and saves it to a ROOT file
//
//  Written by R. Petti (10-23-2014)
//


lowq2tagger()
{

  // Load basic libraries
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");


  //
  // hard code parameters for now, but will improve shortly down the line
 
  const UInt_t cellNumx            = 6;
  const UInt_t cellNumy            = 4;
  const Double_t cellFaceSize      = 50.00;   //mm  5cm x 5cm sensor size
  const Double_t cellLength        = 0.04;   //mm  -> 400 micron thickness
  const Double_t detectorLength    = cellNumx*cellFaceSize;

  const Double_t beamLineOffset    = -15000.;  //mm  -> 15.4 m
  const Double_t nominalX          = 230.;  //mm -> 23 cm
  const Double_t desiredEdgeDistanceToBeam = 20.;  //mm -> 2cm
  const Double_t displacementInX = nominalX + detectorLength/2. + desiredEdgeDistanceToBeam; //mm   // 23 cm is where nominal beam hits, add some distance to that
  const Double_t angle = -30.e-3;  // radians

  std::cout << "displacementInX = " << displacementInX << std::endl;

  // Detector name will be "lowQtag"; should be consistent through whole chain
  LqstGeoParData *lqs = new LqstGeoParData("LQST");  
  lqs->InitializeDetector(lqs, 2, cellNumx, cellNumy, 0.1*cellFaceSize, 0.1*cellLength, 0.1*beamLineOffset, 0.1*displacementInX, angle);
  lqs->set_ecalTowerLength(0.1*100);  // in [cm]
  lqs->constructDetector();
  //  lqs = NULL;
  /*
  LqstGeoParData *lqs = new LqstGeoParData("LQST2");  
  lqs->InitializeDetector(lqs, cellNumx, cellNumy, 0.1*cellFaceSize, 0.1*cellLength, 0.1*beamLineOffset, 0.1*displacementInX, angle);
  lqs->constructDetector();
  */
  
  exit(0);


}  // lowq2tagger()
