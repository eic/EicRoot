
void analysis()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");  

  // Input simulated & reconstructed files;
  TFile *ff = new TFile("simulation.root");
  TTree *cbmsim = ff->Get("cbmsim"); 
  cbmsim->AddFriend("cbmsim", "reconstruction.root");

  // Build 1D dE/E histogram;
  TH1D *de = new TH1D("de", "de", 100, -200., 200.);
  cbmsim->Project("de", "100.*(FhacClusterGroup.mEnergy-MCTrack.fE)/MCTrack.fE", 
		  "MCTrack.fMotherID==-1");
  de->Fit("gaus");
} // analysis()
