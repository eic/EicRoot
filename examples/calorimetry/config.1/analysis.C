
//
// A trivial analysis ROOT script sufficient to build calorimeter energy 
// resolution plot; all relationships between Monte-Carlo tracks and reconstructed 
// ones are ignored, so that this script can work for single track events only; 
// is pretty much sufficient to see sensitivity to obvious actions though:
//
//   - change crystal count and/or length in calorimeter.C;
//   - change alveole thickness in calorimeter.C;
//   - decrease distance from IP in calorimeter.C and/or change air to vacuum in cave.geo;
//   - change particle energy in simulation.C;
//   - play with light output, noise, threshold, att.length, etc in digitization.C;
//   - play with thresholds in reconstruction.C;
//

void analysis()
{
  // Input simulated & reconstructed files;
  auto ff = new TFile("simulation.root");
  auto cbmsim = dynamic_cast<TTree *>(ff->Get("cbmsim")); 
  cbmsim->AddFriend("cbmsim", "reconstruction.root");

  // Build 1D dE/E histogram;
  auto de = new TH1D("de", "de", 100, -20., 20.);
  cbmsim->Project("de", "100.*(CalorimeterClusterGroup.mEnergy-MCTrack.fE)/MCTrack.fE", 
		  "MCTrack.fMotherID==-1");
  de->Fit("gaus");
} // analysis()
