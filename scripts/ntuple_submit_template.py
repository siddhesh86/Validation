# template for crab submission.
# submit_jobs.py will insert definitions above
conditionType = "def" # default
# if new L1TriggerObjects conditons have been specified with either
# a new tag or file
if NEWCONDITIONS:
    conditionType = "new_cond"

from CRABClient.UserUtilities import config
config = config()

config.General.requestName = 'hcal' + '_' + str(PFA) + "_" + str(DATASET.split("/")[1]) + "_" + conditionType
config.General.transferLogs = True
config.General.transferOutputs = True

# Name of the CMSSW configuration file                                                                                                       
config.JobType.psetName = 'ntuple_maker_' + conditionType + '.py'
config.JobType.allowUndistributedCMSSW = True

config.JobType.pluginName = 'Analysis'
#config.JobType.maxMemoryMB = 2500                                                                                                           
config.JobType.outputFiles = ['L1Ntuple.root']

config.Data.inputDataset = DATASET
config.Data.ignoreLocality = False
config.Data.inputDBS = 'global'
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 1
config.Data.useParent = USEPARENT

# This string is used to construct the output dataset name                                                                                   
config.Data.outputDatasetTag = 'Hcal' + '_' + str(PFA) + "_" + str(DATASET.split("/")[1]) + "_" + conditionType

# These values only make sense for processing data                                                                                           
#    Select input data based on a lumi mask                                                                                                  
#config.Data.lumiMask = LUMIMASK

#    Select input data based on run-ranges                                                                                                   
#config.Data.runRange = str(RUN)

# Where the output files will be transmitted to                                                                                              
config.Site.storageSite = OUTPUTSITE
#config.Site.blacklist = ["T1_RU_JINR"]
