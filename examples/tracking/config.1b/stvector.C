
//
// State vector extraction script; allows one to get access to the MC
// (truth) and reconstructed track parameterizations at the locations of 
// detector planes where a given track produced hits;
//

void stvector()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");  

  // Import simulated & reconstructed files in a "coherent" way;
  TFile *ff = new TFile("simulation.root");
  TTree *cbmsim = ff->Get("cbmsim"); 
  cbmsim->AddFriend("cbmsim", "reconstruction.root");

  // Define branches of interest: simulated and reconstructed tracks;
  TClonesArray *mcTrackArray = new TClonesArray("PndMCTrack");
  cbmsim->SetBranchAddress("MCTrack", &mcTrackArray);
  TClonesArray *rcTrackArray = new TClonesArray("PndPidCandidate");
  cbmsim->SetBranchAddress("PidChargedCand", &rcTrackArray);

  // Loop through all events; NB: for box-generated events without secondaries 
  // could simply use cbmsim->Project() as well; in general EicEventAssembler 
  // in the reconstruction.C script should be used for "true" physics events
  // for multi-particle physics events;
  int nEvents = cbmsim->GetEntries();
  for(unsigned ev=0; ev<nEvents; ev++) {
    cbmsim->GetEntry(ev);

    // Loop through all reconstructed tracks;
    for(unsigned rc=0; rc<rcTrackArray->GetEntriesFast(); rc++) {
      PndPidCandidate *rctrack = rcTrackArray->At(rc);

      // Loop through ALL the stored parameterizations and print out MC (Monte-Carlo) 
      // and RC (reconstructed) {x,p} pairs of 3D vectors (position and momentum);   
      for(unsigned ih=0; ih<rctrack->GetParameterizationCount(); ih++) {
	NaiveTrackParameterization *par = rctrack->GetParameterization(ih);
	
	TVector3 mcpos = par->GetMoCaPosition(), mcmom = par->GetMoCaMomentum();
	TVector3 rcpos = par->GetRecoPosition(), rcmom = par->GetRecoMomentum();
	//printf("%2d (REC) -> V: %7.3f %7.3f %7.3f [cm] & P: %7.3f %7.3f %7.3f [GeV/c]\n", 
	//     ih, rcpos.X(), rcpos.Y(), rcpos.Z(), rcmom.X(), rcmom.Y(), rcmom.Z());
	//printf("   ( MC)       %7.3f %7.3f %7.3f      &    %7.3f %7.3f %7.3f\n\n", 
	//       mcpos.X(), mcpos.Y(), mcpos.Z(), mcmom.X(), mcmom.Y(), mcmom.Z());
      } //for ih

      // Find a parameterization the closest to a given Z-position along the beam line
      // (which is in this case 105cm - silicon plane #5 counting from 0, see tracker.C
      // script with the geometry description); if a given track did NOT produce a hit 
      // in this particular plane (missed it, in other words), the below sequence of 
      // calls will find some other closest hit parameterization, so one needs to watch
      // out and place a tight enough cut either on the mcpos.Z() value explicitly or 
      // on the par->DistanceToPlane(plxx, plnx) return value);
      {
	// A 3D plane parameterization in space - a point and a normal vector along the
	// beam line direction;
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
