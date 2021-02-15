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


   private:
      virtual void beginJob() override;
      virtual void analyze(const Event&, const EventSetup&) override;
      virtual void endJob() override;
	  bool IsHardProcess(int s){return (s > 20 && s<30);}

      int nPar;
      int PID[MAXPAR], STATUS[MAXPAR];
      float E[MAXPAR],PX[MAXPAR], PY[MAXPAR], PZ[MAXPAR], PT[MAXPAR], ETA[MAXPAR], PHI[MAXPAR];
      float weight;
	  
      // ----------member data ---------------------------
	  EDGetTokenT<LHEEventProduct> generatorlheToken_;
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
   tree_->Branch("STATUS",STATUS,"STATUS[nPar]/F");
   tree_->Branch("weight",&weight);

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
   weight = evet.isValid() ? evet->originalXWGTUP() : 0;
     
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
