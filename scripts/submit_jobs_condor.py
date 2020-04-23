#!/usr/bin/env python

import sys, os, argparse, json, subprocess, shutil
from time import strftime

def files4Dataset(dataset):

    # Construct dasgoclient call and make it, reading back the standard out to get the list of files
    proc = subprocess.Popen(['dasgoclient', '--query=file dataset=%s'%(dataset)], stdout=subprocess.PIPE)
    files = proc.stdout.readlines();  files = [file.rstrip() for file in files]

    # Prep the files for insertion into cmsRun config by adding the global redirector
    returnFiles = []
    for file in files: returnFiles.append(file.replace("/store/", "root://cms-xrd-global.cern.ch///store/"))

    return returnFiles 

def getParents4File(file):

    # We can't do a dasgoclient query with a redirector in the path...
    cleanFile = file.replace("root://cms-xrd-global.cern.ch//", "")

    # Find the parent files (more than one!) for each file in the dataset
    proc = subprocess.Popen(['dasgoclient', '--query=parent file=%s'%(cleanFile)], stdout=subprocess.PIPE)
    parents = proc.stdout.readlines();  parents = [parent.rstrip() for parent in parents]

    # Prep the parents files for insertion into cmsRun config by adding the global redirector.
    returnParents = []
    for parent in parents:
        returnParent = "'%s'"%(parent)
        returnParents.append(returnParent.replace("/store/", "root://cms-xrd-global.cern.ch///store/"))

    return returnParents

# Run cmsDriver to make cmsRun config for generating L1 ntuples
def generate_ntuple_config(caloparams, globaltag, era, doRates, doEffs, isData, useParent):

    commandList = ['cmsDriver.py', 'l1Ntuple', '-s', 'RAW2DIGI']

    commandList.append('--python_filename=ntuple_maker.py')
    commandList.append('-n'); commandList.append('-1')

    # suppresses the (unneeded) the RAW2DIGI output
    commandList.append('--no_output')

    # should always be set to Run2_2018 for 2018 data
    commandList.append('--era=%s'%(era))

    # validations are always run on data, not MC
    if isData: commandList.append('--data')
    else:      commandList.append('--mc')

    # default conditions
    commandList.append('--conditions=%s'%(globaltag))

    # run re-emulation including re-emulation of HCAL TPs
    commandList.append('--customise=L1Trigger/Configuration/customiseReEmul.L1TReEmulFromRAWsimHcalTP')

    # include emulated quantities in L1Ntuple
    if doRates:  commandList.append('--customise=L1Trigger/L1TNtuples/customiseL1Ntuple.L1NtupleRAWEMU')
    elif doEffs: commandList.append('--customise=L1Trigger/L1TNtuples/customiseL1Ntuple.L1NtupleAODRAWEMU')

    # use correct CaloStage2Params; should only change if Layer2 calibration changes
    if(caloparams): commandList.append('--customise=L1Trigger/Configuration/customiseSettings.L1TSettingsToCaloParams_%s'%(caloparams))

    # need to use LUTGenerationMode = False because we are using L1TriggerObjects
    commandList.append("--customise_commands=process.HcalTPGCoderULUT.LUTGenerationMode=cms.bool(False)")

    # default input file
    commandList.append('--filein=CHILDFILE')

    if useParent: commandList.append('--secondfilein=PARENTFILE')

    commandList.append('--no_exec')
    commandList.append('--fileout=L1Ntuple.root')

    print "\n"
    print "Running cmsDriver like this:\n"
    print " ".join(commandList)
    print "\n"

    subprocess.call(commandList)

# Write .sh script to be run by Condor
def generate_job_steerer(workingDir, outputDir, useParent):

    scriptFile = open("%s/runJob.sh"%(workingDir), "w")
    scriptFile.write("#!/bin/bash\n\n")
    scriptFile.write("CHILDFILE=$1\n")

    if useParent:
        scriptFile.write("PARENTFILE=$2\n")
        scriptFile.write("JOB=$3\n\n")
    else:
        scriptFile.write("JOB=$2\n\n")

    scriptFile.write("export SCRAM_ARCH=slc7_amd64_gcc700\n\n")
    scriptFile.write("source /cvmfs/cms.cern.ch/cmsset_default.sh\n") 
    scriptFile.write("eval `scramv1 project CMSSW CMSSW_10_6_0`\n\n")
    scriptFile.write("tar -xf CMSSW_10_6_0.tar.gz\n")
    scriptFile.write("mv ntuple_maker.py algo_weights.py CMSSW_10_6_0/src\n")
    scriptFile.write("cd CMSSW_10_6_0/src\n")
    scriptFile.write("ls -lrth\n\n")
    scriptFile.write("scramv1 b ProjectRename\n")
    scriptFile.write("eval `scramv1 runtime -sh`\n")
    scriptFile.write("sed -i \"s|CHILDFILE|${CHILDFILE}|g\" ntuple_maker.py\n")
    scriptFile.write("sed -i \"s|'PARENTFILE'|${PARENTFILE}|g\" ntuple_maker.py\n\n")
    scriptFile.write("cmsRun ntuple_maker.py\n\n")
    scriptFile.write("xrdcp -f L1Ntuple.root %s/L1Ntuple_${JOB}.root 2>&1\n\n"%(outputDir))
    scriptFile.write("cd ${_CONDOR_SCRATCH_DIR}\n")
    scriptFile.write("rm -r CMSSW_10_6_0*\n")
    scriptFile.close()

