// Script for calculating rate histograms
// Originally from Aaron Bundock
#include "TMath.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TChain.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "L1Trigger/L1TNtuples/interface/L1AnalysisEventDataFormat.h"
#include "L1Trigger/L1TNtuples/interface/L1AnalysisL1UpgradeDataFormat.h"
#include "L1Trigger/L1TNtuples/interface/L1AnalysisRecoVertexDataFormat.h"
#include "L1Trigger/L1TNtuples/interface/L1AnalysisCaloTPDataFormat.h"
#include <sys/stat.h>
#include <cstdlib>
//#include <json>
#include <fstream>
//#include <jsoncpp/json/json.h>
//#include <jsoncpp/json.h>
//#include <json/json.h>
//#include "TBufferJSON.h"
#include "jsoncpp.cpp"

/* TODO: put errors in rates...
creates the the rates and distributions for l1 trigger objects
How to use:
1. input the number of bunches in the run (~line 35)
2. change the variables "newConditionsNtuples" and "oldConditionsNtuples" to ntuple paths
3. If good run JSON is not applied during ntuple production, modify isGoodLumiSection()

Optionally, if you want to rescale to a given instantaneous luminosity:
1. input the instantaneous luminosity of the run (~line 32) [only if we scale to 2016 nominal]
2. select whether you rescale to L=1.5e34 (~line606??...) generally have it setup to rescale
nb: for 2&3 I have provided the info in runInfoForRates.txt
*/

// configurable parameters
double numBunch = 2544; //the number of bunches colliding for the run of interest
double runLum = 0.02; // 0.44: 275783  0.58:  276363 //luminosity of the run of interest (*10^34)
double expectedLum = 1.15; //expected luminosity of 2016 runs (*10^34)
bool toCheckWithJosnFile = true; // check goodLumiSections if not done while producing l1tNtuples
std::string goodLumiSectionJSONFile = "/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions18/13TeV/PromptReco/Cert_314472-325175_13TeV_PromptReco_Collisions18_JSON.txt"; // 2018 promptReco


// Run3 LHC parameters for normalizing rates
double instLumi = 2e34; // Hz/cm^2, https://indico.cern.ch/event/880508/contributions/3720014/attachments/1980197/3297287/CMS-Week_20200203_LHCStatus_Schaumann_v2.pdf
double mbXSec   = 6.92e-26; // cm^2, minimum bias cross section from Run2: https://twiki.cern.ch/twiki/bin/view/CMS/PileupJSONFileforData#Recommended_cross_section

void rates(bool newConditions, const std::string& inputFileDirectory);

int main(int argc, char *argv[])
{
  bool newConditions = true;
  std::string ntuplePath("");

  if (argc != 3) {
    std::cout << "Usage: rates.exe [new/def] [path to ntuples]\n"
	      << "[new/def] indicates new or default (existing) conditions" << std::endl;
    exit(1);
  }
  else {
    std::string par1(argv[1]);
    std::transform(par1.begin(), par1.end(), par1.begin(), ::tolower);
    if(par1.compare("new") == 0) newConditions = true;
    else if(par1.compare("def") == 0) newConditions = false;
    else {
      std::cout << "First parameter must be \"new\" or \"def\"" << std::endl;
      exit(1);
    }
    ntuplePath = argv[2];
  }

  rates(newConditions, ntuplePath);

  return 0;
}

// only need to edit this section if good run JSON
// is not used during ntuple production
bool isGoodLumiSection(int lumiBlock)
{
  if (lumiBlock >= 1
      || lumiBlock <= 10000) {
    return true;
  }

  return false;
}

bool isGoodLumiSection(uint runNumber, uint lumiBlock, bool toCheckWithJosnFile)
{
  if ( ! toCheckWithJosnFile ) {
    if (lumiBlock >= 1
	|| lumiBlock <= 10000) {
      return true;
    }
  }

  //std::cout << "isGoodLumiSection():: runNumber: "<<runNumber<<", lumiBlock: " << lumiBlock << std::endl;
  bool isGoodLumiSec = false;
  // Reading JSON file:
  // Ref: https://en.wikibooks.org/wiki/JsonCpp
  // Ref to obtain jconcpp.cpp etc: https://github.com/open-source-parsers/jsoncpp/tree/update  
  std::ifstream fGoodLumiSects(goodLumiSectionJSONFile.c_str(), std::ifstream::binary);
  Json::Reader reader;
  Json::Value obj;
  reader.parse(fGoodLumiSects, obj);
  std::string sRun=Form("%u",(uint)runNumber);
  //std::cout << " run : " << sRun << ", size: " << obj[sRun.c_str()].size() << std::endl;
  for (int iLumiBlock=0; iLumiBlock < (int)obj[sRun.c_str()].size(); iLumiBlock++) {    
    uint runFirstLumiSec = obj[sRun.c_str()][iLumiBlock][0].asUInt();
    uint runLastLumiSec  = obj[sRun.c_str()][iLumiBlock][1].asUInt();
    //std::cout << "iLumiBlock " << iLumiBlock << ", size: " << obj[sRun.c_str()][iLumiBlock].size()
    //	      << ", lumi [" << runFirstLumiSec << ", " << runLastLumiSec << "]" << std::endl;
    if (lumiBlock >= runFirstLumiSec  &&  lumiBlock <= runLastLumiSec) {
      //std::cout <<" GoodLumiSec" << std::endl;
      isGoodLumiSec = true;
      break;
    }
  }
  //if ( ! isGoodLumiSec) std::cout <<" noGoodLumiSec" << std::endl;
  return isGoodLumiSec;
}

