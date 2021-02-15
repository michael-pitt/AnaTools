import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing

options = VarParsing.VarParsing('python')
options.register('isGEN', False, VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.bool, "set to True if running on GEN")
options.register('isAOD', False, VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.bool, "set to True if running on AOD")
options.register('isNLO', False, VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.bool, "set to True if running on aMC@NLO")
options.register('gridpack',
                 '/eos/cms/store/cmst3/group/top/WbWb/gridpacks/ST_4f_w_lo_slc7_amd64_gcc630_CMSSW_9_3_16_tarball.tar.xz',
                 VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.string,
                 "gridpack to use")
options.register('seed', 123456789, VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.int, "seed to use")

options.parseArguments()
print(options)

process = cms.Process("LHE")

# import of standard configurations
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.load('Configuration.StandardSequences.Services_cff')
process.MessageLogger.cerr.FwkReport.reportEvery = cms.untracked.int32(250)

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(options.maxEvents) )

if options.isGEN or options.isAOD:
  if not options.inputFiles: print('ERROR: no input file provided!')
  else: process.source = cms.Source("PoolSource",fileNames = cms.untracked.vstring(options.inputFiles))
else:
  process.source = cms.Source("EmptySource")
  process.MessageLogger.cerr.FwkReport.reportEvery = cms.untracked.int32(500)

  #configure the LHE producer (for gridpack run)
  process.externalLHEProducer = cms.EDProducer("ExternalLHEProducer",
    nEvents = cms.untracked.uint32(options.maxEvents),
    outputFile = cms.string('cmsgrid_final.lhe'),
    scriptName = cms.FileInPath('GeneratorInterface/LHEInterface/data/run_generic_tarball_cvmfs.sh'),
    numberOfParameters = cms.uint32(1),
    args = cms.vstring(options.gridpack)
  )

  from Configuration.Generator.Pythia8CommonSettings_cfi import *
  from Configuration.Generator.MCTunes2017.PythiaCP5Settings_cfi import *
  from Configuration.Generator.PSweightsPythia.PythiaPSweightsSettings_cfi import *
  if options.isNLO: from Configuration.Generator.Pythia8aMCatNLOSettings_cfi import pythia8aMCatNLOSettingsBlock

  # LO hadronization without matching (ickkw=0 in datacards)
  process.generator = cms.EDFilter("Pythia8HadronizerFilter",
                                 maxEventsToPrint = cms.untracked.int32(0),
                                 pythiaPylistVerbosity = cms.untracked.int32(1),
                                 filterEfficiency = cms.untracked.double(1.0),
                                 pythiaHepMCVerbosity = cms.untracked.bool(False),
                                 comEnergy = cms.double(13000.),
                                 PythiaParameters = cms.PSet(
                                     parameterSets = cms.vstring('pythia8CommonSettings',
                                                                 'pythia8CUEP8M1Settings'),
                                                                         pythia8CUEP8M1Settings = cms.vstring('Tune:pp 14',
                                                                         'Tune:ee 7',
                                                                         'MultipartonInteractions:pT0Ref=2.4024',
                                                                         'MultipartonInteractions:ecmPow=0.25208',
                                                                         'MultipartonInteractions:expPow=1.6'),
                                                                         pythia8CommonSettings = cms.vstring('Tune:preferLHAPDF = 2',
                                                                         'Main:timesAllowErrors = 10000',
                                                                         'Check:epTolErr = 0.01',
                                                                         'Beams:setProductionScalesFromLHEF = off',
                                                                         'SLHA:keepSM = on',
                                                                         'SLHA:minMassSM = 1000.',
                                                                         'ParticleDecays:limitTau0 = on',
                                                                         'ParticleDecays:tau0Max = 10',
                                                                         'ParticleDecays:allowPhotonRadiation = on'
                                     )
                                 )
  ) 
  #NLO Matching
  if options.isNLO:
    process.generator = cms.EDFilter("Pythia8HadronizerFilter",
                                 maxEventsToPrint = cms.untracked.int32(1),
                                 pythiaPylistVerbosity = cms.untracked.int32(1),
                                 filterEfficiency = cms.untracked.double(1.0),
                                 pythiaHepMCVerbosity = cms.untracked.bool(False),
                                 comEnergy = cms.double(13000.),
                                 PythiaParameters = cms.PSet(
                                     pythia8CommonSettingsBlock,
                                     pythia8CP5SettingsBlock,
                                                                         pythia8aMCatNLOSettingsBlock,
                                                                         processParameters = cms.vstring('TimeShower:nPartonsInBorn = 3'), #number of coloured particles (before resonance decays) in born matrix element
                                     parameterSets = cms.vstring('pythia8CommonSettings',
                                                                 'pythia8CP5Settings',
                                                                 'pythia8aMCatNLOSettings',
                                                                 'processParameters',
                                     )
                                 )
    )
  print(process.generator.PythiaParameters)
  process.generator_step = cms.Path(process.externalLHEProducer*process.generator)

  from IOMC.RandomEngine.RandomServiceHelper import RandomNumberServiceHelper
  randSvc = RandomNumberServiceHelper(process.RandomNumberGeneratorService)
  process.RandomNumberGeneratorService.externalLHEProducer.initialSeed=cms.untracked.int32(options.seed)
  randSvc.populate()
  print 'Seed is',process.RandomNumberGeneratorService.externalLHEProducer.initialSeed
  
  process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
  process.load('PhysicsTools.HepMCCandAlgos.genParticles_cfi')
  process.genParticles.src= cms.InputTag("generator","unsmeared")
  process.load('PhysicsTools.PatAlgos.slimming.prunedGenParticles_cfi') 
  process.prun_step = cms.Path(process.genParticles*process.prunedGenParticles)
  

process.TFileService = cms.Service("TFileService",fileName = cms.string(options.outputFile))

process.analysis = cms.EDAnalyzer('LHEAnalyzer')
process.analysis_step = cms.Path(process.analysis)

#process.analysis = cms.OutputModule("PoolOutputModule",
#                          fileName = cms.untracked.string("pool.root"),
#                          outputCommands = cms.untracked.vstring("keep *"))
#process.analysis_step = cms.EndPath(process.analysis)

if options.isGEN or options.isAOD:
  process.schedule = cms.Schedule(process.analysis_step)
else:
  process.schedule = cms.Schedule(process.generator_step,process.prun_step,process.analysis_step)


