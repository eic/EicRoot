
beampipe()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  //
  // Prefer to think in [mm] and convert to [cm] when calling ROOT shape 
  // definition routines only;
  //

  BeamPipeGeoParData *bpipe = new BeamPipeGeoParData();

  // Create a central beam pipe element; 800um thick beryllium; 
  BeamPipeElement *ip = new BeamPipeElement("beryllium", 0.800);
  // Let it be a cylindrical pipe of some length; more or less similar to ALICE IP;
  ip->AddSection(-400.0, 36.00);
  ip->AddSection( 400.0, 36.00);

  // And some arbitrary stucture to cover the rest of +/-4.5m around the IP;
  BeamPipeElement *p1 = new BeamPipeElement("aluminum", 1.000);
  // NB: starting offset does not really matter here;
  p1->AddSection( 400.0,  36.00);
  p1->AddSection(1000.0,  40.00);
  p1->AddSection(4500.0,  40.00);

  // Place all elements in this order; assume can use IP-symmetric pipe 
  // for now; NB: swap the electron-beam-going piece;
  bpipe->AddElement  (p1, BeamPipeElement::Swap);
  bpipe->AddIpElement(ip);
  bpipe->AddElement  (p1);

  // Let it just be all gray?;
  bpipe->GetColorTable()->AddPrefixMatch("Beampipe", kGray);

  // Geometry is declared -> create it in ROOT and write out;
  bpipe->ConstructGeometry();

  // Yes, always exit, since otherwise CINT (sometimes) ignores editing results and uses 
  // cached values (really so?);
  exit(0);
}