void rates(bool newConditions, const std::string& inputFileDirectory){
  
  bool hwOn = true;   //are we using data from hardware? (upgrade trigger had to be running!!!)
  bool emuOn = true;  //are we using data from emulator?

  if (hwOn==false && emuOn==false){
    std::cout << "exiting as neither hardware or emulator selected" << std::endl;
    return;
  }

  std::string inputFile(inputFileDirectory);
  inputFile += "/L1Ntuple_*.root";
  std::string outputDirectory = "emu";  //***runNumber, triggerType, version, hw/emu/both***MAKE SURE IT EXISTS
  std::string outputFilename = "rates_def.root";
  if(newConditions) outputFilename = "rates_new_cond.root";
  TFile* kk = TFile::Open( outputFilename.c_str() , "recreate");
  // if (kk!=0){
  //   cout << "TERMINATE: not going to overwrite file " << outputFilename << endl;
  //   return;
  // }


  // make trees
  std::cout << "Loading up the TChain..." << std::endl;
  TChain * treeL1emu = new TChain("l1UpgradeEmuTree/L1UpgradeTree");
  if (emuOn){
    treeL1emu->Add(inputFile.c_str());
  }
  TChain * treeL1hw = new TChain("l1UpgradeTree/L1UpgradeTree");
  if (hwOn){
    treeL1hw->Add(inputFile.c_str());
  }
  TChain * eventTree = new TChain("l1EventTree/L1EventTree");
  eventTree->Add(inputFile.c_str());

  // In case you want to include PU info
  // TChain * vtxTree = new TChain("l1RecoTree/RecoTree");
  // if(binByPileUp){
  //   vtxTree->Add(inputFile.c_str());
  // }


  TChain * treeL1TPemu = new TChain("l1CaloTowerEmuTree/L1CaloTowerTree");
  if (emuOn){
    treeL1TPemu->Add(inputFile.c_str());
  }

  TChain * treeL1TPhw = new TChain("l1CaloTowerTree/L1CaloTowerTree");
  if (hwOn){
    treeL1TPhw->Add(inputFile.c_str());
  }

  L1Analysis::L1AnalysisL1UpgradeDataFormat    *l1emu_ = new L1Analysis::L1AnalysisL1UpgradeDataFormat();
  treeL1emu->SetBranchAddress("L1Upgrade", &l1emu_);
  L1Analysis::L1AnalysisL1UpgradeDataFormat    *l1hw_ = new L1Analysis::L1AnalysisL1UpgradeDataFormat();
  treeL1hw->SetBranchAddress("L1Upgrade", &l1hw_);
  L1Analysis::L1AnalysisEventDataFormat    *event_ = new L1Analysis::L1AnalysisEventDataFormat();
  eventTree->SetBranchAddress("Event", &event_);
  // L1Analysis::L1AnalysisRecoVertexDataFormat    *vtx_ = new L1Analysis::L1AnalysisRecoVertexDataFormat();
  // vtxTree->SetBranchAddress("Vertex", &vtx_);

  L1Analysis::L1AnalysisCaloTPDataFormat    *l1TPemu_ = new L1Analysis::L1AnalysisCaloTPDataFormat();
  treeL1TPemu->SetBranchAddress("CaloTP", &l1TPemu_);
  L1Analysis::L1AnalysisCaloTPDataFormat    *l1TPhw_ = new L1Analysis::L1AnalysisCaloTPDataFormat();
  treeL1TPhw->SetBranchAddress("CaloTP", &l1TPhw_);


  // get number of entries
  Long64_t nentries;
  if (emuOn) nentries = treeL1emu->GetEntries();
  else nentries = treeL1hw->GetEntries();
  int goodLumiEventCount = 0;

  std::string outputTxtFileDir = "output_rates/" + outputDirectory;
  struct stat buffer;
  if (stat(outputTxtFileDir.c_str(), &buffer) != 0) {
    printf("output directory %s doesn't exists... creading it\n",outputTxtFileDir.c_str());
    const int dir_err = system(Form("mkdir -p %s",outputTxtFileDir.c_str()));
    if (dir_err == -1) {
      printf("Error creating directory!n");
      exit(1);
    }    
  }
  std::string outputTxtFilename = outputTxtFileDir + "/extraInfo_def.txt";
  if (newConditions) outputTxtFilename = outputTxtFileDir + "/extraInfo_new.txt";  
  std::ofstream myfile; // save info about the run, including rates for a given lumi section, and number of events we used.
  myfile.open(outputTxtFilename.c_str());
  eventTree->GetEntry(0);
  myfile << "run number = " << event_->run << std::endl;

  // set parameters for histograms
  // jet bins
  int nJetBins = 400;
  float jetLo = 0.;
  float jetHi = 400.;
  float jetBinWidth = (jetHi-jetLo)/nJetBins;

  // EG bins
  int nEgBins = 300;
  float egLo = 0.;
  float egHi = 300.;
  float egBinWidth = (egHi-egLo)/nEgBins;

  // tau bins
  int nTauBins = 300;
  float tauLo = 0.;
  float tauHi = 300.;
  float tauBinWidth = (tauHi-tauLo)/nTauBins;

  // htSum bins
  int nHtSumBins = 1000;
  float htSumLo = 0.;
  float htSumHi = 1000.;
  float htSumBinWidth = (htSumHi-htSumLo)/nHtSumBins;

  // mhtSum bins
  int nMhtSumBins = 300;
  float mhtSumLo = 0.;
  float mhtSumHi = 300.;
  float mhtSumBinWidth = (mhtSumHi-mhtSumLo)/nMhtSumBins;

  // etSum bins
  int nEtSumBins = 1000;
  float etSumLo = 0.;
  float etSumHi = 1000.;
  float etSumBinWidth = (etSumHi-etSumLo)/nEtSumBins;

  // metSum bins
  int nMetSumBins = 300;
  float metSumLo = 0.;
  float metSumHi = 300.;
  float metSumBinWidth = (metSumHi-metSumLo)/nMetSumBins;

  // metHFSum bins
  int nMetHFSumBins = 300;
  float metHFSumLo = 0.;
  float metHFSumHi = 300.;
  float metHFSumBinWidth = (metHFSumHi-metHFSumLo)/nMetHFSumBins;

  // tp bins
  int nTpBins = 100;
  float tpLo = 0.;
  float tpHi = 100.;

  int nPVbins = 101;
  float pvLow = -0.5;
  float pvHi  = 100.5;

  // eta bins
  std::map<std::string, std::vector<double>> ETA_CAT;
  ETA_CAT["HBEF"] = {0.000, 5.210};  // ## Whole detector, 1 - 41
  ETA_CAT["HB"]   = {0.000, 1.392};  // ## Trigger towers  1 - 16
  ETA_CAT["HE1"]  = {1.392, 1.740};  // ## Trigger towers 17 - 20
  ETA_CAT["HE2a"] = {1.740, 2.322};  // ## Trigger towers 21 - 25
  ETA_CAT["HE2b"] = {2.322, 3.000};  // ## Trigger towers 26 - 28
  ETA_CAT["HF"]   = {3.000, 5.210};  // ## Trigger towers 30 - 41
  
  std::string axR = ";Threshold E_{T} (GeV);nPV;rate (Hz)";
  std::string axET = ";E_{T} (GeV);nPV;Events / bin";
  std::string axHT = ";H_{T} (GeV);nPV;Events / bin";
  std::string axMET = ";MET (GeV);nPV;Events / bin";
  std::string axMHT = ";MHT (GeV);nPV;Events / bin";
  std::string axMETHF = ";MET HF (GeV);nPV;Events / bin";

  std::string axD = ";E_{T} (GeV);nPV;Events / bin"; 

  TH2F* singleJetRates_emu = new TH2F("singleJetRates_emu", axR.c_str(), nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F* doubleJetRates_emu = new TH2F("doubleJetRates_emu", axR.c_str(), nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F* tripleJetRates_emu = new TH2F("tripleJetRates_emu", axR.c_str(), nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F* quadJetRates_emu = new TH2F("quadJetRates_emu", axR.c_str(), nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F* singleEgRates_emu = new TH2F("singleEgRates_emu", axR.c_str(), nEgBins, egLo, egHi, nPVbins, pvLow, pvHi);
  TH2F* doubleEgRates_emu = new TH2F("doubleEgRates_emu", axR.c_str(), nEgBins, egLo, egHi, nPVbins, pvLow, pvHi);
  TH2F* singleTauRates_emu = new TH2F("singleTauRates_emu", axR.c_str(), nTauBins, tauLo, tauHi, nPVbins, pvLow, pvHi);
  TH2F* doubleTauRates_emu = new TH2F("doubleTauRates_emu", axR.c_str(), nTauBins, tauLo, tauHi, nPVbins, pvLow, pvHi);
  TH2F* singleISOEgRates_emu = new TH2F("singleISOEgRates_emu", axR.c_str(), nEgBins, egLo, egHi, nPVbins, pvLow, pvHi);
  TH2F* doubleISOEgRates_emu = new TH2F("doubleISOEgRates_emu", axR.c_str(), nEgBins, egLo, egHi, nPVbins, pvLow, pvHi);
  TH2F* singleISOTauRates_emu = new TH2F("singleISOTauRates_emu", axR.c_str(), nTauBins, tauLo, tauHi, nPVbins, pvLow, pvHi);
  TH2F* doubleISOTauRates_emu = new TH2F("doubleISOTauRates_emu", axR.c_str(), nTauBins, tauLo, tauHi, nPVbins, pvLow, pvHi);

  TH2F* htSumRates_emu = new TH2F("htSumRates_emu",axR.c_str(), nHtSumBins, htSumLo, htSumHi, nPVbins, pvLow, pvHi);
  TH2F* mhtSumRates_emu = new TH2F("mhtSumRates_emu",axR.c_str(), nMhtSumBins, mhtSumLo, mhtSumHi, nPVbins, pvLow, pvHi);
  TH2F* etSumRates_emu = new TH2F("etSumRates_emu",axR.c_str(), nEtSumBins, etSumLo, etSumHi, nPVbins, pvLow, pvHi);
  TH2F* metSumRates_emu = new TH2F("metSumRates_emu",axR.c_str(), nMetSumBins, metSumLo, metSumHi, nPVbins, pvLow, pvHi); 
  TH2F* metHFSumRates_emu = new TH2F("metHFSumRates_emu",axR.c_str(), nMetHFSumBins, metHFSumLo, metHFSumHi, nPVbins, pvLow, pvHi); 

  TH2F* htSum_emu = new TH2F("htSum_emu",axHT.c_str(), nHtSumBins, htSumLo, htSumHi, nPVbins, pvLow, pvHi);
  TH2F* mhtSum_emu = new TH2F("mhtSum_emu",axMHT.c_str(), nMhtSumBins, mhtSumLo, mhtSumHi, nPVbins, pvLow, pvHi);
  TH2F* etSum_emu = new TH2F("etSum_emu",axET.c_str(), nEtSumBins, etSumLo, etSumHi, nPVbins, pvLow, pvHi);
  TH2F* metSum_emu = new TH2F("metSum_emu",axMET.c_str(), nMetSumBins, metSumLo, metSumHi, nPVbins, pvLow, pvHi); 
  TH2F* metHFSum_emu = new TH2F("metHFSum_emu",axMETHF.c_str(), nMetHFSumBins, metHFSumLo, metHFSumHi, nPVbins, pvLow, pvHi); 

  std::map<std::string, TH2F*> singleJetRates_etaCat_emu;
  for (auto etaBin : ETA_CAT) {
    std::string sEtaBin = etaBin.first;
    singleJetRates_etaCat_emu[sEtaBin] = new TH2F(Form("singleJetRates_%s_emu",sEtaBin.c_str()), axR.c_str(), nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  }
    
  TH2F* singleJetRates_hw = new TH2F("singleJetRates_hw", axR.c_str(), nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F* doubleJetRates_hw = new TH2F("doubleJetRates_hw", axR.c_str(), nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F* tripleJetRates_hw = new TH2F("tripleJetRates_hw", axR.c_str(), nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F* quadJetRates_hw = new TH2F("quadJetRates_hw", axR.c_str(), nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F* singleEgRates_hw = new TH2F("singleEgRates_hw", axR.c_str(), nEgBins, egLo, egHi, nPVbins, pvLow, pvHi);
  TH2F* doubleEgRates_hw = new TH2F("doubleEgRates_hw", axR.c_str(), nEgBins, egLo, egHi, nPVbins, pvLow, pvHi);
  TH2F* singleTauRates_hw = new TH2F("singleTauRates_hw", axR.c_str(), nTauBins, tauLo, tauHi, nPVbins, pvLow, pvHi);
  TH2F* doubleTauRates_hw = new TH2F("doubleTauRates_hw", axR.c_str(), nTauBins, tauLo, tauHi, nPVbins, pvLow, pvHi);
  TH2F* singleISOEgRates_hw = new TH2F("singleISOEgRates_hw", axR.c_str(), nEgBins, egLo, egHi, nPVbins, pvLow, pvHi);
  TH2F* doubleISOEgRates_hw = new TH2F("doubleISOEgRates_hw", axR.c_str(), nEgBins, egLo, egHi, nPVbins, pvLow, pvHi);
  TH2F* singleISOTauRates_hw = new TH2F("singleISOTauRates_hw", axR.c_str(), nTauBins, tauLo, tauHi, nPVbins, pvLow, pvHi);
  TH2F* doubleISOTauRates_hw = new TH2F("doubleISOTauRates_hw", axR.c_str(), nTauBins, tauLo, tauHi, nPVbins, pvLow, pvHi);
  TH2F* htSumRates_hw = new TH2F("htSumRates_hw",axR.c_str(), nHtSumBins, htSumLo, htSumHi, nPVbins, pvLow, pvHi);
  TH2F* mhtSumRates_hw = new TH2F("mhtSumRates_hw",axR.c_str(), nMhtSumBins, mhtSumLo, mhtSumHi, nPVbins, pvLow, pvHi);
  TH2F* etSumRates_hw = new TH2F("etSumRates_hw",axR.c_str(), nEtSumBins, etSumLo, etSumHi, nPVbins, pvLow, pvHi);
  TH2F* metSumRates_hw = new TH2F("metSumRates_hw",axR.c_str(), nMetHFSumBins, metHFSumLo, metHFSumHi, nPVbins, pvLow, pvHi); 
  TH2F* metHFSumRates_hw = new TH2F("metHFSumRates_hw",axR.c_str(), nMetHFSumBins, metHFSumLo, metHFSumHi, nPVbins, pvLow, pvHi); 

  TH2F* hcalTP_emu = new TH2F("hcalTP_emu", ";TP E_{T};nPV;# Entries", nTpBins, tpLo, tpHi, nPVbins, pvLow, pvHi);
  TH2F* ecalTP_emu = new TH2F("ecalTP_emu", ";TP E_{T};nPV;# Entries", nTpBins, tpLo, tpHi, nPVbins, pvLow, pvHi);

  TH2F* hcalTP_hw = new TH2F("hcalTP_hw", ";TP E_{T};nPV;# Entries", nTpBins, tpLo, tpHi,nPVbins, pvLow, pvHi);
  TH2F* ecalTP_hw = new TH2F("ecalTP_hw", ";TP E_{T};nPV;# Entries", nTpBins, tpLo, tpHi,nPVbins, pvLow, pvHi);

  std::map<std::string, TH2F*> singleJetRates_etaCat_hw;
  for (auto etaBin : ETA_CAT) {
    std::string sEtaBin = etaBin.first;
    singleJetRates_etaCat_hw[sEtaBin] = new TH2F(Form("singleJetRates_%s_hw",sEtaBin.c_str()), axR.c_str(), nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  }


  
  /////////////////////////////////
  // loop through all the entries//
  /////////////////////////////////
  Long64_t nentries0;
  nentries0 = nentries;
  //nentries = 3e+05;
  printf("\nRun %lld entries out of total %lld entries\n",nentries,nentries0);
  for (Long64_t jentry=0; jentry<nentries; jentry++){
      if((jentry%10000)==0) std::cout << "Done " << jentry  << " events of " << nentries << std::endl;
    
    
      //lumi break clause
      eventTree->GetEntry(jentry);
      //skip the corresponding event
      //if (!isGoodLumiSection(event_->lumi)) continue;
      //std::cout << "run: " << event_->run << ", lumi: " << event_->lumi << ", PV: " << event_->nPV << std::endl;
      if (!isGoodLumiSection(event_->run, event_->lumi, toCheckWithJosnFile)) continue;
      goodLumiEventCount++; 
      
      //int nPV = event_->nPV_True;
      int nPV = event_->nPV;
      //printf("\nentry %lld: nPV: %i %i",jentry,event_->nPV,event_->nPV_True);
      //std::cout << "entry " << jentry << ": nPV: " << event_->nPV << ", nPT_True: " << event_->nPV_True << std::endl;

      // nPV, nPV_True not filled for data
      if (nPV > 10000) nPV = 0;
      
      //do routine for L1 emulator quantites
      if (emuOn){

          treeL1TPemu->GetEntry(jentry);
          double tpEt(0.);
        
          for(int i=0; i < l1TPemu_->nHCALTP; i++){
	          tpEt = l1TPemu_->hcalTPet[i];
              hcalTP_emu->Fill(tpEt, nPV);
          }
          for(int i=0; i < l1TPemu_->nECALTP; i++){
	          tpEt = l1TPemu_->ecalTPet[i];
	          ecalTP_emu->Fill(tpEt, nPV);
          }

          treeL1emu->GetEntry(jentry);

          // get jetEt*, egEt*, tauEt, htSum, mhtSum, etSum, metSum
          double jetEt_1 = 0;
          double jetEt_2 = 0;
          double jetEt_3 = 0;
          double jetEt_4 = 0;
          if (l1emu_->nJets>0) jetEt_1 = l1emu_->jetEt[0];
          if (l1emu_->nJets>1) jetEt_2 = l1emu_->jetEt[1];
          if (l1emu_->nJets>2) jetEt_3 = l1emu_->jetEt[2];
          if (l1emu_->nJets>3) jetEt_4 = l1emu_->jetEt[3];
	  double jetEta_1 = 0;
	  std::vector<std::string> sJetEtaBins_1; // string representing jet1 eta: HB, HE1, HE2a, HE2b, HF, HBEF (all HCAL)
	  if (l1emu_->nJets>0) jetEta_1 = l1emu_->jetEta[0];
	  for (auto etaBin : ETA_CAT) {
	    if (l1emu_->nJets>0 && std::abs(jetEta_1) >= etaBin.second.at(0) && std::abs(jetEta_1) < etaBin.second.at(1)) {
	      sJetEtaBins_1.push_back(etaBin.first);
	    }
	  }
	  	  
          double egEt_1 = 0;
          double egEt_2 = 0;
          //EG pt's are not given in descending order...bx?
          for (UInt_t c=0; c<l1emu_->nEGs; c++){
            if (l1emu_->egEt[c] > egEt_1){
              egEt_2 = egEt_1;
              egEt_1 = l1emu_->egEt[c];
            }
            else if (l1emu_->egEt[c] <= egEt_1 && l1emu_->egEt[c] > egEt_2){
              egEt_2 = l1emu_->egEt[c];
            }
          }

          double tauEt_1 = 0;
          double tauEt_2 = 0;
          //tau pt's are not given in descending order
          for (UInt_t c=0; c<l1emu_->nTaus; c++){
            if (l1emu_->tauEt[c] > tauEt_1){
              tauEt_2 = tauEt_1;
              tauEt_1 = l1emu_->tauEt[c];
            }
            else if (l1emu_->tauEt[c] <= tauEt_1 && l1emu_->tauEt[c] > tauEt_2){
              tauEt_2 = l1emu_->tauEt[c];
            }
          }

          double egISOEt_1 = 0;
          double egISOEt_2 = 0;
          //EG pt's are not given in descending order...bx?
          for (UInt_t c=0; c<l1emu_->nEGs; c++){
            if (l1emu_->egEt[c] > egISOEt_1 && l1emu_->egIso[c]==1){
              egISOEt_2 = egISOEt_1;
              egISOEt_1 = l1emu_->egEt[c];
            }
            else if (l1emu_->egEt[c] <= egISOEt_1 && l1emu_->egEt[c] > egISOEt_2 && l1emu_->egIso[c]==1){
              egISOEt_2 = l1emu_->egEt[c];
            }
          }

          double tauISOEt_1 = 0;
          double tauISOEt_2 = 0;
          //tau pt's are not given in descending order
          for (UInt_t c=0; c<l1emu_->nTaus; c++){
            if (l1emu_->tauEt[c] > tauISOEt_1 && l1emu_->tauIso[c]>0){
              tauISOEt_2 = tauISOEt_1;
              tauISOEt_1 = l1emu_->tauEt[c];
            }
            else if (l1emu_->tauEt[c] <= tauISOEt_1 && l1emu_->tauEt[c] > tauISOEt_2 && l1emu_->tauIso[c]>0){
              tauISOEt_2 = l1emu_->tauEt[c];
            }
          }

          double htSum(0.0);
          double mhtSum(0.0);
          double etSum(0.0);
          double metSum(0.0);
          double metHFSum(0.0);
          for (unsigned int c=0; c<l1emu_->nSums; c++){
              if( l1emu_->sumBx[c] != 0 ) continue;
              if( l1emu_->sumType[c] == L1Analysis::kTotalEt ) etSum = l1emu_->sumEt[c];
              if( l1emu_->sumType[c] == L1Analysis::kTotalHt ) htSum = l1emu_->sumEt[c];
              if( l1emu_->sumType[c] == L1Analysis::kMissingEt ) metSum = l1emu_->sumEt[c];
	          if( l1emu_->sumType[c] == L1Analysis::kMissingEtHF ) metHFSum = l1emu_->sumEt[c];
              if( l1emu_->sumType[c] == L1Analysis::kMissingHt ) mhtSum = l1emu_->sumEt[c];
          }
          htSum_emu->Fill(htSum, nPV);
          mhtSum_emu->Fill(mhtSum, nPV);
          etSum_emu->Fill(etSum, nPV);
          metSum_emu->Fill(metSum, nPV);
          metHFSum_emu->Fill(metHFSum, nPV);

          for(int bin=0; bin<nJetBins; bin++){
            if( (jetEt_1 ) >= jetLo + (bin*jetBinWidth) ) {
	      singleJetRates_emu->Fill(jetLo+(bin*jetBinWidth),nPV);  //GeV
	      for (auto sJetEtaBin : sJetEtaBins_1) {
		singleJetRates_etaCat_emu[sJetEtaBin]->Fill(jetLo+(bin*jetBinWidth),nPV);  //GeV
	      }
            }
          }

          for(int bin=0; bin<nJetBins; bin++){
            if( (jetEt_2) >= jetLo + (bin*jetBinWidth) ) {
             doubleJetRates_emu->Fill(jetLo+(bin*jetBinWidth),nPV);  //GeV
            }
          }  

          for(int bin=0; bin<nJetBins; bin++){
            if( (jetEt_3) >= jetLo + (bin*jetBinWidth) ) {
                tripleJetRates_emu->Fill(jetLo+(bin*jetBinWidth),nPV);  //GeV
            }
          }  

          for(int bin=0; bin<nJetBins; bin++){
            if( (jetEt_4) >= jetLo + (bin*jetBinWidth) ) {
                quadJetRates_emu->Fill(jetLo+(bin*jetBinWidth),nPV);  //GeV
            }
          }  
                 
          for(int bin=0; bin<nEgBins; bin++){
            if( (egEt_1) >= egLo + (bin*egBinWidth) ) {
             singleEgRates_emu->Fill(egLo+(bin*egBinWidth),nPV);  //GeV
            }
          } 

          for(int bin=0; bin<nEgBins; bin++){
            if( (egEt_2) >= egLo + (bin*egBinWidth) ) {
             doubleEgRates_emu->Fill(egLo+(bin*egBinWidth),nPV);  //GeV
            }
          }  

          for(int bin=0; bin<nTauBins; bin++){
            if( (tauEt_1) >= tauLo + (bin*tauBinWidth) ) {
             singleTauRates_emu->Fill(tauLo+(bin*tauBinWidth),nPV);  //GeV
            }
          }

          for(int bin=0; bin<nTauBins; bin++){
            if( (tauEt_2) >= tauLo + (bin*tauBinWidth) ) {
             doubleTauRates_emu->Fill(tauLo+(bin*tauBinWidth),nPV);  //GeV
            }
          } 

          for(int bin=0; bin<nEgBins; bin++){
            if( (egISOEt_1) >= egLo + (bin*egBinWidth) ) {
             singleISOEgRates_emu->Fill(egLo+(bin*egBinWidth),nPV);  //GeV
            }
          } 

          for(int bin=0; bin<nEgBins; bin++){
            if( (egISOEt_2) >= egLo + (bin*egBinWidth) ) {
             doubleISOEgRates_emu->Fill(egLo+(bin*egBinWidth),nPV);  //GeV
            }
          }  

          for(int bin=0; bin<nTauBins; bin++){
            if( (tauISOEt_1) >= tauLo + (bin*tauBinWidth) ) {
             singleISOTauRates_emu->Fill(tauLo+(bin*tauBinWidth),nPV);  //GeV
            }
          }

          for(int bin=0; bin<nTauBins; bin++){
            if( (tauISOEt_2) >= tauLo + (bin*tauBinWidth) ) {
             doubleISOTauRates_emu->Fill(tauLo+(bin*tauBinWidth),nPV);  //GeV
            }
          } 


          for(int bin=0; bin<nHtSumBins; bin++){
            if( (htSum) >= htSumLo+(bin*htSumBinWidth) ) htSumRates_emu->Fill(htSumLo+(bin*htSumBinWidth),nPV); //GeV
          }

          for(int bin=0; bin<nMhtSumBins; bin++){
            if( (mhtSum) >= mhtSumLo+(bin*mhtSumBinWidth) ) mhtSumRates_emu->Fill(mhtSumLo+(bin*mhtSumBinWidth),nPV); //GeV           
          }

          for(int bin=0; bin<nEtSumBins; bin++){
            if( (etSum) >= etSumLo+(bin*etSumBinWidth) ) etSumRates_emu->Fill(etSumLo+(bin*etSumBinWidth),nPV); //GeV           
          }

          for(int bin=0; bin<nMetSumBins; bin++){
            if( (metSum) >= metSumLo+(bin*metSumBinWidth) ) metSumRates_emu->Fill(metSumLo+(bin*metSumBinWidth),nPV); //GeV           
          }
          for(int bin=0; bin<nMetHFSumBins; bin++){
            if( (metHFSum) >= metHFSumLo+(bin*metHFSumBinWidth) ) metHFSumRates_emu->Fill(metHFSumLo+(bin*metHFSumBinWidth),nPV); //GeV           
          }


    }// closes if 'emuOn' is true


    //do routine for L1 hardware quantities
    if (hwOn){

      treeL1TPhw->GetEntry(jentry);
      double tpEt(0.);
      
      for(int i=0; i < l1TPhw_->nHCALTP; i++){
	tpEt = l1TPhw_->hcalTPet[i];
	hcalTP_hw->Fill(tpEt,nPV);
      }
      for(int i=0; i < l1TPhw_->nECALTP; i++){
	tpEt = l1TPhw_->ecalTPet[i];
	ecalTP_hw->Fill(tpEt,nPV);
      }


      treeL1hw->GetEntry(jentry);
      // get jetEt*, egEt*, tauEt, htSum, mhtSum, etSum, metSum
      // ***INCLUDES NON_ZERO bx*** can't just read values off
      double jetEt_1 = 0;
      double jetEt_2 = 0;
      double jetEt_3 = 0;
      double jetEt_4 = 0;
      double jetEta_1 = 99999.; 
      for (UInt_t c=0; c<l1hw_->nJets; c++){
        if (l1hw_->jetBx[c]==0 && l1hw_->jetEt[c] > jetEt_1){
          jetEt_4 = jetEt_3;
          jetEt_3 = jetEt_2;
          jetEt_2 = jetEt_1;
          jetEt_1 = l1hw_->jetEt[c];
	  jetEta_1 = l1hw_->jetEta[c];
        }
        else if (l1hw_->jetBx[c]==0 && l1hw_->jetEt[c] <= jetEt_1 && l1hw_->jetEt[c] > jetEt_2){
          jetEt_4 = jetEt_3;
          jetEt_3 = jetEt_2;      
          jetEt_2 = l1hw_->jetEt[c];
        }
        else if (l1hw_->jetBx[c]==0 && l1hw_->jetEt[c] <= jetEt_2 && l1hw_->jetEt[c] > jetEt_3){
          jetEt_4 = jetEt_3;     
          jetEt_3 = l1hw_->jetEt[c];
        }
        else if (l1hw_->jetBx[c]==0 && l1hw_->jetEt[c] <= jetEt_3 && l1hw_->jetEt[c] > jetEt_4){   
          jetEt_4 = l1hw_->jetEt[c];
        }
      }
      std::vector<std::string> sJetEtaBins_1; // string representing jet1 eta: HB, HE1, HE2a, HE2b, HF, HBEF (all HCAL)
      for (auto etaBin : ETA_CAT) {
	if (std::abs(jetEta_1 - 99999.) > 1e-5 && std::abs(jetEta_1) >= etaBin.second.at(0) && std::abs(jetEta_1) < etaBin.second.at(1)) {
	  sJetEtaBins_1.push_back(etaBin.first);
	}
      } 

      double egEt_1 = 0;
      double egEt_2 = 0;
      for (UInt_t c=0; c<l1hw_->nEGs; c++){
        if (l1hw_->egBx[c]==0 && l1hw_->egEt[c] > egEt_1){
          egEt_2 = egEt_1;
          egEt_1 = l1hw_->egEt[c];
        }
        else if (l1hw_->egBx[c]==0 && l1hw_->egEt[c] <= egEt_1 && l1hw_->egEt[c] > egEt_2){
          egEt_2 = l1hw_->egEt[c];
        }
      }

      double tauEt_1 = 0;
      double tauEt_2 = 0;
      //tau pt's are not given in descending order
      for (UInt_t c=0; c<l1hw_->nTaus; c++){
        if (l1hw_->tauBx[c]==0 && l1hw_->tauEt[c] > tauEt_1){
          tauEt_1 = l1hw_->tauEt[c];
        }
        else if (l1hw_->tauBx[c]==0 && l1hw_->tauEt[c] <= tauEt_1 && l1hw_->tauEt[c] > tauEt_2){
          tauEt_2 = l1hw_->tauEt[c];
        }
      }

      double egISOEt_1 = 0;
      double egISOEt_2 = 0;
      //EG pt's are not given in descending order...bx?
      for (UInt_t c=0; c<l1hw_->nEGs; c++){
        if (l1hw_->egBx[c]==0 && l1hw_->egEt[c] > egISOEt_1 && l1hw_->egIso[c]==1){
          egISOEt_2 = egISOEt_1;
          egISOEt_1 = l1hw_->egEt[c];
        }
        else if (l1hw_->egBx[c]==0 && l1hw_->egEt[c] <= egISOEt_1 && l1hw_->egEt[c] > egISOEt_2 && l1hw_->egIso[c]==1){
          egISOEt_2 = l1hw_->egEt[c];
        }
      }

      double tauISOEt_1 = 0;
      double tauISOEt_2 = 0;
      //tau pt's are not given in descending order
      for (UInt_t c=0; c<l1hw_->nTaus; c++){
        if (l1hw_->tauBx[c]==0 && l1hw_->tauEt[c] > tauISOEt_1 && l1hw_->tauIso[c]>0){
          tauISOEt_2 = tauISOEt_1;
          tauISOEt_1 = l1hw_->tauEt[c];
        }
        else if (l1hw_->tauBx[c]==0 && l1hw_->tauEt[c] <= tauISOEt_1 && l1hw_->tauEt[c] > tauISOEt_2 && l1hw_->tauIso[c]>0){
          tauISOEt_2 = l1hw_->tauEt[c];
        }
      }

      double htSum = 0;
      double mhtSum = 0;
      double etSum = 0;
      double metSum = 0;
      double metHFSum = 0;
      // HW includes -2,-1,0,1,2 bx info (hence the different numbers, could cause a seg fault if this changes)
      for (unsigned int c=0; c<l1hw_->nSums; c++){
          if( l1hw_->sumBx[c] != 0 ) continue;
          if( l1hw_->sumType[c] == L1Analysis::kTotalEt ) etSum = l1hw_->sumEt[c];
          if( l1hw_->sumType[c] == L1Analysis::kTotalHt ) htSum = l1hw_->sumEt[c];
          if( l1hw_->sumType[c] == L1Analysis::kMissingEt ) metSum = l1hw_->sumEt[c];
	      if( l1hw_->sumType[c] == L1Analysis::kMissingEtHF ) metHFSum = l1hw_->sumEt[c];
          if( l1hw_->sumType[c] == L1Analysis::kMissingHt ) mhtSum = l1hw_->sumEt[c];
      }

      // for each bin fill according to whether our object has a larger corresponding energy
      for(int bin=0; bin<nJetBins; bin++){
        if( (jetEt_1) >= jetLo + (bin*jetBinWidth) ) {
	  singleJetRates_hw->Fill(jetLo+(bin*jetBinWidth),nPV);  //GeV
	  for (auto sJetEtaBin : sJetEtaBins_1) {
	    singleJetRates_etaCat_hw[sJetEtaBin]->Fill(jetLo+(bin*jetBinWidth),nPV);  //GeV
	  }
	}
      } 

      for(int bin=0; bin<nJetBins; bin++){
        if( (jetEt_2) >= jetLo + (bin*jetBinWidth) ) doubleJetRates_hw->Fill(jetLo+(bin*jetBinWidth),nPV);  //GeV
      }  

      for(int bin=0; bin<nJetBins; bin++){
        if( (jetEt_3) >= jetLo + (bin*jetBinWidth) ) tripleJetRates_hw->Fill(jetLo+(bin*jetBinWidth),nPV);  //GeV
      }  

      for(int bin=0; bin<nJetBins; bin++){
        if( (jetEt_4) >= jetLo + (bin*jetBinWidth) ) quadJetRates_hw->Fill(jetLo+(bin*jetBinWidth),nPV);  //GeV
      }  
             
      for(int bin=0; bin<nEgBins; bin++){
        if( (egEt_1) >= egLo + (bin*egBinWidth) ) singleEgRates_hw->Fill(egLo+(bin*egBinWidth),nPV);  //GeV
      } 

      for(int bin=0; bin<nEgBins; bin++){
        if( (egEt_2) >= egLo + (bin*egBinWidth) ) doubleEgRates_hw->Fill(egLo+(bin*egBinWidth),nPV);  //GeV
      }  

      for(int bin=0; bin<nTauBins; bin++){
        if( (tauEt_1) >= tauLo + (bin*tauBinWidth) ) singleTauRates_hw->Fill(tauLo+(bin*tauBinWidth),nPV);  //GeV
      } 

      for(int bin=0; bin<nTauBins; bin++){
        if( (tauEt_2) >= tauLo + (bin*tauBinWidth) ) doubleTauRates_hw->Fill(tauLo+(bin*tauBinWidth),nPV);  //GeV
      } 

      for(int bin=0; bin<nEgBins; bin++){
        if( (egISOEt_1) >= egLo + (bin*egBinWidth) ) singleISOEgRates_hw->Fill(egLo+(bin*egBinWidth),nPV);  //GeV
      } 

      for(int bin=0; bin<nEgBins; bin++){
        if( (egISOEt_2) >= egLo + (bin*egBinWidth) ) doubleISOEgRates_hw->Fill(egLo+(bin*egBinWidth),nPV);  //GeV
      }  

      for(int bin=0; bin<nTauBins; bin++){
        if( (tauISOEt_1) >= tauLo + (bin*tauBinWidth) ) singleISOTauRates_hw->Fill(tauLo+(bin*tauBinWidth),nPV);  //GeV
      }

      for(int bin=0; bin<nTauBins; bin++){
        if( (tauISOEt_2) >= tauLo + (bin*tauBinWidth) ) doubleISOTauRates_hw->Fill(tauLo+(bin*tauBinWidth),nPV);  //GeV
      } 

      for(int bin=0; bin<nHtSumBins; bin++){
        if( (htSum) >= htSumLo+(bin*htSumBinWidth) ) htSumRates_hw->Fill(htSumLo+(bin*htSumBinWidth),nPV); //GeV
      }

      for(int bin=0; bin<nMhtSumBins; bin++){
        if( (mhtSum) >= mhtSumLo+(bin*mhtSumBinWidth) ) mhtSumRates_hw->Fill(mhtSumLo+(bin*mhtSumBinWidth),nPV); //GeV           
      }

      for(int bin=0; bin<nEtSumBins; bin++){
        if( (etSum) >= etSumLo+(bin*etSumBinWidth) ) etSumRates_hw->Fill(etSumLo+(bin*etSumBinWidth),nPV); //GeV           
      }

      for(int bin=0; bin<nMetSumBins; bin++){
        if( (metSum) >= metSumLo+(bin*metSumBinWidth) ) metSumRates_hw->Fill(metSumLo+(bin*metSumBinWidth),nPV); //GeV           
      } 
      for(int bin=0; bin<nMetHFSumBins; bin++){
        if( (metHFSum) >= metHFSumLo+(bin*metHFSumBinWidth) ) metHFSumRates_hw->Fill(metHFSumLo+(bin*metHFSumBinWidth),nPV); //GeV           
      } 

    }// closes if 'hwOn' is true

  }// closes loop through events

  //  TFile g( outputFilename.c_str() , "new");
  kk->cd();
  // normalisation factor for rate histograms (11kHz is the orbit frequency)
  // double norm = 11246*(numBunch/goodLumiEventCount); // no lumi rescale
  //  double norm = 11246*(numBunch/goodLumiEventCount)*(expectedLum/runLum); //scale to nominal lumi
  //  Run3 normalization ==> inst lumi * min bias xsec / <PU>
  double norm = instLumi * mbXSec / nentries;
  if (toCheckWithJosnFile) norm = instLumi * mbXSec / goodLumiEventCount;
  
  if (emuOn){
    singleJetRates_emu->Scale(norm);
    doubleJetRates_emu->Scale(norm);
    tripleJetRates_emu->Scale(norm);
    quadJetRates_emu->Scale(norm);
    singleEgRates_emu->Scale(norm);
    doubleEgRates_emu->Scale(norm);
    singleTauRates_emu->Scale(norm);
    doubleTauRates_emu->Scale(norm);
    singleISOEgRates_emu->Scale(norm);
    doubleISOEgRates_emu->Scale(norm);
    singleISOTauRates_emu->Scale(norm);
    doubleISOTauRates_emu->Scale(norm);
    htSumRates_emu->Scale(norm);
    mhtSumRates_emu->Scale(norm);
    etSumRates_emu->Scale(norm);
    metSumRates_emu->Scale(norm);
    metHFSumRates_emu->Scale(norm);
    for (auto etaBin : ETA_CAT) {
      std::string sEtaBin = etaBin.first;
      singleJetRates_etaCat_emu[sEtaBin]->Scale(norm);
    }

    htSum_emu->Write();
    mhtSum_emu->Write();
    etSum_emu->Write();
    metSum_emu->Write();
    metHFSum_emu->Write();

    //set the errors for the rates
    //want error -> error * sqrt(norm) ?

    hcalTP_emu->Write();
    ecalTP_emu->Write();

    singleJetRates_emu->Write();
    doubleJetRates_emu->Write();
    tripleJetRates_emu->Write();
    quadJetRates_emu->Write();
    singleEgRates_emu->Write();
    doubleEgRates_emu->Write();
    singleTauRates_emu->Write();
    doubleTauRates_emu->Write();
    singleISOEgRates_emu->Write();
    doubleISOEgRates_emu->Write();
    singleISOTauRates_emu->Write();
    doubleISOTauRates_emu->Write();

    htSumRates_emu->Write();
    mhtSumRates_emu->Write();
    etSumRates_emu->Write();
    metSumRates_emu->Write();
    metHFSumRates_emu->Write();

    for (auto etaBin : ETA_CAT) {
      std::string sEtaBin = etaBin.first;
      singleJetRates_etaCat_emu[sEtaBin]->Write();
    }    
  }

  if (hwOn){

    singleJetRates_hw->Scale(norm);
    doubleJetRates_hw->Scale(norm);
    tripleJetRates_hw->Scale(norm);
    quadJetRates_hw->Scale(norm);
    singleEgRates_hw->Scale(norm);
    doubleEgRates_hw->Scale(norm);
    singleTauRates_hw->Scale(norm);
    doubleTauRates_hw->Scale(norm);
    singleISOEgRates_hw->Scale(norm);
    doubleISOEgRates_hw->Scale(norm);
    singleISOTauRates_hw->Scale(norm);
    doubleISOTauRates_hw->Scale(norm);
    htSumRates_hw->Scale(norm);
    mhtSumRates_hw->Scale(norm);
    etSumRates_hw->Scale(norm);
    metSumRates_hw->Scale(norm);
    metHFSumRates_hw->Scale(norm);
    for (auto etaBin : ETA_CAT) {
      std::string sEtaBin = etaBin.first;
      singleJetRates_etaCat_hw[sEtaBin]->Scale(norm);
    }
    
    hcalTP_hw->Write();
    ecalTP_hw->Write();
    singleJetRates_hw->Write();
    doubleJetRates_hw->Write();
    tripleJetRates_hw->Write();
    quadJetRates_hw->Write();
    singleEgRates_hw->Write();
    doubleEgRates_hw->Write();
    singleTauRates_hw->Write();
    doubleTauRates_hw->Write();
    singleISOEgRates_hw->Write();
    doubleISOEgRates_hw->Write();
    singleISOTauRates_hw->Write();
    doubleISOTauRates_hw->Write();
    htSumRates_hw->Write();
    mhtSumRates_hw->Write();
    etSumRates_hw->Write();
    metSumRates_hw->Write();
    metHFSumRates_hw->Write();
    for (auto etaBin : ETA_CAT) {
      std::string sEtaBin = etaBin.first;
      singleJetRates_etaCat_hw[sEtaBin]->Write();
    }
  }
  myfile << "using the following ntuple: " << inputFile << std::endl;
  myfile << "number of colliding bunches = " << numBunch << std::endl;
  myfile << "run luminosity = " << runLum << std::endl;
  myfile << "expected luminosity = " << expectedLum << std::endl;
  myfile << "norm factor used = " << norm << std::endl;
  myfile << "number of total events = " << nentries << std::endl;  
  myfile << "number of good events = " << goodLumiEventCount << std::endl;
  myfile.close();

  printf("\nDetails wrote to %s file\n",outputTxtFilename.c_str());
}//closes the function 'rates'
