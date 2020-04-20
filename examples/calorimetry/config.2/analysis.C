
void analysis()
{
  // Input simulated & reconstructed files;
  auto ff = new TFile("simulation.root");
  auto cbmsim = dynamic_cast<TTree *>(ff->Get("cbmsim")); 
  cbmsim->AddFriend("cbmsim", "reconstruction.root");

  // Build 1D dE/E histogram;
  auto de = new TH1D("de", "de", 100, -200., 200.);
  cbmsim->Project("de", "100.*(FhacClusterGroup.mEnergy-MCTrack.fE)/MCTrack.fE", 
		  "MCTrack.fMotherID==-1");
  de->Fit("gaus");
} // analysis()
