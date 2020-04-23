#!/usr/bin/env python
"""Create CRAB submission script for L1 rate change 
when L1TriggerObjects conditions are updated"""
import sys
import os
import argparse
import json

## choose the type of jobs: use ['def', 'new_cond'] for Validation and ['def'] for other:
COND_LIST = ['def']
#COND_LIST = ['def', 'new_cond']
# Relatively stable parameters can have defaults.
# era should never change within a year
ERA = 'Run3'
# current data global tag
CONDITIONS = '106X_mcRun3_2021_realistic_v3'
# L1 calibrations; needs to be updated when L1 calibrations change
CALOSTAGE2PARAMS = '2018_v1_3'
# dummy value needed so that cmsDriver.py will
# assume that there is an input file
DEFAULTINPUT = '/store/express/Run2017B/ExpressPhysics/FEVT/Express-v1/000/297/562/00000/EE1F5F26-145B-E711-A146-02163E019C23.root'
# frontier database (needs to be specified when overriding conditions)
FRONTIER = 'frontier://FrontierProd/CMS_CONDITIONS'

def check_setup():
    if not ("CMSSW_BASE" in os.environ):
        sys.exit("Please issue 'cmsenv' before running")
    if not ("crabclient" in os.environ['PATH']):
        sys.exit("Please set up crab environment before running")

def generate_ntuple_config(configtype, newtag, caloparams, algo):
    """Generates ntuple python file for a given 
    config type (default or new conditions) and 
    a new HcalL1TriggerObjects tag"""

    cmd = 'cmsDriver.py l1Ntuple -s RAW2DIGI '
    cmd += '--python_filename=ntuple_maker_' + configtype + '.py '
    # number of events; overridden by CRAB
    cmd += '-n 10 '
    # suppresses the (unneeded) the RAW2DIGI output
    cmd += '--no_output '
    # should always be set to Run2_2018 for 2018 data
    cmd += '--era=' + ERA +  ' '
    # validations are always run on data, not MC
    cmd += '--mc '
    # default conditions
    cmd += '--conditions=' + CONDITIONS + ' '
    # run re-emulation including re-emulation of HCAL TPs
    cmd += '--customise=L1Trigger/Configuration/customiseReEmul.L1TReEmulFromRAWsimHcalTP '

    if ARGS.rates:
        # include emulated quantities in L1Ntuple
        cmd += '--customise=L1Trigger/L1TNtuples/customiseL1Ntuple.L1NtupleRAWEMU '
    elif ARGS.effs:
        # include emulated quantities in L1Ntuple
        cmd += '--customise=L1Trigger/L1TNtuples/customiseL1Ntuple.L1NtupleAODRAWEMU '
    # use correct CaloStage2Params; should only change if Layer2 calibration changes
    if(caloparams):
        cmd += '--customise=L1Trigger/Configuration/customiseSettings.L1TSettingsToCaloParams_' + CALOSTAGE2PARAMS + ' '
    # override HcalL1TriggerObjects
    if(configtype == 'new_cond'):
        cmd += '--custom_conditions=' + newtag + ',HcalL1TriggerObjectsRcd,' + FRONTIER + ' '
    # need to use LUTGenerationMode = False because we are using L1TriggerObjects
    cmd += "--customise_commands='process.HcalTPGCoderULUT.LUTGenerationMode=cms.bool(False)' "
    # default input file

    cmd += '--filein=' + DEFAULTINPUT + ' '
    cmd += '--no_exec '
    return cmd

PARSER = argparse.ArgumentParser()
# existing GT
PARSER.add_argument('-g', '--globaltag')
# new L1TriggerObjects tag
PARSER.add_argument('-t', '--newtag', required=False)
#PARSER.add_argument('-l', '--lumimask', required=True)
PARSER.add_argument('-d', '--dataset', required=True)
PARSER.add_argument('-s', '--scheme', required=True)
PARSER.add_argument('-o', '--outputsite', required=True)
PARSER.add_argument('-p', '--useparent', default=False, action="store_true")
PARSER.add_argument('-n', '--no_exec', default=False, action="store_true")
PARSER.add_argument('-c', '--caloparams')
PARSER.add_argument('--rates', default=False, action="store_true")
PARSER.add_argument('--effs', default=False, action="store_true") 
ARGS = PARSER.parse_args()

if ARGS.rates == False and ARGS.effs == False:
    print "Must decide running rates or efficiencies"
    print "Exiting..."
    quit()

# check environment setup
check_setup()

#FILE = file(ARGS.lumimask)
#GOOD_RUN_STRING = FILE.read()
#GOOD_RUN_DATA = json.loads(GOOD_RUN_STRING)
if(ARGS.globaltag):
    CONDITIONS = ARGS.globaltag
if(ARGS.caloparams):
    CALOSTAGE2PARAMS = ARGS.caloparams   
#if len(GOOD_RUN_DATA) != 1:
#    sys.exit("Only running on a single run at a time is supported.")
#RUN = GOOD_RUN_DATA.keys()[0]
# generate configs both for default and new conditions
#for jobtype in ['def', 'new_cond']:
if ARGS.newtag>0:
    COND_LIST.append('new_cond') 

