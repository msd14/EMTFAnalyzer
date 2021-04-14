import FWCore.ParameterSet.Config as cms

GEMEMTFMatcher = cms.EDProducer(
    'GEMEMTFMatcher',
    emtfHitTag       = cms.InputTag("simEmtfDigis"),
    emtfTrackTag     = cms.InputTag("simEmtfDigis"),
    gemCoPadTag      = cms.InputTag("simCscTriggerPrimitiveDigis"),
    verbose = cms.bool(False),
    minChamber = cms.int32(-1),
    maxChamber = cms.int32(1),
)