# Write Condor submit file 
def generate_condor_submit(workingDir, inputFileMap, useParent):

    condorSubmit = open("%s/condorSubmit.jdl"%(workingDir), "w")
    condorSubmit.write("Executable           =  %s/runJob.sh\n"%(workingDir))
    condorSubmit.write("Universe             =  vanilla\n")
    condorSubmit.write("Requirements         =  OpSys == \"LINUX\" && Arch ==\"x86_64\"\n")
    condorSubmit.write("Request_Memory       =  3.5 Gb\n")
    condorSubmit.write("Output               =  %s/logs/$(Cluster)_$(Process).stdout\n"%(workingDir))
    condorSubmit.write("Error                =  %s/logs/$(Cluster)_$(Process).stderr\n"%(workingDir))
    condorSubmit.write("Log                  =  %s/logs/$(Cluster)_$(Process).log\n"%(workingDir))
    condorSubmit.write("Transfer_Input_Files =  %s/ntuple_maker.py, %s/runJob.sh, %s/algo_weights.py, %s/%s.tar.gz\n"%(workingDir,workingDir,workingDir,workingDir,os.getenv("CMSSW_VERSION")))
    condorSubmit.write("x509userproxy        =  $ENV(X509_USER_PROXY)\n\n")
    
    iJob = 1
    for childFile, parentFiles in inputFileMap.iteritems():
        
        stub = childFile.split("/")[-1].split(".root")[0]

        if useParent: condorSubmit.write("Arguments       = %s %s %d\n"%(childFile, parentFiles, iJob))
        else:         condorSubmit.write("Arguments       = %s %d\n"%(childFile, iJob))
        condorSubmit.write("Queue\n\n")

        iJob += 1
    
    condorSubmit.close()

