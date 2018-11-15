
void analysis()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");  

  // Input simulated & reconstructed files;
  TFile *ff = new TFile("simulation.root");
  TTree *cbmsim = ff->Get("cbmsim"); 
  cbmsim->AddFriend("cbmsim", "reconstruction.root");

  // Branches of interest;
  TClonesArray *mcTrackArray = new TClonesArray("PndMCTrack");
  cbmsim->SetBranchAddress("MCTrack", &mcTrackArray);
  TClonesArray *rcTrackArray = new TClonesArray("PndPidCandidate");
  cbmsim->SetBranchAddress("PidChargedCand", &rcTrackArray);

  // Book 1D dp/p histogram;
  TH1D *dp = new TH1D("dp", "dp", 100, -20., 20.);

  // Loop through all events; NB: for box-generated events without secondaries 
  // could simply use cbmsim->Project() as well; in general EicEventAssembler 
  // should be used for "true" physics events;
  int nEvents = cbmsim->GetEntries();
  for(unsigned ev=0; ev<nEvents; ev++) {
    cbmsim->GetEntry(ev);

    // Loop through all reconstructed tracks;
    for(unsigned rc=0; rc<rcTrackArray->GetEntriesFast(); rc++) {
      PndPidCandidate *rctrack = rcTrackArray->At(rc);
      {
	for(unsigned iq=0; iq<rctrack->GetSmoothedValuesCount(); iq++) {
	  const TVector3 &pos = rctrack->GetSmoothedPosition(iq);
	  const TVector3 &mom = rctrack->GetSmoothedMomentum(iq);
	  
	  printf("%10.4f %10.4f %10.4f -> %10.4f %10.4f %10.4f\n", 
	  	 pos.X(), pos.Y(), pos.Z(), mom.X(), mom.Y(), mom.Z());
	} //for iq
      }

      int mcTrackId = rctrack->GetMcIndex();

      // Select only correctly rc->mc identified tracks;
      if (mcTrackId < 0 || mcTrackId >= mcTrackArray->GetEntriesFast()) continue;

      // Find MC track associated with this reconstructed track;
      PndMCTrack *mctrack = mcTrackArray->At(mcTrackId);

      // Well, for plotting select primary pi-minus tracks only (see simulation.C);
      if (mctrack->GetPdgCode() == 211 && mctrack->GetMotherID() == -1)
	dp->Fill(100.*(rctrack->GetMomentum().Mag() - mctrack->GetMomentum().Mag())/mctrack->GetMomentum().Mag());
    } //for rc
  } //for ev

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
} // analysis()
