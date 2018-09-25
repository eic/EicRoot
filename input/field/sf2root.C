
void sf2root(TString fname)
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // Import ${VMCWORKDIR}/input/${fname}.dat & initialize;
  PndSolenoidMap *fField = new PndSolenoidMap(fname, "A");
  fField->Init();

  // Convert and dump ROOT version to ./${fname}.root;
  fField->WriteRootFile(fname + ".root", fname);

  exit(0);
} // sf2root()
  
