#include <iostream>
#include <cmath>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include "TTree.h"
#include "TFile.h"
#include "TLeaf.h"
#include "TLorentzVector.h"
#include "TSystem.h"

using namespace std;

int main(int argc, char* argv[]){
	
	
  if (argc !=2) {
    cout << "ERROR: missing arguments" << endl
     << "Usage: " << argv[0] << " inputMCFileName" << endl;
    return 1;
  }
	
   // Check input files
  if(gSystem->AccessPathName(argv[1])){
    cout << "ERROR: missing input file " << argv[1] << endl;
    return 0;
  }
	
  TString infile = TString(argv[1]);
  TString outfile = TString(TString(argv[1]).Tokenize("/")->Last()->GetName()).ReplaceAll(".root","_proc.root");
	
  TFile output(outfile,"RECREATE","");
  TTree tree_out("tree","");
  float weights[5], mtop, mtopp, mtopm; int bcharge;
  tree_out.Branch("weights",weights,"weights[5]/F");
  tree_out.Branch("mtop",&mtop,"mtop/F");
  tree_out.Branch("mtopp",&mtopp,"mtopp/F");
  tree_out.Branch("mtopm",&mtopm,"mtopm/F");
  tree_out.Branch("bcharge",&bcharge,"bcharge/I");
	
  // Read the input file
  TFile file(argv[1]);
  TTree* tree = (TTree*) file.Get("analysis/tree");
  int nevents = tree->GetEntries();
  std::cout << nevents << " events read from file(s) " << argv[1] << std::endl;
	
  for(int h=0; h < nevents; h++){
    if(h%1000==0) printf ("\r [%3.0f%%] done", 100.*(float)h/(float)nevents);
	tree->GetEvent(h);
    
	// generator weights (nominal + scale)
	for(int i=0;i<5;i++) weights[i]=tree->GetLeaf("weights")->GetValue(i);
	
	// GEN content
	int nPar=tree->GetLeaf("nPar")->GetValue(0);
	TLorentzVector p4_W, p4_bp, p4_bm;
	for(int i=0;i<nPar;i++){
	  if(tree->GetLeaf("STATUS")->GetValue(i)<0) continue; // skip incoming partons   
	  int pid=tree->GetLeaf("PID")->GetValue(i);
	  int abspid=TMath::Abs(pid);
	  float px=tree->GetLeaf("PX")->GetValue(i);
	  float py=tree->GetLeaf("PY")->GetValue(i);
	  float pz=tree->GetLeaf("PZ")->GetValue(i);
	  float e=tree->GetLeaf("E")->GetValue(i);
	  if (abspid==24){p4_W.SetPxPyPzE(px,py,pz,e);}
	  else if(abspid==5){
		  if(pid>0) p4_bm.SetPxPyPzE(px,py,pz,e);
		  else p4_bp.SetPxPyPzE(px,py,pz,e);
	  }
	  // store top mass
	  
	  if( p4_bm.Pt()>p4_bp.Pt() ){
		  bcharge = -1;
		  mtop= (p4_W+p4_bm).M();
	  }
	  else{
		  bcharge = +1;
		  mtop= (p4_W+p4_bp).M();
	  }
	  mtopm = (p4_W+p4_bm).M();
	  mtopp = (p4_W+p4_bp).M();
	} // end loop oevr particles
		
	tree_out.Fill();
			
  }
  printf ("\r [%3.0f%%] done\n", 100.);
			
  cout << "Writes " << output.GetName() << endl;
  output.Write();		
  output.Close();

  return 0;
}
