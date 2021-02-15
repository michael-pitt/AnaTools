#include <iostream>
#include <cmath>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include "TApplication.h"
#include "TMatrixD.h"
#include "TF1.h"
#include "TTree.h"
#include "TFile.h"
#include "TLeaf.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TLorentzVector.h"
#include "TGraph.h"
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
	float weight, mtop;
	tree_out.Branch("weight",&weight,"weight/F");
	tree_out.Branch("mtop",&mtop,"mtop/F");
	
	// Read the input file
	TFile file(argv[1]);
	TTree* tree = (TTree*) file.Get("analysis/tree");
	int nevents = tree->GetEntries();
	std::cout << nevents << " events read from file(s) " << argv[1] << std::endl;
	
	for(int h=0; h < nevents; h++){
		if(h%1000==0) printf ("\r [%3.0f%%] done", 100.*(float)h/(float)nevents);
		tree->GetEvent(h);
		
		int nPar=tree->GetLeaf("nPar")->GetValue(0);
		weight=tree->GetLeaf("weight")->GetValue(0);
		int nb=0; TLorentzVector p4_W, p4_b;
		for(int i=0;i<nPar;i++){
			if(tree->GetLeaf("STATUS")->GetValue(i)<0) continue; // skip incoming partons   
			int pid=TMath::Abs(tree->GetLeaf("PID")->GetValue(i));
			float px=tree->GetLeaf("PX")->GetValue(i);
			float py=tree->GetLeaf("PY")->GetValue(i);
			float pz=tree->GetLeaf("PZ")->GetValue(i);
			float e=tree->GetLeaf("E")->GetValue(i);
			if (pid==24){p4_W.SetPxPyPzE(px,py,pz,e);}
			else if(pid==5){if(nb) p4_b.SetPxPyPzE(px,py,pz,e); nb++;}
			// store top mass
			mtop=(p4_W+p4_b).M();
		}
		
		tree_out.Fill();
			
	}
			
	cout << "Writes " << output.GetName() << endl;
    output.Write();		
	output.Close();

    return 0;
}
