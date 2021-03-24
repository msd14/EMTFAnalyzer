# -*- coding: utf-8 -*-
#CRAB configuration to do processing of Monte Carlo. Inputs a dataset from previous step of processing chain.
#Note: Filenames in psetName must match filenames of input dataset.

from CRABClient.UserUtilities import config
config = config()

config.General.requestName = 'CRAB3_Mu_1to50OneOverPt_NTuples_03242021'
config.General.transferLogs = True
config.General.transferOutputs = True

config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'Run3_MC_NTuple.py'
config.JobType.allowUndistributedCMSSW = True

#10M OneOverPt 1-50GeV NoPU
config.Data.inputDataset = '/SingleMu_Run3_Pt1to50OneOverPt_noPU_10M_01042021/madecaro-CRAB3_Mu_OneOverPt1to50-_DIGI_L1-Run3CCLUT_01042021-7ed74bfbab65b9f484d29c14384023fa/USER'

#20M 1-1000GeV Flat NoPU
#config.Data.inputDataset = '/Mu_FlatPt1to1000-pythia8-gun/madecaro-CRAB3_Mu_FlatPt1to1000-pythia8-gun_MiniAOD-NoPU_110X__DIGI_L1-Run3CCLUT_12032020-229f036ac0fca798df437149c176cb37/USER'

#1M Neutrino E10 (for rate studies)
#config.Data.inputDataset = '/Nu_E10-pythia8-gun/madecaro-CRAB3_Nu_E10-pythia8-gun_110X_mcRun3_020521-30dd185d8c700f676e9d5aa13cf2b0e3/USER' 

config.Data.ignoreLocality = True
config.Data.inputDBS = 'phys03'
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 1
config.Data.publication = True
config.Data.outputDatasetTag = 'CRAB3_SingleMu_1to50OneOverPt_NTuples_03242021'

config.Site.storageSite = 'T3_US_FNALLPC'
config.Site.whitelist = ['T2_US*']

config.Site.ignoreGlobalBlacklist = True 
