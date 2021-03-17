// -*- C++ -*-
//
// Package:    AnaTools/LHEAnalyzer
// Class:      LHEAnalyzer
//
/**\class LHEAnalyzer LHEAnalyzer.cc AnaTools/LHEAnalyzer/plugins/LHEAnalyzer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Michael Pitt
//         Created:  Sun, 14 Feb 2021 21:13:25 GMT
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "SimDataFormats/GeneratorProducts/interface/LHEEventProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/LHERunInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

// ROOT includes
#include "TTree.h"

#define MAXPAR 50

//
// class declaration
//

using namespace edm;
using namespace std;
using namespace reco;

class LHEAnalyzer : public EDAnalyzer {
   public:
      explicit LHEAnalyzer(const ParameterSet&);
      ~LHEAnalyzer();

      static void fillDescriptions(ConfigurationDescriptions& descriptions);
      virtual void endRun(const edm::Run&,const edm::EventSetup&);

   private:
      virtual void beginJob() override;
      virtual void analyze(const Event&, const EventSetup&) override;
      virtual void endJob() override;
      bool IsHardProcess(int s){return (s > 20 && s<30);}

      int nPar;
      int PID[MAXPAR], STATUS[MAXPAR];
      float E[MAXPAR],PX[MAXPAR], PY[MAXPAR], PZ[MAXPAR], PT[MAXPAR], ETA[MAXPAR], PHI[MAXPAR];
      float weights[9]; // nominal + 4 scale weights + 4 width weights
	  
      // ----------member data ---------------------------
      EDGetTokenT<LHEEventProduct> generatorlheToken_;
      EDGetTokenT<LHERunInfoProduct> generatorRunInfoToken_;
      EDGetTokenT<GenParticleCollection> prunedGenParticlesToken_;
	  
      TTree *tree_;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
LHEAnalyzer::LHEAnalyzer(const ParameterSet& iConfig)
 :
    generatorlheToken_(consumes<LHEEventProduct>(InputTag("externalLHEProducer",""))),
    generatorRunInfoToken_(consumes<LHERunInfoProduct,InRun>({"externalLHEProducer"})),
	prunedGenParticlesToken_(consumes<GenParticleCollection>(InputTag("prunedGenParticles")))

{
   //now do what ever initialization is needed
   
   Service<TFileService> fs;
   tree_ = fs->make<TTree>("tree","tree LHE info"); 
   
   tree_->Branch("nPar",&nPar);
   tree_->Branch("PID",PID,"PID[nPar]/I");
   tree_->Branch("E",E,"E[nPar]/F");
   tree_->Branch("PX",PX,"PX[nPar]/F");
   tree_->Branch("PY",PY,"PY[nPar]/F");
   tree_->Branch("PZ",PZ,"PZ[nPar]/F");
   tree_->Branch("PT",PT,"PT[nPar]/F");
   tree_->Branch("ETA",ETA,"ETA[nPar]/F");
   tree_->Branch("PHI",PHI,"PHI[nPar]/F");
   tree_->Branch("STATUS",STATUS,"STATUS[nPar]/I");
   tree_->Branch("weights",weights,"weights[9]/F");

}


LHEAnalyzer::~LHEAnalyzer()
{

   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called for each event  ------------
void
LHEAnalyzer::analyze(const Event& iEvent, const EventSetup& iSetup)
{
   
   Handle<LHEEventProduct> evet;
   iEvent.getByToken(generatorlheToken_, evet);
   if(!evet.isValid()){
	   cout << "ERROR: wrong LHEEventProduct provided"<<endl;
	   return;
   }
   //weight =  evet->originalXWGTUP();
   for (unsigned int i=0; i<evet->weights().size(); i++) {
	   string id =evet->weights()[i].id.c_str();
	   // nominal weight
	   if (id == "1001") weights[0]=evet->weights()[i].wgt;
	   // Scale uncertainties
	   if (id == "1016") weights[1]=evet->weights()[i].wgt; // muF*2.0
	   if (id == "1031") weights[2]=evet->weights()[i].wgt; // muF*0.5
	   if (id == "1006") weights[3]=evet->weights()[i].wgt; // muR*2.0
	   if (id == "1011") weights[4]=evet->weights()[i].wgt; // muR*0.5
	   // top width
	   if (id == "rwgt_19") weights[5]=evet->weights()[i].wgt; // SM param
	   if (id == "rwgt_16") weights[6]=evet->weights()[i].wgt; // width=2.5GeV
	   if (id == "rwgt_21") weights[7]=evet->weights()[i].wgt; // width=0.5GeV
	   if (id == "rwgt_18") weights[8]=evet->weights()[i].wgt; // width=1.5GeV
   }
	   
   Handle<GenParticleCollection> prunedGenParticles;
   iEvent.getByToken(prunedGenParticlesToken_,prunedGenParticles);
   if(!prunedGenParticles.isValid()){
	   cout << "ERROR: wrong prunedGenParticlesToken provided"<<endl;
	   return;
   }
   
   nPar=0;
   for (size_t i = 0; i < prunedGenParticles->size(); ++i){
	   const GenParticle & genIt = (*prunedGenParticles)[i];
	   //int absid=abs(genIt.pdgId());
	   //bool outGoingProton(absid==2212 && genIt.status()==1 && fabs(genIt.eta())>4.7);
	   //bool topLastCopy(absid==6 && genIt.isLastCopy());
	   //bool wLastCopy(absid==24 && genIt.isLastCopy());
	   //bool zLastCopy(absid==23 && genIt.isLastCopy());
	   //bool bLastCopy(absid==5 && genIt.isLastCopy());
	   if(IsHardProcess(genIt.status())){
		   PID[nPar]=genIt.pdgId();
		   PX[nPar]=genIt.px();
		   PY[nPar]=genIt.py();
		   PZ[nPar]=genIt.pz();
		   PT[nPar]=genIt.pt();
		   ETA[nPar]=genIt.eta();
		   PHI[nPar]=genIt.phi();
		   E[nPar]=genIt.energy();
		   STATUS[nPar]=genIt.status();
		   nPar++;
	   }
	   //cout << genIt.pdgId() << " , " << genIt.status() << " , " << genIt.isLastCopy() << " , " << genIt.isLastCopyBeforeFSR() << endl;
   }
   
   tree_->Fill();
   
}


// ------------ method called once each job just before starting event loop  ------------
void
LHEAnalyzer::beginJob()
{
}

void
LHEAnalyzer::endRun(const edm::Run& iRun,
		     const EventSetup& iSetup)
{
  try{

    cout << "[LHEAnalyzer::endRun]" << endl;
	
	// Following lines list the generator weights as described in 
	//https://twiki.cern.ch/twiki/bin/viewauth/CMS/LHEReaderCMSSW#Retrieving_the_weights
/*
    edm::Handle<LHERunInfoProduct> lheruninfo;
    typedef std::vector<LHERunInfoProduct::Header>::const_iterator headers_const_iterator;
    iRun.getByToken(generatorRunInfoToken_, lheruninfo );

    LHERunInfoProduct myLHERunInfoProduct = *(lheruninfo.product());
	
	// Print all weights and corresponding integers
	for (headers_const_iterator iter=myLHERunInfoProduct.headers_begin(); iter!=myLHERunInfoProduct.headers_end(); iter++){
      std::cout << "tag="<<iter->tag() << std::endl;
      std::vector<std::string> lines = iter->lines();
      for (unsigned int iLine = 0; iLine<lines.size(); iLine++) {
		if(lines.at(iLine)=="") continue;
		//if(lines.at(iLine).find("weightgroup")==std::string::npos) continue;
        std::cout << lines.at(iLine) << std::endl;
      }
    }
*/
  }
  catch(std::exception &e){
    std::cout << e.what() << endl
	      << "Failed to retrieve LHERunInfoProduct" << std::endl;
  }
  

}

// ------------ method called once each job just after ending the event loop  ------------
void
LHEAnalyzer::endJob()
{
	
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
LHEAnalyzer::fillDescriptions(ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);

}

//define this as a plug-in
DEFINE_FWK_MODULE(LHEAnalyzer);
