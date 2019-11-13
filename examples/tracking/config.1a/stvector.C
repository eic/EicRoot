
void stvector()
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

  // Loop through all events; NB: for box-generated events without secondaries 
  // could simply use cbmsim->Project() as well; in general EicEventAssembler 
  // should be used for "true" physics events;
  int nEvents = cbmsim->GetEntries();
  for(unsigned ev=0; ev<nEvents; ev++) {
    cbmsim->GetEntry(ev);

    // Loop through all reconstructed tracks;
    for(unsigned rc=0; rc<rcTrackArray->GetEntriesFast(); rc++) {
      PndPidCandidate *rctrack = rcTrackArray->At(rc);

      for(unsigned ih=0; ih<rctrack->GetParameterizationCount(); ih++) {
	NaiveTrackParameterization *par = rctrack->GetParameterization(ih);
	
	TVector3 mcpos = par->GetMoCaPosition(), mcmom = par->GetMoCaMomentum();
	TVector3 rcpos = par->GetRecoPosition(), rcmom = par->GetRecoMomentum();
	//printf("%2d (REC) -> V: %7.3f %7.3f %7.3f [cm] & P: %7.3f %7.3f %7.3f [GeV/c]\n", 
	//     ih, rcpos.X(), rcpos.Y(), rcpos.Z(), rcmom.X(), rcmom.Y(), rcmom.Z());
	//printf("   ( MC)       %7.3f %7.3f %7.3f      &    %7.3f %7.3f %7.3f\n\n", 
	//       mcpos.X(), mcpos.Y(), mcpos.Z(), mcmom.X(), mcmom.Y(), mcmom.Z());
      } //for ih

      {
	TVector3 plxx(0.0, 0.0, 105.0), plnx(0.0, 0.0, 1.0);  
	NaiveTrackParameterization *par = rctrack->GetNearestParameterization(plxx, plnx);

	if (par) {
	  TVector3 mcpos = par->GetMoCaPosition(), mcmom = par->GetMoCaMomentum();
	  TVector3 rcpos = par->GetRecoPosition(), rcmom = par->GetRecoMomentum();
	  printf("Hit-to-plane distance: %7.2f [cm]\n", par->DistanceToPlane(plxx, plnx));
	  printf("   (REC) -> V: %7.3f %7.3f %7.3f [cm] & P: %7.3f %7.3f %7.3f [GeV/c]\n", 
		 rcpos.X(), rcpos.Y(), rcpos.Z(), rcmom.X(), rcmom.Y(), rcmom.Z());
	  printf("   ( MC)       %7.3f %7.3f %7.3f      &    %7.3f %7.3f %7.3f\n\n", 
		 mcpos.X(), mcpos.Y(), mcpos.Z(), mcmom.X(), mcmom.Y(), mcmom.Z());
	} 
	else
	  // Can hardly happen (fitter would not be started on such a track);
	  printf("  ---> No hits!\n");
      }
    } //for rc
  } //for ev
} // stvector()
