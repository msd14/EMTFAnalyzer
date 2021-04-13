import FWCore.ParameterSet.Config as cms

from EMTFAnalyzer.NTupleMaker.GEMEMTFMatcher_cfi import *
from EMTFAnalyzer.NTupleMaker.FlatNtuple_cfi import *

## pick up the right GEM copad collection
GEMEMTFMatcher.gemCoPadTag = cms.InputTag("simCscTriggerPrimitiveDigisILT","","DIGI")

## Run-2 matcher
GEMEMTFMatcherMCRun2 = GEMEMTFMatcher.clone()
GEMEMTFMatcherMCRun2.emtfHitTag = cms.InputTag("simEmtfDigis")
GEMEMTFMatcherMCRun2.emtfTrackTag = cms.InputTag("simEmtfDigis")

## Run-3 matcher
GEMEMTFMatcher.emtfHitTag = cms.InputTag("simEmtfDigisRun3CCLUT")
GEMEMTFMatcher.emtfTrackTag = cms.InputTag("simEmtfDigisRun3CCLUT")

## to be sure that the GEMEMTFMatcher is not screwing up any of the existing EMTFHit or EMTFTrack
## collections, run 4 ntuplizers:
##   Run-2
##   Run-2 + GEM
##   Run-3
##   Run-3 + GEM

##   Run-2
FlatNtupleMCRun2 = FlatNtupleMC.clone()
FlatNtupleMCRun2.emtfHitTag = cms.InputTag("simEmtfDigis")
FlatNtupleMCRun2.emtfTrackTag = cms.InputTag("simEmtfDigis")

##   Run-2 + GEM
FlatNtupleMCRun2GEM = FlatNtupleMC.clone()
FlatNtupleMCRun2GEM.emtfHitTag = cms.InputTag("GEMEMTFMatcherMCRun2")
FlatNtupleMCRun2GEM.emtfTrackTag = cms.InputTag("GEMEMTFMatcherMCRun2")

##   Run-3
FlatNtupleMC.emtfHitTag = cms.InputTag("simEmtfDigisRun3CCLUT")
FlatNtupleMC.emtfTrackTag = cms.InputTag("simEmtfDigisRun3CCLUT")

##   Run-3 + GEM
FlatNtupleMCGEM = FlatNtupleMC.clone()
FlatNtupleMCGEM.emtfHitTag = cms.InputTag("GEMEMTFMatcher")
FlatNtupleMCGEM.emtfTrackTag = cms.InputTag("GEMEMTFMatcher")

GEMEMTFMatchers = cms.Sequence(
    GEMEMTFMatcherMCRun2 *
    GEMEMTFMatcher
)

EMTFAnalyzers = cms.Sequence(
    FlatNtupleMCRun2 *
    FlatNtupleMCRun2GEM *
    FlatNtupleMC *
    FlatNtupleMCGEM
)
