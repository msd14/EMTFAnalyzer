
# ## Basic (but usually unnecessary) imports
# import os
# import sys
# import commands

import subprocess

import FWCore.ParameterSet.Config as cms

from Configuration.Eras.Era_Run3_cff import Run3

process = cms.Process('NTUPLE', Run3)

matchGEM = False

## Import standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('SimGeneral.MixingModule.mixNoPU_cfi') ## What does this module do? - AWB 05.09.16
process.load('Configuration.StandardSequences.GeometryRecoDB_cff') ## What does this module do? - AWB 05.09.16
process.load('Configuration.StandardSequences.MagneticField_cff') ## Different than in data ("MagneticField_AutoFromDBCurrent_cff"?)
#process.load('Configuration.StandardSequences.SimL1EmulatorRepack_FullMC_cff') ## Different than in data
process.load('Configuration.StandardSequences.RawToDigi_cff') ## Different than in data ("RawToDigi_Data_cff"?)
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('L1Trigger.CSCTriggerPrimitives.cscTriggerPrimitiveDigis_cfi')

## CSCTF digis, phi / pT LUTs?
process.load("L1TriggerConfig.L1ScalesProducers.L1MuTriggerScalesConfig_cff")
process.load("L1TriggerConfig.L1ScalesProducers.L1MuTriggerPtScaleConfig_cff")

## Import RECO muon configurations
process.load("RecoMuon.TrackingTools.MuonServiceProxy_cff")
process.load("RecoMuon.TrackingTools.MuonTrackLoader_cff")

## Message Logger and Event range
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(100) )
process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(False))

## Global Tags
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase1_2021_realistic', '')

## Default parameters for firmware version, pT LUT XMLs, and coordinate conversion LUTs
#process.load('L1Trigger.L1TMuonEndCap.fakeEmtfParams_cff')

readFiles = cms.untracked.vstring()
process.source = cms.Source(
    'PoolSource',
    fileNames = cms.untracked.vstring(
        '/store/user/mdecaro/SingleMu_Run3_Pt1to50OneOverPt_noPU_10M/CRAB3_SingleMu_Run3_Pt1to50OneOverPt_noPU_10M/210325_152507/0000/step1_85.root'
        #    'file:step2bis_run3.root'
    ),
)

###################
###  NTuplizer  ###
###################
process.load('EMTFAnalyzer.NTupleMaker.GEMEMTFMatcher_cfi')
process.load('EMTFAnalyzer.NTupleMaker.FlatNtuple_cfi')

process.GEMEMTFMatcher.verbose = True

## Run-2 matcher
process.GEMEMTFMatcherMCRun2 = process.GEMEMTFMatcher.clone()
process.GEMEMTFMatcherMCRun2.emtfHitTag = cms.InputTag("simEmtfDigis")
process.GEMEMTFMatcherMCRun2.emtfTrackTag = cms.InputTag("simEmtfDigis")

## Run-3 matcher
process.GEMEMTFMatcher.emtfHitTag = cms.InputTag("simEmtfDigisRun3CCLUT")
process.GEMEMTFMatcher.emtfTrackTag = cms.InputTag("simEmtfDigisRun3CCLUT")

## just in case the copads need to be rerun
#process.GEMEMTFMatcher.gemCoPadTag = cms.InputTag("cscTriggerPrimitiveDigis","","NTUPLE")

## to be sure that the GEMEMTFMatcher is not screwing up any of the existing EMTFHit or EMTFTrack
## collections, run 4 ntuplizers:
##   Run-2
##   Run-2 + GEM
##   Run-3
##   Run-3 + GEM

##   Run-2
process.FlatNtupleMCRun2 = process.FlatNtupleMC.clone()
process.FlatNtupleMCRun2.emtfHitTag = cms.InputTag("simEmtfDigis")
process.FlatNtupleMCRun2.emtfTrackTag = cms.InputTag("simEmtfDigis")

##   Run-2 + GEM
process.FlatNtupleMCRun2GEM = process.FlatNtupleMC.clone()
process.FlatNtupleMCRun2GEM.emtfHitTag = cms.InputTag("GEMEMTFMatcherMCRun2")
process.FlatNtupleMCRun2GEM.emtfTrackTag = cms.InputTag("GEMEMTFMatcherMCRun2")

##   Run-3
process.FlatNtupleMC.emtfHitTag = cms.InputTag("simEmtfDigisRun3CCLUT")
process.FlatNtupleMC.emtfTrackTag = cms.InputTag("simEmtfDigisRun3CCLUT")

##   Run-3 + GEM
process.FlatNtupleMCGEM = process.FlatNtupleMC.clone()
process.FlatNtupleMCGEM.emtfHitTag = cms.InputTag("GEMEMTFMatcher")
process.FlatNtupleMCGEM.emtfTrackTag = cms.InputTag("GEMEMTFMatcher")

process.matcher = cms.Sequence(
    process.GEMEMTFMatcherMCRun2 *
    process.GEMEMTFMatcher
)

process.Analysis = cms.Sequence(
    process.FlatNtupleMCRun2 *
    process.FlatNtupleMCRun2GEM *
    process.FlatNtupleMC *
    process.FlatNtupleMCGEM
)

process.Analysis_step = cms.Path(
    process.matcher *
    process.Analysis
)

process.endjob_step = cms.EndPath(process.endOfProcess)

## NTuple output File
process.TFileService = cms.Service(
    "TFileService",
    fileName = cms.string('EMTF_MC_NTuple_SingleMu.root')
)

# Schedule definition
process.schedule = cms.Schedule(process.Analysis_step,process.endjob_step)