for jobtype in COND_LIST:
    tmpfile = 'submit_tmp.py'
    crab_submit_script = open(tmpfile, 'w')
    #crab_submit_script.write("RUN = " + str(RUN) + '\n')
    if jobtype == 'def':
        crab_submit_script.write("NEWCONDITIONS = False\n")
    else:
        crab_submit_script.write("NEWCONDITIONS = True\n")
    crab_submit_script.write("OUTPUTSITE = '" + ARGS.outputsite + "'\n")
    #crab_submit_script.write("LUMIMASK = '" + ARGS.lumimask + "'\n")
    crab_submit_script.write("DATASET = '" + ARGS.dataset + "'\n\n")
    crab_submit_script.write("USEPARENT = %r"%(ARGS.useparent) + "\n\n")
    crab_submit_script.write("PFA = '" + ARGS.scheme + "'\n\n")
    crab_submit_script.close()
    
    # concatenate crab submission file with template
    filename = 'submit_run_' + jobtype + '.py'
    command = "cat submit_tmp.py ntuple_submit_template.py > " + filename
    os.system(command)
    os.remove(tmpfile)

    # generate cmsDriver commands
    if ARGS.newtag>0:
        print generate_ntuple_config(jobtype, ARGS.newtag, ARGS.caloparams, ARGS.scheme)
        os.system(generate_ntuple_config(jobtype, ARGS.newtag, ARGS.caloparams, ARGS.scheme))
    else:
        print generate_ntuple_config(jobtype,0, ARGS.caloparams, ARGS.scheme)
        os.system(generate_ntuple_config(jobtype, 0, ARGS.caloparams, ARGS.scheme))  

    f = open("ntuple_maker_def.py", "r")
    contents = f.readlines()
    f.close()
    
    # Begin lamest hack in the world
    contents.insert(2, "import sys\n")
    contents.insert(3, "sys.path.insert(0, \"/uscms/home/jhiltb/nobackup/HCAL_Trigger_Study/scripts/\")\n")
    contents.insert(10, "from algo_weights import pfaWeightsMap\n")

    contents.insert(22,"process.load(\"SimCalorimetry.HcalTrigPrimProducers.hcaltpdigi_cff\")\n")
    contents.insert(23,"\n")
    
    if "PFA2p" in ARGS.scheme:
        contents.insert(24, "print \"Only using 3 samples\"\n")
        contents.insert(25, "process.simHcalTriggerPrimitiveDigis.numberOfSamplesQIE11 = 3\n")
        contents.insert(26, "process.simHcalTriggerPrimitiveDigis.numberOfPresamplesQIE11 = 1\n")
        contents.insert(27, "process.HcalTPGCoderULUT.contain1TS = False\n")
        contents.insert(28, "process.HcalTPGCoderULUT.containPhaseNS = 3.0\n")
    
    elif "PFA2" in ARGS.scheme:
        contents.insert(24, "print \"Only using 2 samples\"\n")
        contents.insert(25, "process.simHcalTriggerPrimitiveDigis.numberOfSamplesQIE11 = 2\n")
        contents.insert(26, "process.simHcalTriggerPrimitiveDigis.numberOfPresamplesQIE11 = 0\n")
        contents.insert(27, "process.HcalTPGCoderULUT.contain1TS = False\n")
        contents.insert(28, "process.HcalTPGCoderULUT.containPhaseNS = 3.0\n")

    elif "PFA1p" in ARGS.scheme:
        contents.insert(24, "print \"Only using 2 sample\"\n")
        contents.insert(25, "process.simHcalTriggerPrimitiveDigis.numberOfSamplesQIE11 = 2\n")
        contents.insert(26, "process.simHcalTriggerPrimitiveDigis.numberOfPresamplesQIE11 = 1\n")
        contents.insert(27, "process.HcalTPGCoderULUT.contain1TS = True\n")
        contents.insert(28, "process.HcalTPGCoderULUT.containPhaseNS = 3.0\n")

    elif "PFA1" in ARGS.scheme:
        contents.insert(24, "print \"Only using 1 sample\"\n")
        contents.insert(25, "process.simHcalTriggerPrimitiveDigis.numberOfSamplesQIE11 = 1\n")
        contents.insert(26, "process.simHcalTriggerPrimitiveDigis.numberOfPresamplesQIE11 = 0\n")
        contents.insert(27, "process.HcalTPGCoderULUT.contain1TS = True\n")
        contents.insert(28, "process.HcalTPGCoderULUT.containPhaseNS = 3.0\n")

    contents.insert(28,"\n")
    contents.insert(29,"if \"%s\" in pfaWeightsMap:\n"%(ARGS.scheme))
    contents.insert(30,"    process.simHcalTriggerPrimitiveDigis.weightsQIE11 = pfaWeightsMap[\"%s\"]\n"%(ARGS.scheme))
    contents.insert(31,"else:\n")
    contents.insert(32,"    print \"No weights defined for scheme '%s'; defaulting to zero weights!\"\n\n"%(ARGS.scheme))
    
    f = open("ntuple_maker_def.py", "w")
    contents = "".join(contents)
    f.write(contents)
    f.close()

    if(not ARGS.no_exec):
        crabcmd = "crab submit " + filename
        os.system(crabcmd)
