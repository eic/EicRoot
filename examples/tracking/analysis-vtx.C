
void analysis_vtx()
{
  // Import simulated & reconstructed files in a "coherent" way;
  auto ff = new TFile("simulation.root");
  auto cbmsim = dynamic_cast<TTree *>(ff->Get("cbmsim")); 
  cbmsim->AddFriend("cbmsim", "reconstruction.root");

  // Define branches of interest: simulated and reconstructed tracks;
  auto mcTrackArray = new TClonesArray("PndMCTrack");
  cbmsim->SetBranchAddress("MCTrack", &mcTrackArray);
  auto kfTrackArray = new TClonesArray("PndTrack");
  cbmsim->SetBranchAddress("EicIdealGenTrack", &kfTrackArray);
  auto rcTrackArray = new TClonesArray("PndPidCandidate");
  cbmsim->SetBranchAddress("PidChargedCand", &rcTrackArray);

  // Book 1D dp/p histogram;
  auto dp  = new TH1D("dp",  "dp",  100, -20., 20.);
  auto dfi = new TH1D("dfi", "dfi", 100, -20., 20.);
  auto dth = new TH1D("dth", "dth", 100, -20., 20.);

  // Loop through all events; NB: for box-generated events without secondaries 
  // could simply use cbmsim->Project() as well; in general EicEventAssembler 
  // in the reconstruction.C script should be used for "true" physics events
  // for multi-particle physics events;
  int nEvents = cbmsim->GetEntries();
  for(unsigned ev=0; ev<nEvents; ev++) {
    cbmsim->GetEntry(ev);

    for(unsigned rc=0; rc<kfTrackArray->GetEntriesFast(); rc++) {
      auto rctrack = dynamic_cast<PndTrack *>(kfTrackArray->At(rc));
      // Here the trick comes: GetParamFirst() is defined at the IP bubble location;
      FairTrackParP vtxpar = rctrack->GetParamFirst();
      
      // I hope this is correct?;
      int mcTrackId = rctrack->GetTrackCandPtr()->getMcTrackId();

      // Select only the correctly rc->mc identified tracks;
      if (mcTrackId < 0 || mcTrackId >= mcTrackArray->GetEntriesFast()) continue;

      // Find MC track associated with this reconstructed track;
      auto mctrack = dynamic_cast<PndMCTrack *>(mcTrackArray->At(mcTrackId));

      // Well, for plotting select primary pi- tracks only (see simulation.C);
      if (mctrack->GetPdgCode() == 211 && mctrack->GetMotherID() == -1) {
	//printf("%7.3f %7.3f -> %7.3f\n", vtxpar.GetMomentum().Phi(), mctrack->GetMomentum().Phi(), 
	//     1000*(vtxpar.GetMomentum().Phi() - mctrack->GetMomentum().Phi()));
	printf("%7.3f %7.3f -> %7.3f\n", vtxpar.GetMomentum().Theta(), mctrack->GetMomentum().Theta(), 
	       1000*(vtxpar.GetMomentum().Theta() - mctrack->GetMomentum().Theta()));
	
	// This is just a dp/p in [%] units; 
	dp->Fill(100.*(vtxpar.GetMomentum().Mag() - mctrack->GetMomentum().Mag())/mctrack->GetMomentum().Mag());
      } //if
    } //for rc
  } //for ev

  // ROOT plotting magic;
  gStyle->SetOptStat(0);

  dp->SetTitle("Momentum resolution");

  dp->GetXaxis()->SetTitle("(P_{rec} - P_{sim})/P_{sim}, [%]");
  dp->GetXaxis()->SetTitleOffset(0.9);
  dp->GetXaxis()->SetLabelFont(52);
  dp->GetXaxis()->SetLabelSize(0.040);
  dp->GetXaxis()->SetTitleFont(52);
  dp->GetXaxis()->SetTitleSize(0.050);

  dp->GetYaxis()->SetTitle("Events");
  dp->GetYaxis()->SetTitleOffset(0.7);
  dp->GetYaxis()->SetLabelFont(52);
  dp->GetYaxis()->SetLabelSize(0.040);
  dp->GetYaxis()->SetTitleFont(52);
  dp->GetYaxis()->SetTitleSize(0.050);

  dp->Fit("gaus");
} // analysis_vtx()