# This function injects config code fragments into the cmsRun config that was generated
# These things cannot be specified during the cmsDriver execution
def hackCMSRUNconfig(scheme, workingDir):

    f = open("ntuple_maker.py", "r")
    contents = f.readlines()
    f.close()
    
    # Begin lamest hack in the world
    contents.insert(2, "import sys\n")
    contents.insert(10, "from algo_weights import pfaWeightsMap\n")
    
    contents.insert(22,"process.load(\"SimCalorimetry.HcalTrigPrimProducers.hcaltpdigi_cff\")\n")
    contents.insert(23,"\n")
    
    if "PFA2p" in scheme:
        contents.insert(24, "print \"Only using 3 samples\"\n")
        contents.insert(25, "process.simHcalTriggerPrimitiveDigis.numberOfSamplesQIE11 = 3\n")
        contents.insert(26, "process.simHcalTriggerPrimitiveDigis.numberOfPresamplesQIE11 = 1\n")
        contents.insert(27, "process.HcalTPGCoderULUT.contain1TS = False\n")
        contents.insert(28, "process.HcalTPGCoderULUT.containPhaseNS = 3.0\n")
    
    elif "PFA2" in scheme:
        contents.insert(24, "print \"Only using 2 samples\"\n")
        contents.insert(25, "process.simHcalTriggerPrimitiveDigis.numberOfSamplesQIE11 = 2\n")
        contents.insert(26, "process.simHcalTriggerPrimitiveDigis.numberOfPresamplesQIE11 = 0\n")
        contents.insert(27, "process.HcalTPGCoderULUT.contain1TS = False\n")
        contents.insert(28, "process.HcalTPGCoderULUT.containPhaseNS = 3.0\n")

    elif "PFA1p" in scheme:
        contents.insert(24, "print \"Only using 2 sample\"\n")
        contents.insert(25, "process.simHcalTriggerPrimitiveDigis.numberOfSamplesQIE11 = 2\n")
        contents.insert(26, "process.simHcalTriggerPrimitiveDigis.numberOfPresamplesQIE11 = 1\n")
        contents.insert(27, "process.HcalTPGCoderULUT.contain1TS = True\n")
        contents.insert(28, "process.HcalTPGCoderULUT.containPhaseNS = 3.0\n")

    elif "PFA1" in scheme:
        contents.insert(24, "print \"Only using 1 sample\"\n")
        contents.insert(25, "process.simHcalTriggerPrimitiveDigis.numberOfSamplesQIE11 = 1\n")
        contents.insert(26, "process.simHcalTriggerPrimitiveDigis.numberOfPresamplesQIE11 = 0\n")
        contents.insert(27, "process.HcalTPGCoderULUT.contain1TS = True\n")
        contents.insert(28, "process.HcalTPGCoderULUT.containPhaseNS = 3.0\n")

    contents.insert(28,"\n")
    contents.insert(29,"if \"%s\" in pfaWeightsMap:\n"%(scheme))
    contents.insert(30,"    process.simHcalTriggerPrimitiveDigis.weightsQIE11 = pfaWeightsMap[\"%s\"]\n"%(scheme))
    contents.insert(31,"else:\n")
    contents.insert(32,"    print \"No weights defined for scheme '%s'; defaulting to zero weights!\"\n\n"%(scheme))

    f = open("%s/ntuple_maker.py"%(workingDir), "w")
    contents = "".join(contents)
    f.write(contents)
    f.close()

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-g', '--globalTag' , type=str , default='106X_mcRun3_2021_realistic_v3')
    parser.add_argument('-l', '--lumimask'  , type=str , default='NULL')
    parser.add_argument('-d', '--dataset'   , type=str , required=True)
    parser.add_argument('-a', '--scheme'      , type=str , required=True)
    parser.add_argument('-n', '--noSubmit'  , default=False, action="store_true")
    parser.add_argument('-c', '--caloParams', type=str , default='2018_v1_3')
    parser.add_argument('-e', '--era'       , type=str , default='Run3')
    parser.add_argument('-r', '--doRates'   , default=False, action="store_true")
    parser.add_argument('-f', '--doEffs'    , default=False, action="store_true") 
    args = parser.parse_args()

    era        = args.era
    scheme     = args.scheme
    doEffs     = args.doEffs
    doRates    = args.doRates
    dataset    = args.dataset
    globalTag  = args.globalTag
    caloParams = args.caloParams   

    HOME = os.getenv("HOME")
    USER = os.getenv("USER")
    CMSSW_BASE = os.getenv("CMSSW_BASE")
    CMSSW_VERSION = os.getenv("CMSSW_VERSION")

    # If we are running efficiencies we need AOD and the RAW parent
    useParent = False;
    if doEffs: useParent = True

    # From the dataset specified, determine if we are running on data or mc
    isData = True;
    if "mcRun" in dataset: isData = False

    # Do some incompatible arguments checks
    if isData and "mcRun" in globalTag:
        print "STOP! Trying to run on data with an MC global tag!"
        print "Exiting..."
        quit()

    if not isData and "dataRun" in globalTag:
        print "STOP! Trying to run on MC with a data global tag!"
        print "Exiting..."
        quit()

    if doRates == False and doEffs == False or doRates == True and doEffs == True:
        print "Must decide running rates or efficiencies"
        print "Exiting..."
        quit()

    # Use date and time as the base directory for the workspace
    taskDir = strftime("%Y%m%d_%H%M%S")

    # Get the physics process from the input dataset name to use for setting up the workspace structure.
    physProcess = dataset.split("/")[1].split("_")[0]
    
    # Setup the output directory and working directory given the pass parameters
    outputDir = "root://cmseos.fnal.gov//store/user/%s/HCAL_Trigger_Study/L1Ntuples/%s/%s"%(USER, physProcess,scheme)
    workingDir = "%s/condor/%s_%s_%s"%(os.getcwd(),physProcess,scheme,taskDir)
    
    # Use proper eos syntax to make a new directory there
    subprocess.call(["eos", "root://cmseos.fnal.gov", "mkdir", "-p", outputDir[23:]])
    os.makedirs(workingDir)
    
    if outputDir.split("/")[-1] == "":  outputDir  = outputDir[:-1]
    if workingDir.split("/")[-1] == "": workingDir = workingDir[:-1]

    # Create directories to save logs
    logDir = "%s/logs"%(workingDir);  os.makedirs(logDir)

    scriptsDir = "%s/nobackup/HCAL_Trigger_Study/scripts"%(HOME)
    shutil.copy2("%s/algo_weights.py"%(scriptsDir), "%s"%(workingDir))

    # From the input dataset get the list of files to run on and also the parent files if applicable
    datasetFiles = files4Dataset(dataset)
    
    childParentFilePair = {} 
    for file in datasetFiles:
        if useParent:
            parents = getParents4File(file)
            childParentFilePair[file] = ",".join(parents)
        else:
            childParentFilePair[file] = "NULL"

    # Generate cmsDriver commands
    generate_ntuple_config(caloParams, globalTag, era, doRates, doEffs, isData, useParent)
    
    # From freshly created cmsRun config, hack it and inject some extra code...
    hackCMSRUNconfig(scheme, workingDir)

    # Write the sh file used to run the show on the worker node
    generate_job_steerer(workingDir, outputDir, useParent)    

    # Write the condor submit file for condor to do its thing
    generate_condor_submit(workingDir, childParentFilePair, useParent)    

    subprocess.call(["chmod", "+x", "%s/runJob.sh"%(workingDir)])

    subprocess.call(["tar", "--exclude-caches-all", "--exclude-vcs", "-zcf", "%s/%s.tar.gz"%(workingDir,CMSSW_VERSION), "-C", "%s/.."%(CMSSW_BASE), CMSSW_VERSION, "--include=lib", "--include=python", "--include=src/L1Trigger"])
    
    if not args.noSubmit: os.system("condor_submit %s/condorSubmit.jdl"%(workingDir))
