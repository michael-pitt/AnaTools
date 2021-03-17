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

#define NWEIGHT 9

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
  float weights[NWEIGHT], mtop, mtop_rec, mtop_rec2; int bcharge, top_charge;
  float bjet1_pt, bjet2_pt, W_pt;
  float bjet1_eta, bjet2_eta , W_eta;
  tree_out.Branch("weights",weights,Form("weights[%d]/F",NWEIGHT));
  tree_out.Branch("mtop",&mtop,"mtop/F");
  tree_out.Branch("mtop_rec",&mtop_rec,"mtop_rec/F");
  tree_out.Branch("mtop_rec2",&mtop_rec2,"mtop_rec2/F");
  tree_out.Branch("W_pt",&W_pt,"W_pt/F");
  tree_out.Branch("bjet1_pt",&bjet1_pt,"bjet1_pt/F");
  tree_out.Branch("bjet2_pt",&bjet2_pt,"bjet2_pt/F");
  tree_out.Branch("W_eta",&W_eta,"W_eta/F");
  tree_out.Branch("bjet1_eta",&bjet1_eta,"bjet1_eta/F");
  tree_out.Branch("bjet2_eta",&bjet2_eta,"bjet2_eta/F");
  tree_out.Branch("bcharge",&bcharge,"bcharge/I");
  tree_out.Branch("top_charge",&top_charge,"top_charge/I");
	
  // Read the input file
  TFile file(argv[1]);
  TTree* tree = (TTree*) file.Get("analysis/tree");
  int nevents = tree->GetEntries();
  std::cout << nevents << " events read from file(s) " << argv[1] << std::endl;
	
  for(int h=0; h < nevents; h++){
    if(h%1000==0) printf ("\r [%3.0f%%] done", 100.*(float)h/(float)nevents);
	tree->GetEvent(h);
    
	// generator weights (nominal + scale)
	for(int i=0;i<NWEIGHT;i++) weights[i]=tree->GetLeaf("weights")->GetValue(i);
	
	// GEN content
	int nPar=tree->GetLeaf("nPar")->GetValue(0);
	TLorentzVector p4_W, p4_b1, p4_b2;
	int w_charge=0; top_charge=0;
	
	// get top and W charges
	for(int i=0;i<nPar;i++){
		int pid=tree->GetLeaf("PID")->GetValue(i);
		int abspid=TMath::Abs(pid);
		if (abspid==6) top_charge=pid/abspid;
		if (abspid==24) w_charge=pid/abspid;
	}
	
	
	for(int i=0;i<nPar;i++){
	  if(tree->GetLeaf("STATUS")->GetValue(i)==21) continue; // skip incoming partons   
	  int pid=tree->GetLeaf("PID")->GetValue(i);
	  int abspid=TMath::Abs(pid);
	  float px=tree->GetLeaf("PX")->GetValue(i);
	  float py=tree->GetLeaf("PY")->GetValue(i);
	  float pz=tree->GetLeaf("PZ")->GetValue(i);
	  float e=tree->GetLeaf("E")->GetValue(i);
	  if (abspid==24){p4_W.SetPxPyPzE(px,py,pz,e);}
	  else if(abspid==5){
		  if(top_charge){
			  if(w_charge==(pid/abspid)) p4_b1.SetPxPyPzE(px,py,pz,e);
			  else p4_b2.SetPxPyPzE(px,py,pz,e);
		  }
		  else {
			  if(w_charge==(pid/abspid)) p4_b1.SetPxPyPzE(px,py,pz,e);
			  else p4_b2.SetPxPyPzE(px,py,pz,e);
		  }
	  }
	  
	} // end loop oevr particles

    // store top mass
    mtop = (p4_W+p4_b1).M();
    bjet1_pt = p4_b1.Pt();
	bjet2_pt = p4_b2.Pt();
    bjet1_eta = p4_b1.Eta();
	bjet2_eta = p4_b2.Eta();
	W_pt = p4_W.Pt();
	W_eta = p4_W.Eta();
	if (bjet1_pt>bjet2_pt){
		mtop_rec  = (p4_W+p4_b1).M();
		mtop_rec2 = (p4_W+p4_b2).M();
	}
    else{
		mtop_rec2 = (p4_W+p4_b1).M();
		mtop_rec  = (p4_W+p4_b2).M();
	}
	bcharge = top_charge ? top_charge : w_charge;
	
	tree_out.Fill();
			
  }
  printf ("\r [%3.0f%%] done\n", 100.);
			
  cout << "Writes " << output.GetName() << endl;
  output.Write();		
  output.Close();

  return 0;
}
