// Script for calculating efficiency and resolution histograms
#include "TMath.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TChain.h"
#include <iostream>
#include <fstream>
#include <string>
#include "L1Trigger/L1TNtuples/interface/L1AnalysisEventDataFormat.h"
#include "L1Trigger/L1TNtuples/interface/L1AnalysisL1UpgradeDataFormat.h"
#include "L1Trigger/L1TNtuples/interface/L1AnalysisRecoVertexDataFormat.h"
#include "L1Trigger/L1TNtuples/interface/L1AnalysisCaloTPDataFormat.h"

#include "L1Trigger/L1TNtuples/interface/L1AnalysisRecoJetDataFormat.h"
#include "L1Trigger/L1TNtuples/interface/L1AnalysisRecoMetDataFormat.h"
#include "L1Trigger/L1TNtuples/interface/L1AnalysisRecoMuon2DataFormat.h"
#include "L1Trigger/L1TNtuples/interface/L1AnalysisRecoMetFilterDataFormat.h"

#include "jsoncpp.cpp"
#include <glob.h> // glob(), globfree()
#include <vector>
#include <stdexcept>
#include <sstream>
#include <sys/stat.h>

/* TODO: put errors in rates...
creates the rates and distributions for l1 trigger objects
How to use:
1. input the number of bunches in the run (~line 35)
2. change the variables "newConditionsNtuples" and "oldConditionsNtuples" to ntuple paths
3. If good run JSON is not applied during ntuple production, modify isGoodLumiSection()

Optionally, if you want to rescale to a given instantaneous luminosity:
1. input the instantaneous luminosity of the run (~line 32) [only if we scale to 2016 nominal]
2. select whether you rescale to L=1.5e34 (~line606??...) generally have it setup to rescale
nb: for 2&3 I have provided the info in runInfoForRates.txt
*/

bool toCheckWithJosnFile = false; // check goodLumiSections if not done while producing l1tNtuples
std::string goodLumiSectionJSONFile = "/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions18/13TeV/PromptReco/Cert_314472-325175_13TeV_PromptReco_Collisions18_JSON.txt"; // 2018 promptReco
bool isMC = false;
long l1NTuple_FileSizeThrsh = 0; //45000; // in bytes
bool useRecoMuon_hlt_isomu_Cut = false;

void jetanalysis(bool newConditions, const std::string& inputFileDirectory);

int main(int argc, char *argv[])
{
    bool newConditions = true;
    std::string ntuplePath("");

    if (argc != 3) {
        std::cout << "Usage: l1jetanalysis.exe [new/def] [path to ntuples]\n"
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

    jetanalysis(newConditions, ntuplePath);

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
    return false;
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

double deltaPhi(double phi1, double phi2) {
    double result = phi1 - phi2;
    if(fabs(result) > 9999) return result;
    while (result > TMath::Pi()) result -= 2*TMath::Pi();
    while (result <= -TMath::Pi()) result += 2*TMath::Pi();
    return result;
}

double deltaR(double eta1, double phi1, double eta2, double phi2) {
    double deta = eta1 - eta2;
    double dphi = deltaPhi(phi1, phi2);
    return sqrt(deta*deta + dphi*dphi);
}

std::vector<std::string> glob(const std::string& pattern) {
    using namespace std;

    // glob struct resides on the stack
    glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));

    // do the glob operation
    int return_value = glob(pattern.c_str(), GLOB_TILDE, NULL, &glob_result);
    if(return_value != 0) {
        globfree(&glob_result);
        stringstream ss;
        ss << "glob() failed with return_value " << return_value << endl;
        throw std::runtime_error(ss.str());
    }

    // collect all the filenames into a std::list<std::string>
    vector<string> filenames;
    for(size_t i = 0; i < glob_result.gl_pathc; ++i) {
        filenames.push_back(string(glob_result.gl_pathv[i]));
    }

    // cleanup
    globfree(&glob_result);

    // done
    return filenames;
}

long GetFileSize(std::string filename)
{
    struct stat stat_buf; 
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}


void jetanalysis(bool newConditions, const std::string& inputFileDirectory) {
  
  bool hwOn = true;   //are we using data from hardware? (upgrade trigger had to be running!!!)
  bool emuOn = true;  //are we using data from emulator?
  //for efficiencies & resolutions:    
  bool recoOn = true; //are we using reco quantities? 
    
  if (hwOn==false && emuOn==false){
    std::cout << "exiting as neither hardware or emulator selected" << std::endl;
    return;
  }

  std::string inputFile(inputFileDirectory);
  inputFile += "/L1Ntuple_*.root";
  std::string outputDirectory = "emu";  //***runNumber, triggerType, version, hw/emu/both***MAKE SURE IT EXISTS
  std::string outputFilename = "l1analysis_def.root";
  if(newConditions) outputFilename = "l1analysis_new_cond.root";
  TFile* kk = TFile::Open( outputFilename.c_str() , "recreate");
  // if (kk!=0){
  //   cout << "TERMINATE: not going to overwrite file " << outputFilename << endl;
  //   return;
  // } 


  TChain * treeL1emu = new TChain("l1UpgradeEmuTree/L1UpgradeTree");
  TChain * treeL1hw = new TChain("l1UpgradeTree/L1UpgradeTree");
  TChain * eventTree = new TChain("l1EventTree/L1EventTree");
  // TChain * vtxTree = new TChain("l1RecoTree/RecoTree"); // In case you want to include PU info
  TChain * treeL1TPemu = new TChain("l1CaloTowerEmuTree/L1CaloTowerTree");
  TChain * treeL1TPhw = new TChain("l1CaloTowerTree/L1CaloTowerTree");

  // In case you want to include RECO info
  TChain * recoTree = new TChain("l1JetRecoTree/JetRecoTree");
  TChain * metfilterTree = new TChain("l1MetFilterRecoTree/MetFilterRecoTree");
  TChain * muonTree = new TChain("l1MuonRecoTree/Muon2RecoTree");

  std::cout << "i/p trees: " << inputFile << std::endl;
  std::vector<std::string> vInputFiles = glob(inputFile);
  std::cout << "no. of files: " << vInputFiles.size();
  for (size_t i=0; i<vInputFiles.size(); i++) {
    long fileSize = GetFileSize(vInputFiles[i]);
    //printf("\t %lu %ld %s\n",i,fileSize,vInputFiles[i].c_str());
    if (fileSize < l1NTuple_FileSizeThrsh) {
      //printf("\t\t\t fileSize (%ld) < l1NTuple_FileSizeThrsh (%ld)",fileSize, l1NTuple_FileSizeThrsh);
      continue;
    }

    std::string file(vInputFiles[i]);
    if (emuOn){
      treeL1emu->Add(file.c_str());
    }
    if (hwOn){
      treeL1hw->Add(file.c_str());
    }
    eventTree->Add(file.c_str());

    // In case you want to include PU info
    // if(binByPileUp){
    //   vtxTree->Add(file.c_str());
    // }

    if (emuOn){
      treeL1TPemu->Add(file.c_str());
    }

    if (hwOn){
      treeL1TPhw->Add(file.c_str());
    }

    if (recoOn) {
      recoTree->Add(file.c_str());
      metfilterTree->Add(file.c_str());
      muonTree->Add(file.c_str());
    }
    
  }
    

  /*
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

  // In case you want to include RECO info
  TChain * recoTree = new TChain("l1JetRecoTree/JetRecoTree");
  TChain * metfilterTree = new TChain("l1MetFilterRecoTree/MetFilterRecoTree");
  TChain * muonTree = new TChain("l1MuonRecoTree/Muon2RecoTree");
  if (recoOn) {
  recoTree->Add(inputFile.c_str());
  metfilterTree->Add(inputFile.c_str());
  muonTree->Add(inputFile.c_str());
  }

  TChain * treeL1TPemu = new TChain("l1CaloTowerEmuTree/L1CaloTowerTree");
  if (emuOn){
  treeL1TPemu->Add(inputFile.c_str());
  }

  TChain * treeL1TPhw = new TChain("l1CaloTowerTree/L1CaloTowerTree");
  if (hwOn){
  treeL1TPhw->Add(inputFile.c_str());
  }
  */

  L1Analysis::L1AnalysisL1UpgradeDataFormat    *l1emu_ = new L1Analysis::L1AnalysisL1UpgradeDataFormat();
  treeL1emu->SetBranchAddress("L1Upgrade", &l1emu_);
  L1Analysis::L1AnalysisL1UpgradeDataFormat    *l1hw_ = new L1Analysis::L1AnalysisL1UpgradeDataFormat();
  treeL1hw->SetBranchAddress("L1Upgrade", &l1hw_);
  L1Analysis::L1AnalysisEventDataFormat    *event_ = new L1Analysis::L1AnalysisEventDataFormat();
  eventTree->SetBranchAddress("Event", &event_);

  L1Analysis::L1AnalysisRecoJetDataFormat    *jet_ = new L1Analysis::L1AnalysisRecoJetDataFormat();
  recoTree->SetBranchAddress("Jet", &jet_);
  L1Analysis::L1AnalysisRecoMuon2DataFormat    *muon_ = new L1Analysis::L1AnalysisRecoMuon2DataFormat();
  muonTree->SetBranchAddress("Muon", &muon_);
  L1Analysis::L1AnalysisRecoMetDataFormat    *met_ = new L1Analysis::L1AnalysisRecoMetDataFormat();
  recoTree->SetBranchAddress("Sums", &met_);
  L1Analysis::L1AnalysisRecoMetFilterDataFormat    *metfilter_ = new L1Analysis::L1AnalysisRecoMetFilterDataFormat();
  metfilterTree->SetBranchAddress("MetFilters", &metfilter_);
    
  L1Analysis::L1AnalysisCaloTPDataFormat    *l1TPemu_ = new L1Analysis::L1AnalysisCaloTPDataFormat();
  treeL1TPemu->SetBranchAddress("CaloTP", &l1TPemu_);
  L1Analysis::L1AnalysisCaloTPDataFormat    *l1TPhw_ = new L1Analysis::L1AnalysisCaloTPDataFormat();
  treeL1TPhw->SetBranchAddress("CaloTP", &l1TPhw_);


  // get number of entries
  Long64_t nentries;
  if (emuOn) nentries = treeL1emu->GetEntries();
  else nentries = treeL1hw->GetEntries();
  int goodLumiEventCount = 0; 
  int  nEventsSelectedLevel1 = 0;
  int  nEventsSelectedLevel2 = 0;
  int  nEventsSelectedLevel3 = 0;
  int  nEventsSelectedLevel4 = 0;
  int  nEventsSelectedLevel5 = 0;
  int  nEventsSelectedLevel6 = 0;
    
  eventTree->GetEntry(0);

  // set parameters for histograms
  // jet bins
  int nJetBins = 500;
  float jetLo = 0.;
  float jetHi = 500.;
  //  float jetBinWidth = (jetHi-jetLo)/nJetBins;

  // htSum bins
  int nHtSumBins = 800;
  float htSumLo = 0.;
  float htSumHi = 800.;
  // float htSumBinWidth = (htSumHi-htSumLo)/nHtSumBins;

  // mhtSum bins
  int nMhtSumBins = 500;
  float mhtSumLo = 0.;
  float mhtSumHi = 500.;
  //  float mhtSumBinWidth = (mhtSumHi-mhtSumLo)/nMhtSumBins;

  // etSum bins
  int nEtSumBins = 1000;
  float etSumLo = 0.;
  float etSumHi = 1000.;
  // float etSumBinWidth = (etSumHi-etSumLo)/nEtSumBins;

  // metSum bins
  int nMetSumBins = 500;
  float metSumLo = 0.;
  float metSumHi = 500.;
  //  float metSumBinWidth = (metSumHi-metSumLo)/nMetSumBins;

  // tp bins
  int nTpBins = 100;
  float tpLo = 0.;
  float tpHi = 100.;

  // nPV bins
  int nPVbins = 101;
  float pvLow = -0.5;
  float pvHi  = 100.5;

  std::string axR = ";Threshold E_{T} (GeV);nPV;rate (Hz)";
  std::string axD = ";E_{T} (GeV);nPV;Events / bin";
  std::string metD = ";MET (GeV);nPV;Events / bin";
    
  //make histos
  // TH1F *NCenJets = new TH1F("NCenJets","n",20,-0.5,19.5);

  std::vector<double> jetThresholds; std::vector<double> metThresholds;
  jetThresholds = {12.0, 35.0, 60.0, 90.0, 120.0, 180.0};
  metThresholds = {50.0, 100.0, 120.0, 180.0};

  // Jets
  TH2F *refJetET_Incl_emu = new TH2F("RefJet_Incl_emu", "all Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *refmJetET_Incl_emu = new TH2F("RefmJet_Incl_emu", "all matched Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);

  TH2F *refJetET_HB_emu = new TH2F("RefJet_HB_emu", "all Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *refmJetET_HB_emu = new TH2F("RefmJet_HB_emu", "all matched Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);

  TH2F *refJetET_HE1_emu = new TH2F("RefJet_HE1_emu", "all Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *refmJetET_HE1_emu = new TH2F("RefmJet_HE1_emu", "all matched Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
    
  TH2F *refJetET_HE2_emu = new TH2F("RefJet_HE2_emu", "all Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *refmJetET_HE2_emu = new TH2F("RefmJet_HE2_emu", "all matched Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);

  TH2F *refJetET_HE_emu = new TH2F("RefJet_HE_emu", "all Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *refmJetET_HE_emu = new TH2F("RefmJet_HE_emu", "all matched Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);

  TH2F *jetET12_emu   = new TH2F( "JetEt12_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET35_emu   = new TH2F( "JetEt35_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET60_emu   = new TH2F( "JetEt60_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET90_emu   = new TH2F( "JetEt90_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET120_emu  = new TH2F( "JetEt120_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET180_emu  = new TH2F( "JetEt180_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);

  TH2F *jetET12_HB_emu   = new TH2F( "JetEt12_HB_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET35_HB_emu   = new TH2F( "JetEt35_HB_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET60_HB_emu   = new TH2F( "JetEt60_HB_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET90_HB_emu   = new TH2F( "JetEt90_HB_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET120_HB_emu  = new TH2F( "JetEt120_HB_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET180_HB_emu  = new TH2F( "JetEt180_HB_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);

  TH2F *jetET12_HE1_emu   = new TH2F( "JetEt12_HE1_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET35_HE1_emu   = new TH2F( "JetEt35_HE1_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET60_HE1_emu   = new TH2F( "JetEt60_HE1_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET90_HE1_emu   = new TH2F( "JetEt90_HE1_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET120_HE1_emu  = new TH2F( "JetEt120_HE1_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET180_HE1_emu  = new TH2F( "JetEt180_HE1_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);

  TH2F *jetET12_HE2_emu   = new TH2F( "JetEt12_HE2_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET35_HE2_emu   = new TH2F( "JetEt35_HE2_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET60_HE2_emu   = new TH2F( "JetEt60_HE2_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET90_HE2_emu   = new TH2F( "JetEt90_HE2_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET120_HE2_emu  = new TH2F( "JetEt120_HE2_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET180_HE2_emu  = new TH2F( "JetEt180_HE2_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);

  TH2F *jetET12_HE_emu   = new TH2F( "JetEt12_HE_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET35_HE_emu   = new TH2F( "JetEt35_HE_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET60_HE_emu   = new TH2F( "JetEt60_HE_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET90_HE_emu   = new TH2F( "JetEt90_HE_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET120_HE_emu  = new TH2F( "JetEt120_HE_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET180_HE_emu  = new TH2F( "JetEt180_HE_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
   
  TH2F *l1jetET1_emu = new TH2F( "singleJet_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *l1jetET2_emu = new TH2F( "doubleJet_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *l1jetET3_emu = new TH2F( "tripleJet_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *l1jetET4_emu = new TH2F( "quadJet_emu" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);

  TH2F *l1ET_emu = new TH2F("etSum_emu","L1 sumET (GeV)",nEtSumBins,etSumLo,etSumHi, nPVbins, pvLow, pvHi);
  TH2F *l1MET_emu = new TH2F("metSum_emu","L1 MET (GeV)",nMetSumBins,metSumLo,metSumHi, nPVbins, pvLow, pvHi);
  TH2F *l1METHF_emu = new TH2F("metHFSum_emu","L1 METHF (GeV)",nMetSumBins,metSumLo,metSumHi, nPVbins, pvLow, pvHi);
  TH2F *l1HT_emu = new TH2F("htSum_emu","L1 HT (GeV)",nHtSumBins,htSumLo,htSumHi, nPVbins, pvLow, pvHi);
  TH2F *l1MHT_emu = new TH2F("mhtSum_emu","L1 MHT (GeV)",nMhtSumBins,mhtSumLo,mhtSumHi, nPVbins, pvLow, pvHi);

  // and Sums
  TH2F *refMET_Calo_emu  = new TH2F("RefMET_Calo_emu",metD.c_str(), nMetSumBins, metSumLo, metSumHi, nPVbins, pvLow, pvHi);
  TH2F *MET_150U_Calo_emu = new TH2F("MET150_Calo_emu",metD.c_str(), nMetSumBins, metSumLo, metSumHi, nPVbins, pvLow, pvHi);
  TH2F *MET_120U_Calo_emu  = new TH2F("MET120_Calo_emu",metD.c_str(), nMetSumBins, metSumLo, metSumHi, nPVbins, pvLow, pvHi);
  TH2F *MET_100U_Calo_emu  = new TH2F("MET100_Calo_emu",metD.c_str(), nMetSumBins, metSumLo, metSumHi, nPVbins, pvLow, pvHi);
  TH2F *MET_50U_Calo_emu  = new TH2F("MET50_Calo_emu",metD.c_str(), nMetSumBins, metSumLo, metSumHi, nPVbins, pvLow, pvHi);

  TH2F *refMET_PF_emu  = new TH2F("RefMET_PF_emu",metD.c_str(), nMetSumBins, metSumLo, metSumHi, nPVbins, pvLow, pvHi);
  TH2F *MET_150U_PF_emu = new TH2F("MET150_PF_emu",metD.c_str(), nMetSumBins, metSumLo, metSumHi, nPVbins, pvLow, pvHi);
  TH2F *MET_120U_PF_emu  = new TH2F("MET120_PF_emu",metD.c_str(), nMetSumBins, metSumLo, metSumHi, nPVbins, pvLow, pvHi);
  TH2F *MET_100U_PF_emu  = new TH2F("MET100_PF_emu",metD.c_str(), nMetSumBins, metSumLo, metSumHi, nPVbins, pvLow, pvHi);
  TH2F *MET_50U_PF_emu  = new TH2F("MET50_PF_emu",metD.c_str(), nMetSumBins, metSumLo, metSumHi, nPVbins, pvLow, pvHi);


  // resolution histograms
  TH3F *hresMET_PF_emu = new TH3F("hResMET_PF_emu","",nMetSumBins,metSumLo,metSumHi,100,-5,5,nPVbins,pvLow,pvHi);

  TH2F *h_resMET1_PF_emu = new TH2F("hresMET1_PF_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET2_PF_emu = new TH2F("hresMET2_PF_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET3_PF_emu = new TH2F("hresMET3_PF_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET4_PF_emu = new TH2F("hresMET4_PF_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET5_PF_emu = new TH2F("hresMET5_PF_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET6_PF_emu = new TH2F("hresMET6_PF_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET7_PF_emu = new TH2F("hresMET7_PF_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET8_PF_emu = new TH2F("hresMET8_PF_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET9_PF_emu = new TH2F("hresMET9_PF_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET10_PF_emu = new TH2F("hresMET10_PF_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
    
  TH3F *hresMET_Calo_emu = new TH3F("hResMET_Calo_emu","",nMetSumBins,metSumLo,metSumHi,100,-5,5,nPVbins,pvLow,pvHi);

  TH2F *h_resMET1_Calo_emu = new TH2F("hresMET1_Calo_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET2_Calo_emu = new TH2F("hresMET2_Calo_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET3_Calo_emu = new TH2F("hresMET3_Calo_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET4_Calo_emu = new TH2F("hresMET4_Calo_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET5_Calo_emu = new TH2F("hresMET5_Calo_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET6_Calo_emu = new TH2F("hresMET6_Calo_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET7_Calo_emu = new TH2F("hresMET7_Calo_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET8_Calo_emu = new TH2F("hresMET8_Calo_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET9_Calo_emu = new TH2F("hresMET9_Calo_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET10_Calo_emu = new TH2F("hresMET10_Calo_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;

  TH3F *hresJet_HB_emu = new TH3F("hresJet_HB_emu","",nJetBins, jetLo, jetHi,100,-5,5,nPVbins,pvLow,pvHi);
  TH3F *hresJet_HE1_emu = new TH3F("hresJet_HE1_emu","",nJetBins, jetLo, jetHi,100,-5,5,nPVbins,pvLow,pvHi);
  TH3F *hresJet_HE2_emu = new TH3F("hresJet_HE2_emu","",nJetBins, jetLo, jetHi,100,-5,5,nPVbins,pvLow,pvHi);
  TH3F *hresJet_HE_emu = new TH3F("hresJet_HE_emu","",nJetBins, jetLo, jetHi,100,-5,5,nPVbins,pvLow,pvHi);
  TH3F *hresJet_Incl_emu = new TH3F("hresJet_Incl_emu","",nJetBins, jetLo, jetHi,100,-5,5,nPVbins,pvLow,pvHi);

  TH2F *h_resJet1_emu = new TH2F("hresJet1_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resJet2_emu = new TH2F("hresJet2_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resJet3_emu = new TH2F("hresJet3_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resJet4_emu = new TH2F("hresJet4_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resJet5_emu = new TH2F("hresJet5_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resJet6_emu = new TH2F("hresJet6_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resJet7_emu = new TH2F("hresJet7_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resJet8_emu = new TH2F("hresJet8_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resJet9_emu = new TH2F("hresJet9_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resJet10_emu = new TH2F("hresJet10_emu","",100,-5,5, nPVbins, pvLow, pvHi) ;
      
  // hcal/ecal TPs 
  TH2F* hcalTP_emu = new TH2F("hcalTP_emu", ";TP E_{T}; # Entries", nTpBins, tpLo, tpHi, nPVbins, pvLow, pvHi);
  TH2F* ecalTP_emu = new TH2F("ecalTP_emu", ";TP E_{T}; # Entries", nTpBins, tpLo, tpHi, nPVbins, pvLow, pvHi);


  // hardware trigger ---------
  // Jets
  TH2F *refJetET_Incl_hw = new TH2F("RefJet_Incl_hw", "all Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *refmJetET_Incl_hw = new TH2F("RefmJet_Incl_hw", "all matched Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);

  TH2F *refJetET_HB_hw = new TH2F("RefJet_HB_hw", "all Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *refmJetET_HB_hw = new TH2F("RefmJet_HB_hw", "all matched Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);

  TH2F *refJetET_HE1_hw = new TH2F("RefJet_HE1_hw", "all Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *refmJetET_HE1_hw = new TH2F("RefmJet_HE1_hw", "all matched Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
    
  TH2F *refJetET_HE2_hw = new TH2F("RefJet_HE2_hw", "all Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *refmJetET_HE2_hw = new TH2F("RefmJet_HE2_hw", "all matched Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);

  TH2F *refJetET_HE_hw = new TH2F("RefJet_HE_hw", "all Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *refmJetET_HE_hw = new TH2F("RefmJet_HE_hw", "all matched Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);

  TH2F *jetET12_hw   = new TH2F( "JetEt12_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET35_hw   = new TH2F( "JetEt35_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET60_hw   = new TH2F( "JetEt60_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET90_hw   = new TH2F( "JetEt90_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET120_hw  = new TH2F( "JetEt120_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET180_hw  = new TH2F( "JetEt180_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);

  TH2F *jetET12_HB_hw   = new TH2F( "JetEt12_HB_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET35_HB_hw   = new TH2F( "JetEt35_HB_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET60_HB_hw   = new TH2F( "JetEt60_HB_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET90_HB_hw   = new TH2F( "JetEt90_HB_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET120_HB_hw  = new TH2F( "JetEt120_HB_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET180_HB_hw  = new TH2F( "JetEt180_HB_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);

  TH2F *jetET12_HE1_hw   = new TH2F( "JetEt12_HE1_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET35_HE1_hw   = new TH2F( "JetEt35_HE1_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET60_HE1_hw   = new TH2F( "JetEt60_HE1_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET90_HE1_hw   = new TH2F( "JetEt90_HE1_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET120_HE1_hw  = new TH2F( "JetEt120_HE1_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET180_HE1_hw  = new TH2F( "JetEt180_HE1_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);

  TH2F *jetET12_HE2_hw   = new TH2F( "JetEt12_HE2_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET35_HE2_hw   = new TH2F( "JetEt35_HE2_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET60_HE2_hw   = new TH2F( "JetEt60_HE2_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET90_HE2_hw   = new TH2F( "JetEt90_HE2_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET120_HE2_hw  = new TH2F( "JetEt120_HE2_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET180_HE2_hw  = new TH2F( "JetEt180_HE2_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);

  TH2F *jetET12_HE_hw   = new TH2F( "JetEt12_HE_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET35_HE_hw   = new TH2F( "JetEt35_HE_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET60_HE_hw   = new TH2F( "JetEt60_HE_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET90_HE_hw   = new TH2F( "JetEt90_HE_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET120_HE_hw  = new TH2F( "JetEt120_HE_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *jetET180_HE_hw  = new TH2F( "JetEt180_HE_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
   
  TH2F *l1jetET1_hw = new TH2F( "singleJet_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *l1jetET2_hw = new TH2F( "doubleJet_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *l1jetET3_hw = new TH2F( "tripleJet_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);
  TH2F *l1jetET4_hw = new TH2F( "quadJet_hw" , axD.c_str(),nJetBins, jetLo, jetHi, nPVbins, pvLow, pvHi);

  TH2F *l1ET_hw = new TH2F("etSum_hw","L1 sumET (GeV)",nEtSumBins,etSumLo,etSumHi, nPVbins, pvLow, pvHi);
  TH2F *l1MET_hw = new TH2F("metSum_hw","L1 MET (GeV)",nMetSumBins,metSumLo,metSumHi, nPVbins, pvLow, pvHi);
  TH2F *l1METHF_hw = new TH2F("metHFSum_hw","L1 METHF (GeV)",nMetSumBins,metSumLo,metSumHi, nPVbins, pvLow, pvHi);
  TH2F *l1HT_hw = new TH2F("htSum_hw","L1 HT (GeV)",nHtSumBins,htSumLo,htSumHi, nPVbins, pvLow, pvHi);
  TH2F *l1MHT_hw = new TH2F("mhtSum_hw","L1 MHT (GeV)",nMhtSumBins,mhtSumLo,mhtSumHi, nPVbins, pvLow, pvHi);

  // and Sums
  TH2F *refMET_Calo_hw  = new TH2F("RefMET_Calo_hw",metD.c_str(), nMetSumBins, metSumLo, metSumHi, nPVbins, pvLow, pvHi);
  TH2F *MET_150U_Calo_hw = new TH2F("MET150_Calo_hw",metD.c_str(), nMetSumBins, metSumLo, metSumHi, nPVbins, pvLow, pvHi);
  TH2F *MET_120U_Calo_hw  = new TH2F("MET120_Calo_hw",metD.c_str(), nMetSumBins, metSumLo, metSumHi, nPVbins, pvLow, pvHi);
  TH2F *MET_100U_Calo_hw  = new TH2F("MET100_Calo_hw",metD.c_str(), nMetSumBins, metSumLo, metSumHi, nPVbins, pvLow, pvHi);
  TH2F *MET_50U_Calo_hw  = new TH2F("MET50_Calo_hw",metD.c_str(), nMetSumBins, metSumLo, metSumHi, nPVbins, pvLow, pvHi);

  TH2F *refMET_PF_hw  = new TH2F("RefMET_PF_hw",metD.c_str(), nMetSumBins, metSumLo, metSumHi, nPVbins, pvLow, pvHi);
  TH2F *MET_150U_PF_hw = new TH2F("MET150_PF_hw",metD.c_str(), nMetSumBins, metSumLo, metSumHi, nPVbins, pvLow, pvHi);
  TH2F *MET_120U_PF_hw  = new TH2F("MET120_PF_hw",metD.c_str(), nMetSumBins, metSumLo, metSumHi, nPVbins, pvLow, pvHi);
  TH2F *MET_100U_PF_hw  = new TH2F("MET100_PF_hw",metD.c_str(), nMetSumBins, metSumLo, metSumHi, nPVbins, pvLow, pvHi);
  TH2F *MET_50U_PF_hw  = new TH2F("MET50_PF_hw",metD.c_str(), nMetSumBins, metSumLo, metSumHi, nPVbins, pvLow, pvHi);


  // resolution histograms
  TH3F *hresMET_PF_hw = new TH3F("hResMET_PF_hw","",nMetSumBins,metSumLo,metSumHi,100,-5,5,nPVbins,pvLow,pvHi);

  TH2F *h_resMET1_PF_hw = new TH2F("hresMET1_PF_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET2_PF_hw = new TH2F("hresMET2_PF_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET3_PF_hw = new TH2F("hresMET3_PF_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET4_PF_hw = new TH2F("hresMET4_PF_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET5_PF_hw = new TH2F("hresMET5_PF_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET6_PF_hw = new TH2F("hresMET6_PF_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET7_PF_hw = new TH2F("hresMET7_PF_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET8_PF_hw = new TH2F("hresMET8_PF_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET9_PF_hw = new TH2F("hresMET9_PF_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET10_PF_hw = new TH2F("hresMET10_PF_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
    
  TH3F *hresMET_Calo_hw = new TH3F("hResMET_Calo_hw","",nMetSumBins,metSumLo,metSumHi,100,-5,5,nPVbins,pvLow,pvHi);

  TH2F *h_resMET1_Calo_hw = new TH2F("hresMET1_Calo_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET2_Calo_hw = new TH2F("hresMET2_Calo_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET3_Calo_hw = new TH2F("hresMET3_Calo_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET4_Calo_hw = new TH2F("hresMET4_Calo_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET5_Calo_hw = new TH2F("hresMET5_Calo_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET6_Calo_hw = new TH2F("hresMET6_Calo_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET7_Calo_hw = new TH2F("hresMET7_Calo_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET8_Calo_hw = new TH2F("hresMET8_Calo_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET9_Calo_hw = new TH2F("hresMET9_Calo_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resMET10_Calo_hw = new TH2F("hresMET10_Calo_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;

  TH3F *hresJet_HB_hw = new TH3F("hresJet_HB_hw","",nJetBins, jetLo, jetHi,100,-5,5,nPVbins,pvLow,pvHi);
  TH3F *hresJet_HE1_hw = new TH3F("hresJet_HE1_hw","",nJetBins, jetLo, jetHi,100,-5,5,nPVbins,pvLow,pvHi);
  TH3F *hresJet_HE2_hw = new TH3F("hresJet_HE2_hw","",nJetBins, jetLo, jetHi,100,-5,5,nPVbins,pvLow,pvHi);
  TH3F *hresJet_HE_hw = new TH3F("hresJet_HE_hw","",nJetBins, jetLo, jetHi,100,-5,5,nPVbins,pvLow,pvHi);
  TH3F *hresJet_Incl_hw = new TH3F("hresJet_Incl_hw","",nJetBins, jetLo, jetHi,100,-5,5,nPVbins,pvLow,pvHi);

  TH2F *h_resJet1_hw = new TH2F("hresJet1_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resJet2_hw = new TH2F("hresJet2_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resJet3_hw = new TH2F("hresJet3_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resJet4_hw = new TH2F("hresJet4_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resJet5_hw = new TH2F("hresJet5_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resJet6_hw = new TH2F("hresJet6_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resJet7_hw = new TH2F("hresJet7_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resJet8_hw = new TH2F("hresJet8_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resJet9_hw = new TH2F("hresJet9_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;
  TH2F *h_resJet10_hw = new TH2F("hresJet10_hw","",100,-5,5, nPVbins, pvLow, pvHi) ;

  TH2F* hcalTP_hw = new TH2F("hcalTP_hw", ";TP E_{T}; # Entries", nTpBins, tpLo, tpHi, nPVbins, pvLow, pvHi);
  TH2F* ecalTP_hw = new TH2F("ecalTP_hw", ";TP E_{T}; # Entries", nTpBins, tpLo, tpHi, nPVbins, pvLow, pvHi);

  
  /////////////////////////////////
  // loop through all the entries//
  /////////////////////////////////
  for (Long64_t jentry=0; jentry<nentries; jentry++){
    if((jentry%10000)==0) std::cout << "Done " << jentry  << " events of " << nentries
				    << " \t (goodLumiEventCount = " << goodLumiEventCount << ")"
				    << std::endl;

    //lumi break clause
    eventTree->GetEntry(jentry);
    //skip the corresponding event
    //if (!isGoodLumiSection(event_->lumi)) continue;
    if (!isGoodLumiSection(event_->run, event_->lumi, toCheckWithJosnFile)) continue;
    goodLumiEventCount++;
	 
    //int nPV = event_->nPV_True;
    //int nPV = event_->nPV;
    int nPV; 
    if (isMC) nPV = event_->nPV_True;
    else      nPV = event_->nPV;	
    // nPV for data is not filled in l1t-ntuples
    if (nPV >99999) nPV = 0;
	
    // Check for at least one iso muon in the event.
    bool isoMu = false;
    if (recoOn && useRecoMuon_hlt_isomu_Cut) {
      muonTree->GetEntry(jentry);
      for(unsigned int i = 0; i < muon_->nMuons; ++i)
	{
	  if (muon_->hlt_isomu[i] == 1) {
	    isoMu = true;
	    break;
	  }
	}
      if (!isoMu && muonTree->GetEntry(jentry)) continue;	    
    }

    nEventsSelectedLevel1++;
    //do routine for L1 emulator quantites
    if (emuOn){
      nEventsSelectedLevel2++;
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
      // ALL EMU OBJECTS HAVE BX=0...
      double jetEt_1(0.);
      if (l1emu_->nJets>0) {
	jetEt_1 = l1emu_->jetEt[0];
	l1jetET1_emu->Fill(jetEt_1, nPV);
      }
      double jetEt_2(0.);
      if (l1emu_->nJets>1) {
	jetEt_2 = l1emu_->jetEt[1];
	l1jetET2_emu->Fill(jetEt_2, nPV);
      }
      double jetEt_3(0.);
      if (l1emu_->nJets>2) {
	jetEt_3 = l1emu_->jetEt[2];
	l1jetET3_emu->Fill(jetEt_3, nPV);
      }
      double jetEt_4(0.);
      if (l1emu_->nJets>3) {
	jetEt_4 = l1emu_->jetEt[3];
	l1jetET4_emu->Fill(jetEt_4, nPV);
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

      l1ET_emu->Fill(etSum, nPV);
      l1MET_emu->Fill(metSum, nPV);
      l1METHF_emu->Fill(metHFSum, nPV);
      l1HT_emu->Fill(htSum, nPV);
      l1MHT_emu->Fill(mhtSum, nPV);
      nEventsSelectedLevel3++;
	    
      // stuff for efficiencies and resolution
      if (recoOn) {
	nEventsSelectedLevel4++;
	recoTree->GetEntry(jentry);
	metfilterTree->GetEntry(jentry);

	// apply recommended MET filters
	//if (!metfilter_->muonBadTrackFilter) {
	//    std::cout << "GETTING OWNED BY BAD TRACK FILTER" << std::endl;
	//    continue;
	//}
	//if (!metfilter_->badPFMuonFilter) {
	//     std::cout << "GETTING OWNED BY BAD PF FILTER" << std::endl;
	//    continue;
	//}
	//if (!metfilter_->badChCandFilter) { 
	//    std::cout << "GETTING OWNED BY BAD CHC FILTER" << std::endl;
	//    continue;
	//}

	// met
	float pfMET = met_->pfMetNoMu;

	refMET_PF_emu->Fill( pfMET, nPV );
	if( metSum > metThresholds[0]) { MET_50U_PF_emu->Fill(pfMET, nPV);}
	if( metSum > metThresholds[1]) { MET_100U_PF_emu->Fill(pfMET, nPV);}
	if( metSum > metThresholds[2]) { MET_120U_PF_emu->Fill(pfMET, nPV);}
	if( metSum > metThresholds[3]) { MET_150U_PF_emu->Fill(pfMET, nPV);}

	// met resolution
	float resMET_PF = (metSum-pfMET)/pfMET;
	hresMET_PF_emu->Fill(pfMET, resMET_PF, nPV);
                
	if (pfMET<20.)               h_resMET1_PF_emu->Fill(resMET_PF, nPV);
	if (pfMET>=20. && pfMET<40.) h_resMET2_PF_emu->Fill(resMET_PF, nPV);
	if (pfMET>=40. && pfMET<60.) h_resMET3_PF_emu->Fill(resMET_PF, nPV);
	if (pfMET>=60. && pfMET<80.) h_resMET4_PF_emu->Fill(resMET_PF, nPV);
	if (pfMET>=80. && pfMET<100.) h_resMET5_PF_emu->Fill(resMET_PF, nPV);
	if (pfMET>=100. && pfMET<120.) h_resMET6_PF_emu->Fill(resMET_PF, nPV);
	if (pfMET>=120. && pfMET<140.) h_resMET7_PF_emu->Fill(resMET_PF, nPV);
	if (pfMET>=140. && pfMET<180.) h_resMET8_PF_emu->Fill(resMET_PF, nPV);
	if (pfMET>=180. && pfMET<250.) h_resMET9_PF_emu->Fill(resMET_PF, nPV);
	if (pfMET>=250. && pfMET<500.) h_resMET10_PF_emu->Fill(resMET_PF, nPV);
                


	float caloMET = met_->caloMet;
	refMET_Calo_emu->Fill( caloMET, nPV );
	if( metSum > metThresholds[0]) { MET_50U_Calo_emu->Fill(caloMET, nPV);}
	if( metSum > metThresholds[1]) { MET_100U_Calo_emu->Fill(caloMET, nPV);}
	if( metSum > metThresholds[2]) { MET_120U_Calo_emu->Fill(caloMET, nPV);}
	if( metSum > metThresholds[3]) { MET_150U_Calo_emu->Fill(caloMET, nPV);}

	// met resolution
	float resMET_calo = (metSum-caloMET)/caloMET;
	hresMET_Calo_emu->Fill(caloMET, resMET_calo, nPV);
                
	if (caloMET<20.)                 h_resMET1_Calo_emu->Fill(resMET_calo, nPV);
	if (caloMET>=20. && caloMET<40.) h_resMET2_Calo_emu->Fill(resMET_calo, nPV);
	if (caloMET>=40. && caloMET<60.) h_resMET3_Calo_emu->Fill(resMET_calo, nPV);
	if (caloMET>=60. && caloMET<80.) h_resMET4_Calo_emu->Fill(resMET_calo, nPV);
	if (caloMET>=80. && caloMET<100.) h_resMET5_Calo_emu->Fill(resMET_calo, nPV);
	if (caloMET>=100. && caloMET<120.) h_resMET6_Calo_emu->Fill(resMET_calo, nPV);
	if (caloMET>=120. && caloMET<140.) h_resMET7_Calo_emu->Fill(resMET_calo, nPV);
	if (caloMET>=140. && caloMET<180.) h_resMET8_Calo_emu->Fill(resMET_calo, nPV);
	if (caloMET>=180. && caloMET<250.) h_resMET9_Calo_emu->Fill(resMET_calo, nPV);
	if (caloMET>=250. && caloMET<500.) h_resMET10_Calo_emu->Fill(resMET_calo, nPV);

	// leading offline jet
	double maxEn(0.);
	int jetIdx(-1);

	for(unsigned int i = 0; i < jet_->nJets; ++i)
	  {
	    if(jet_->etCorr[i] > maxEn){
	      maxEn = jet_->etCorr[i];
	      if (maxEn>10.) jetIdx = i;
	    }
	  }

	if (jetIdx>=0) { // at least 1 offline jet >10. geV
	  nEventsSelectedLevel5++;
		  
	  double jetEta = abs(jet_->eta[jetIdx]);

	  refJetET_Incl_emu->Fill(jet_->etCorr[jetIdx], nPV);
	  if      (jetEta >= 0 && jetEta < 1.392)     { refJetET_HB_emu->Fill(jet_->etCorr[jetIdx], nPV); }
	  else if (jetEta >= 1.392 && jetEta < 1.74) { refJetET_HE1_emu->Fill(jet_->etCorr[jetIdx], nPV); refJetET_HE_emu->Fill(jet_->etCorr[jetIdx], nPV);} 
	  else if (jetEta >= 1.74 && jetEta < 3.0) { refJetET_HE2_emu->Fill(jet_->etCorr[jetIdx], nPV); refJetET_HE_emu->Fill(jet_->etCorr[jetIdx], nPV);}
                    
	  // return Matched L1 jet
	  int l1jetIdx(-1);
	  double minDR = 999.;
	  double dptmin=1000.;
	  for (unsigned int i=0; i<l1emu_->nJets; i++) {
	    double dR=deltaR(jet_->eta[jetIdx], jet_->phi[jetIdx],l1emu_->jetEta[i],l1emu_->jetPhi[i]);
	    double dpt=fabs( (l1emu_->jetEt[i]-jet_->etCorr[jetIdx])/jet_->etCorr[jetIdx] );
	    if (dR<minDR && dpt<dptmin) {
	      minDR=dR;
	      dptmin=dpt;
	      if (minDR<0.5) l1jetIdx=i;
	    }
	  }
                    
	  if (l1jetIdx>=0) { // found matched l1jet
	    nEventsSelectedLevel6++;
		      
	    float resJet=(l1emu_->jetEt[l1jetIdx]-jet_->etCorr[jetIdx])/jet_->etCorr[jetIdx];
	    refmJetET_Incl_emu->Fill(jet_->etCorr[jetIdx], nPV);
	    hresJet_Incl_emu->Fill(jet_->etCorr[jetIdx],resJet, nPV);

	    if (l1emu_->jetEt[l1jetIdx]>jetThresholds[0]) jetET12_emu->Fill(jet_->etCorr[jetIdx], nPV);
	    if (l1emu_->jetEt[l1jetIdx]>jetThresholds[1]) jetET35_emu->Fill(jet_->etCorr[jetIdx], nPV);
	    if (l1emu_->jetEt[l1jetIdx]>jetThresholds[2]) jetET60_emu->Fill(jet_->etCorr[jetIdx], nPV);
	    if (l1emu_->jetEt[l1jetIdx]>jetThresholds[3]) jetET90_emu->Fill(jet_->etCorr[jetIdx], nPV);
	    if (l1emu_->jetEt[l1jetIdx]>jetThresholds[4]) jetET120_emu->Fill(jet_->etCorr[jetIdx], nPV);
	    if (l1emu_->jetEt[l1jetIdx]>jetThresholds[5]) jetET180_emu->Fill(jet_->etCorr[jetIdx], nPV);

	    if (jetEta >= 0 && jetEta < 1.392) {
	      refmJetET_HB_emu->Fill(jet_->etCorr[jetIdx], nPV);
	      hresJet_HB_emu->Fill(jet_->etCorr[jetIdx],resJet,nPV);

	      if (l1emu_->jetEt[l1jetIdx]>jetThresholds[0]) jetET12_HB_emu->Fill(jet_->etCorr[jetIdx], nPV);
	      if (l1emu_->jetEt[l1jetIdx]>jetThresholds[1]) jetET35_HB_emu->Fill(jet_->etCorr[jetIdx], nPV);
	      if (l1emu_->jetEt[l1jetIdx]>jetThresholds[2]) jetET60_HB_emu->Fill(jet_->etCorr[jetIdx], nPV);
	      if (l1emu_->jetEt[l1jetIdx]>jetThresholds[3]) jetET90_HB_emu->Fill(jet_->etCorr[jetIdx], nPV);
	      if (l1emu_->jetEt[l1jetIdx]>jetThresholds[4]) jetET120_HB_emu->Fill(jet_->etCorr[jetIdx], nPV);
	      if (l1emu_->jetEt[l1jetIdx]>jetThresholds[5]) jetET180_HB_emu->Fill(jet_->etCorr[jetIdx], nPV);
	    } else if (jetEta >= 1.392 && jetEta < 1.74) {
	      refmJetET_HE1_emu->Fill(jet_->etCorr[jetIdx],nPV);
	      hresJet_HE1_emu->Fill(jet_->etCorr[jetIdx],resJet,nPV);

	      refmJetET_HE_emu->Fill(jet_->etCorr[jetIdx],nPV);
	      hresJet_HE_emu->Fill(jet_->etCorr[jetIdx],resJet,nPV);

	      if (l1emu_->jetEt[l1jetIdx]>jetThresholds[0]) {jetET12_HE1_emu->Fill(jet_->etCorr[jetIdx], nPV);  jetET12_HE_emu->Fill(jet_->etCorr[jetIdx], nPV);}
	      if (l1emu_->jetEt[l1jetIdx]>jetThresholds[1]) {jetET35_HE1_emu->Fill(jet_->etCorr[jetIdx], nPV);  jetET35_HE_emu->Fill(jet_->etCorr[jetIdx], nPV);}
	      if (l1emu_->jetEt[l1jetIdx]>jetThresholds[2]) {jetET60_HE1_emu->Fill(jet_->etCorr[jetIdx], nPV);  jetET60_HE_emu->Fill(jet_->etCorr[jetIdx], nPV);}
	      if (l1emu_->jetEt[l1jetIdx]>jetThresholds[3]) {jetET90_HE1_emu->Fill(jet_->etCorr[jetIdx], nPV);  jetET90_HE_emu->Fill(jet_->etCorr[jetIdx], nPV);}
	      if (l1emu_->jetEt[l1jetIdx]>jetThresholds[4]) {jetET120_HE1_emu->Fill(jet_->etCorr[jetIdx], nPV); jetET120_HE_emu->Fill(jet_->etCorr[jetIdx], nPV);}
	      if (l1emu_->jetEt[l1jetIdx]>jetThresholds[5]) {jetET180_HE1_emu->Fill(jet_->etCorr[jetIdx], nPV); jetET180_HE_emu->Fill(jet_->etCorr[jetIdx], nPV);}

	    } else if (jetEta >= 1.74 && jetEta < 3.0) {
	      refmJetET_HE2_emu->Fill(jet_->etCorr[jetIdx], nPV);
	      hresJet_HE2_emu->Fill(jet_->etCorr[jetIdx],resJet, nPV);

	      refmJetET_HE_emu->Fill(jet_->etCorr[jetIdx], nPV);
	      hresJet_HE_emu->Fill(jet_->etCorr[jetIdx],resJet, nPV);

	      if (l1emu_->jetEt[l1jetIdx]>jetThresholds[0]) {jetET12_HE2_emu->Fill(jet_->etCorr[jetIdx], nPV);  jetET12_HE_emu->Fill(jet_->etCorr[jetIdx], nPV);}
	      if (l1emu_->jetEt[l1jetIdx]>jetThresholds[1]) {jetET35_HE2_emu->Fill(jet_->etCorr[jetIdx], nPV);  jetET35_HE_emu->Fill(jet_->etCorr[jetIdx], nPV);}
	      if (l1emu_->jetEt[l1jetIdx]>jetThresholds[2]) {jetET60_HE2_emu->Fill(jet_->etCorr[jetIdx], nPV);  jetET60_HE_emu->Fill(jet_->etCorr[jetIdx], nPV);}
	      if (l1emu_->jetEt[l1jetIdx]>jetThresholds[3]) {jetET90_HE2_emu->Fill(jet_->etCorr[jetIdx], nPV);  jetET90_HE_emu->Fill(jet_->etCorr[jetIdx], nPV);}
	      if (l1emu_->jetEt[l1jetIdx]>jetThresholds[4]) {jetET120_HE2_emu->Fill(jet_->etCorr[jetIdx], nPV); jetET120_HE_emu->Fill(jet_->etCorr[jetIdx], nPV);}
	      if (l1emu_->jetEt[l1jetIdx]>jetThresholds[5]) {jetET180_HE2_emu->Fill(jet_->etCorr[jetIdx], nPV); jetET180_HE_emu->Fill(jet_->etCorr[jetIdx], nPV);}
	    }

	    if (jet_->etCorr[jetIdx]<50.) h_resJet1_emu->Fill(resJet, nPV);
	    if (jet_->etCorr[jetIdx]>=50. && jet_->etCorr[jetIdx]<100.) h_resJet2_emu->Fill(resJet, nPV);
	    if (jet_->etCorr[jetIdx]>=100. && jet_->etCorr[jetIdx]<150.) h_resJet3_emu->Fill(resJet, nPV);
	    if (jet_->etCorr[jetIdx]>=150. && jet_->etCorr[jetIdx]<200.) h_resJet4_emu->Fill(resJet, nPV);
	    if (jet_->etCorr[jetIdx]>=200. && jet_->etCorr[jetIdx]<250.) h_resJet5_emu->Fill(resJet, nPV);
	    if (jet_->etCorr[jetIdx]>=250. && jet_->etCorr[jetIdx]<300.) h_resJet6_emu->Fill(resJet, nPV);
	    if (jet_->etCorr[jetIdx]>=300. && jet_->etCorr[jetIdx]<350.) h_resJet7_emu->Fill(resJet, nPV);
	    if (jet_->etCorr[jetIdx]>=350. && jet_->etCorr[jetIdx]<400.) h_resJet8_emu->Fill(resJet, nPV);
	    if (jet_->etCorr[jetIdx]>=400. && jet_->etCorr[jetIdx]<450.) h_resJet9_emu->Fill(resJet, nPV);
	    if (jet_->etCorr[jetIdx]>=450. && jet_->etCorr[jetIdx]<500.) h_resJet10_emu->Fill(resJet, nPV);
                      
	  } // close 'found matched l1jet'
	} // close 'at least one offline jet'
        
      }// closes if 'recoOn' is true

    }// closes if 'emuOn' is true



    if (hwOn){
      nEventsSelectedLevel2++;
      treeL1TPhw->GetEntry(jentry);
      double tpEt(0.);
            
      for(int i=0; i < l1TPhw_->nHCALTP; i++){
	tpEt = l1TPhw_->hcalTPet[i];
	hcalTP_hw->Fill(tpEt, nPV);
      }
      for(int i=0; i < l1TPhw_->nECALTP; i++){
	tpEt = l1TPhw_->ecalTPet[i];
	ecalTP_hw->Fill(tpEt, nPV);
      }

      treeL1hw->GetEntry(jentry);
      // get jetEt*, egEt*, tauEt, htSum, mhtSum, etSum, metSum
      // ALL hw OBJECTS HAVE BX=0...
      double jetEt_1(0.);
      if (l1hw_->nJets>0) {
	jetEt_1 = l1hw_->jetEt[0];
	l1jetET1_hw->Fill(jetEt_1, nPV);
      }
      double jetEt_2(0.);
      if (l1hw_->nJets>1) {
	jetEt_2 = l1hw_->jetEt[1];
	l1jetET2_hw->Fill(jetEt_2, nPV);
      }
      double jetEt_3(0.);
      if (l1hw_->nJets>2) {
	jetEt_3 = l1hw_->jetEt[2];
	l1jetET3_hw->Fill(jetEt_3, nPV);
      }
      double jetEt_4(0.);
      if (l1hw_->nJets>3) {
	jetEt_4 = l1hw_->jetEt[3];
	l1jetET4_hw->Fill(jetEt_4, nPV);
      }
            
      double htSum(0.0);
      double mhtSum(0.0);
      double etSum(0.0);
      double metSum(0.0);
      double metHFSum(0.0);
      for (unsigned int c=0; c<l1hw_->nSums; c++){
	if( l1hw_->sumBx[c] != 0 ) continue;
	if( l1hw_->sumType[c] == L1Analysis::kTotalEt ) etSum = l1hw_->sumEt[c];
	if( l1hw_->sumType[c] == L1Analysis::kTotalHt ) htSum = l1hw_->sumEt[c];
	if( l1hw_->sumType[c] == L1Analysis::kMissingEt ) metSum = l1hw_->sumEt[c];
	if( l1hw_->sumType[c] == L1Analysis::kMissingEtHF ) metHFSum = l1hw_->sumEt[c];
	if( l1hw_->sumType[c] == L1Analysis::kMissingHt ) mhtSum = l1hw_->sumEt[c];
      }

      l1ET_hw->Fill(etSum, nPV);
      l1MET_hw->Fill(metSum, nPV);
      l1METHF_hw->Fill(metHFSum, nPV);
      l1HT_hw->Fill(htSum, nPV);
      l1MHT_hw->Fill(mhtSum, nPV);
      nEventsSelectedLevel3++;
	    
      // stuff for efficiencies and resolution
      if (recoOn) {
	nEventsSelectedLevel4++;
	recoTree->GetEntry(jentry);
	metfilterTree->GetEntry(jentry);

	// apply recommended MET filters
	//if (!metfilter_->muonBadTrackFilter) {
	//    std::cout << "GETTING OWNED BY BAD TRACK FILTER" << std::endl;
	//    continue;
	//}
	//if (!metfilter_->badPFMuonFilter) {
	//     std::cout << "GETTING OWNED BY BAD PF FILTER" << std::endl;
	//    continue;
	//}
	//if (!metfilter_->badChCandFilter) { 
	//    std::cout << "GETTING OWNED BY BAD CHC FILTER" << std::endl;
	//    continue;
	//}

	// met
	float pfMET = met_->pfMetNoMu;

	refMET_PF_hw->Fill( pfMET, nPV );
	if( metSum > metThresholds[0]) { MET_50U_PF_hw->Fill(pfMET, nPV);}
	if( metSum > metThresholds[1]) { MET_100U_PF_hw->Fill(pfMET, nPV);}
	if( metSum > metThresholds[2]) { MET_120U_PF_hw->Fill(pfMET, nPV);}
	if( metSum > metThresholds[3]) { MET_150U_PF_hw->Fill(pfMET, nPV);}

	// met resolution
	float resMET_PF = (metSum-pfMET)/pfMET;
	hresMET_PF_hw->Fill(pfMET, resMET_PF, nPV);
                
	if (pfMET<20.)               h_resMET1_PF_hw->Fill(resMET_PF, nPV);
	if (pfMET>=20. && pfMET<40.) h_resMET2_PF_hw->Fill(resMET_PF, nPV);
	if (pfMET>=40. && pfMET<60.) h_resMET3_PF_hw->Fill(resMET_PF, nPV);
	if (pfMET>=60. && pfMET<80.) h_resMET4_PF_hw->Fill(resMET_PF, nPV);
	if (pfMET>=80. && pfMET<100.) h_resMET5_PF_hw->Fill(resMET_PF, nPV);
	if (pfMET>=100. && pfMET<120.) h_resMET6_PF_hw->Fill(resMET_PF, nPV);
	if (pfMET>=120. && pfMET<140.) h_resMET7_PF_hw->Fill(resMET_PF, nPV);
	if (pfMET>=140. && pfMET<180.) h_resMET8_PF_hw->Fill(resMET_PF, nPV);
	if (pfMET>=180. && pfMET<250.) h_resMET9_PF_hw->Fill(resMET_PF, nPV);
	if (pfMET>=250. && pfMET<500.) h_resMET10_PF_hw->Fill(resMET_PF, nPV);
                


	float caloMET = met_->caloMet;
	refMET_Calo_hw->Fill( caloMET, nPV );
	if( metSum > metThresholds[0]) { MET_50U_Calo_hw->Fill(caloMET, nPV);}
	if( metSum > metThresholds[1]) { MET_100U_Calo_hw->Fill(caloMET, nPV);}
	if( metSum > metThresholds[2]) { MET_120U_Calo_hw->Fill(caloMET, nPV);}
	if( metSum > metThresholds[3]) { MET_150U_Calo_hw->Fill(caloMET, nPV);}

	// met resolution
	float resMET_calo = (metSum-caloMET)/caloMET;
	hresMET_Calo_hw->Fill(caloMET, resMET_calo, nPV);
                
	if (caloMET<20.)                 h_resMET1_Calo_hw->Fill(resMET_calo, nPV);
	if (caloMET>=20. && caloMET<40.) h_resMET2_Calo_hw->Fill(resMET_calo, nPV);
	if (caloMET>=40. && caloMET<60.) h_resMET3_Calo_hw->Fill(resMET_calo, nPV);
	if (caloMET>=60. && caloMET<80.) h_resMET4_Calo_hw->Fill(resMET_calo, nPV);
	if (caloMET>=80. && caloMET<100.) h_resMET5_Calo_hw->Fill(resMET_calo, nPV);
	if (caloMET>=100. && caloMET<120.) h_resMET6_Calo_hw->Fill(resMET_calo, nPV);
	if (caloMET>=120. && caloMET<140.) h_resMET7_Calo_hw->Fill(resMET_calo, nPV);
	if (caloMET>=140. && caloMET<180.) h_resMET8_Calo_hw->Fill(resMET_calo, nPV);
	if (caloMET>=180. && caloMET<250.) h_resMET9_Calo_hw->Fill(resMET_calo, nPV);
	if (caloMET>=250. && caloMET<500.) h_resMET10_Calo_hw->Fill(resMET_calo, nPV);

	// leading offline jet
	double maxEn(0.);
	int jetIdx(-1);

	for(unsigned int i = 0; i < jet_->nJets; ++i)
	  {
	    if(jet_->etCorr[i] > maxEn){
	      maxEn = jet_->etCorr[i];
	      if (maxEn>10.) jetIdx = i;
	    }
	  }

	if (jetIdx>=0) { // at least 1 offline jet >10. geV
	  nEventsSelectedLevel5++;
		  
	  double jetEta = abs(jet_->eta[jetIdx]);

	  refJetET_Incl_hw->Fill(jet_->etCorr[jetIdx], nPV);
	  if      (jetEta >= 0 && jetEta < 1.392)     { refJetET_HB_hw->Fill(jet_->etCorr[jetIdx], nPV); }
	  else if (jetEta >= 1.392 && jetEta < 1.74) { refJetET_HE1_hw->Fill(jet_->etCorr[jetIdx], nPV); refJetET_HE_hw->Fill(jet_->etCorr[jetIdx], nPV);} 
	  else if (jetEta >= 1.74 && jetEta < 3.0) { refJetET_HE2_hw->Fill(jet_->etCorr[jetIdx], nPV); refJetET_HE_hw->Fill(jet_->etCorr[jetIdx], nPV);}
                    
	  // return Matched L1 jet
	  int l1jetIdx(-1);
	  double minDR = 999.;
	  double dptmin=1000.;
	  for (unsigned int i=0; i<l1hw_->nJets; i++) {
	    double dR=deltaR(jet_->eta[jetIdx], jet_->phi[jetIdx],l1hw_->jetEta[i],l1hw_->jetPhi[i]);
	    double dpt=fabs( (l1hw_->jetEt[i]-jet_->etCorr[jetIdx])/jet_->etCorr[jetIdx] );
	    if (dR<minDR && dpt<dptmin) {
	      minDR=dR;
	      dptmin=dpt;
	      if (minDR<0.5) l1jetIdx=i;
	    }
	  }
                    
	  if (l1jetIdx>=0) { // found matched l1jet
	    nEventsSelectedLevel6++;
		      
	    float resJet=(l1hw_->jetEt[l1jetIdx]-jet_->etCorr[jetIdx])/jet_->etCorr[jetIdx];
	    refmJetET_Incl_hw->Fill(jet_->etCorr[jetIdx], nPV);
	    hresJet_Incl_hw->Fill(jet_->etCorr[jetIdx],resJet, nPV);

	    if (l1hw_->jetEt[l1jetIdx]>jetThresholds[0]) jetET12_hw->Fill(jet_->etCorr[jetIdx], nPV);
	    if (l1hw_->jetEt[l1jetIdx]>jetThresholds[1]) jetET35_hw->Fill(jet_->etCorr[jetIdx], nPV);
	    if (l1hw_->jetEt[l1jetIdx]>jetThresholds[2]) jetET60_hw->Fill(jet_->etCorr[jetIdx], nPV);
	    if (l1hw_->jetEt[l1jetIdx]>jetThresholds[3]) jetET90_hw->Fill(jet_->etCorr[jetIdx], nPV);
	    if (l1hw_->jetEt[l1jetIdx]>jetThresholds[4]) jetET120_hw->Fill(jet_->etCorr[jetIdx], nPV);
	    if (l1hw_->jetEt[l1jetIdx]>jetThresholds[5]) jetET180_hw->Fill(jet_->etCorr[jetIdx], nPV);

	    if (jetEta >= 0 && jetEta < 1.392) {
	      refmJetET_HB_hw->Fill(jet_->etCorr[jetIdx], nPV);
	      hresJet_HB_hw->Fill(jet_->etCorr[jetIdx],resJet,nPV);

	      if (l1hw_->jetEt[l1jetIdx]>jetThresholds[0]) jetET12_HB_hw->Fill(jet_->etCorr[jetIdx], nPV);
	      if (l1hw_->jetEt[l1jetIdx]>jetThresholds[1]) jetET35_HB_hw->Fill(jet_->etCorr[jetIdx], nPV);
	      if (l1hw_->jetEt[l1jetIdx]>jetThresholds[2]) jetET60_HB_hw->Fill(jet_->etCorr[jetIdx], nPV);
	      if (l1hw_->jetEt[l1jetIdx]>jetThresholds[3]) jetET90_HB_hw->Fill(jet_->etCorr[jetIdx], nPV);
	      if (l1hw_->jetEt[l1jetIdx]>jetThresholds[4]) jetET120_HB_hw->Fill(jet_->etCorr[jetIdx], nPV);
	      if (l1hw_->jetEt[l1jetIdx]>jetThresholds[5]) jetET180_HB_hw->Fill(jet_->etCorr[jetIdx], nPV);
	    } else if (jetEta >= 1.392 && jetEta < 1.74) {
	      refmJetET_HE1_hw->Fill(jet_->etCorr[jetIdx],nPV);
	      hresJet_HE1_hw->Fill(jet_->etCorr[jetIdx],resJet,nPV);

	      refmJetET_HE_hw->Fill(jet_->etCorr[jetIdx],nPV);
	      hresJet_HE_hw->Fill(jet_->etCorr[jetIdx],resJet,nPV);

	      if (l1hw_->jetEt[l1jetIdx]>jetThresholds[0]) {jetET12_HE1_hw->Fill(jet_->etCorr[jetIdx], nPV);  jetET12_HE_hw->Fill(jet_->etCorr[jetIdx], nPV);}
	      if (l1hw_->jetEt[l1jetIdx]>jetThresholds[1]) {jetET35_HE1_hw->Fill(jet_->etCorr[jetIdx], nPV);  jetET35_HE_hw->Fill(jet_->etCorr[jetIdx], nPV);}
	      if (l1hw_->jetEt[l1jetIdx]>jetThresholds[2]) {jetET60_HE1_hw->Fill(jet_->etCorr[jetIdx], nPV);  jetET60_HE_hw->Fill(jet_->etCorr[jetIdx], nPV);}
	      if (l1hw_->jetEt[l1jetIdx]>jetThresholds[3]) {jetET90_HE1_hw->Fill(jet_->etCorr[jetIdx], nPV);  jetET90_HE_hw->Fill(jet_->etCorr[jetIdx], nPV);}
	      if (l1hw_->jetEt[l1jetIdx]>jetThresholds[4]) {jetET120_HE1_hw->Fill(jet_->etCorr[jetIdx], nPV); jetET120_HE_hw->Fill(jet_->etCorr[jetIdx], nPV);}
	      if (l1hw_->jetEt[l1jetIdx]>jetThresholds[5]) {jetET180_HE1_hw->Fill(jet_->etCorr[jetIdx], nPV); jetET180_HE_hw->Fill(jet_->etCorr[jetIdx], nPV);}

	    } else if (jetEta >= 1.74 && jetEta < 3.0) {
	      refmJetET_HE2_hw->Fill(jet_->etCorr[jetIdx], nPV);
	      hresJet_HE2_hw->Fill(jet_->etCorr[jetIdx],resJet, nPV);

	      refmJetET_HE_hw->Fill(jet_->etCorr[jetIdx], nPV);
	      hresJet_HE_hw->Fill(jet_->etCorr[jetIdx],resJet, nPV);

	      if (l1hw_->jetEt[l1jetIdx]>jetThresholds[0]) {jetET12_HE2_hw->Fill(jet_->etCorr[jetIdx], nPV);  jetET12_HE_hw->Fill(jet_->etCorr[jetIdx], nPV);}
	      if (l1hw_->jetEt[l1jetIdx]>jetThresholds[1]) {jetET35_HE2_hw->Fill(jet_->etCorr[jetIdx], nPV);  jetET35_HE_hw->Fill(jet_->etCorr[jetIdx], nPV);}
	      if (l1hw_->jetEt[l1jetIdx]>jetThresholds[2]) {jetET60_HE2_hw->Fill(jet_->etCorr[jetIdx], nPV);  jetET60_HE_hw->Fill(jet_->etCorr[jetIdx], nPV);}
	      if (l1hw_->jetEt[l1jetIdx]>jetThresholds[3]) {jetET90_HE2_hw->Fill(jet_->etCorr[jetIdx], nPV);  jetET90_HE_hw->Fill(jet_->etCorr[jetIdx], nPV);}
	      if (l1hw_->jetEt[l1jetIdx]>jetThresholds[4]) {jetET120_HE2_hw->Fill(jet_->etCorr[jetIdx], nPV); jetET120_HE_hw->Fill(jet_->etCorr[jetIdx], nPV);}
	      if (l1hw_->jetEt[l1jetIdx]>jetThresholds[5]) {jetET180_HE2_hw->Fill(jet_->etCorr[jetIdx], nPV); jetET180_HE_hw->Fill(jet_->etCorr[jetIdx], nPV);}
	    }

	    if (jet_->etCorr[jetIdx]<50.) h_resJet1_hw->Fill(resJet, nPV);
	    if (jet_->etCorr[jetIdx]>=50. && jet_->etCorr[jetIdx]<100.) h_resJet2_hw->Fill(resJet, nPV);
	    if (jet_->etCorr[jetIdx]>=100. && jet_->etCorr[jetIdx]<150.) h_resJet3_hw->Fill(resJet, nPV);
	    if (jet_->etCorr[jetIdx]>=150. && jet_->etCorr[jetIdx]<200.) h_resJet4_hw->Fill(resJet, nPV);
	    if (jet_->etCorr[jetIdx]>=200. && jet_->etCorr[jetIdx]<250.) h_resJet5_hw->Fill(resJet, nPV);
	    if (jet_->etCorr[jetIdx]>=250. && jet_->etCorr[jetIdx]<300.) h_resJet6_hw->Fill(resJet, nPV);
	    if (jet_->etCorr[jetIdx]>=300. && jet_->etCorr[jetIdx]<350.) h_resJet7_hw->Fill(resJet, nPV);
	    if (jet_->etCorr[jetIdx]>=350. && jet_->etCorr[jetIdx]<400.) h_resJet8_hw->Fill(resJet, nPV);
	    if (jet_->etCorr[jetIdx]>=400. && jet_->etCorr[jetIdx]<450.) h_resJet9_hw->Fill(resJet, nPV);
	    if (jet_->etCorr[jetIdx]>=450. && jet_->etCorr[jetIdx]<500.) h_resJet10_hw->Fill(resJet, nPV);
                      
	  } // close 'found matched l1jet'
	} // close 'at least one offline jet'
        
      }// closes if 'recoOn' is true

    }// closes if 'hwOn' is true

    
  }// closes loop through events

  //  TFile g( outputFilename.c_str() , "new");
  kk->cd();

  if (emuOn){
    // ecal/hcal TPs
    hcalTP_emu->Write();
    ecalTP_emu->Write();
    // l1 quantities
    l1jetET1_emu->Write(); l1jetET2_emu->Write(); l1jetET3_emu->Write(); l1jetET4_emu->Write();
    l1ET_emu->Write(); l1MET_emu->Write(); l1METHF_emu->Write(); l1HT_emu->Write(); l1MHT_emu->Write();
    // efficiencies
    refJetET_Incl_emu->Write(); refmJetET_Incl_emu->Write();
    refJetET_HB_emu->Write(); refmJetET_HB_emu->Write();
    refJetET_HE1_emu->Write(); refmJetET_HE1_emu->Write();
    refJetET_HE2_emu->Write(); refmJetET_HE2_emu->Write();
    refJetET_HE_emu->Write(); refmJetET_HE_emu->Write();
    jetET12_emu->Write(); jetET35_emu->Write(); jetET60_emu->Write(); jetET90_emu->Write(); jetET120_emu->Write(); jetET180_emu->Write();
    jetET12_HB_emu->Write(); jetET35_HB_emu->Write(); jetET60_HB_emu->Write(); jetET90_HB_emu->Write(); jetET120_HB_emu->Write(); jetET180_HB_emu->Write();
    jetET12_HE1_emu->Write(); jetET35_HE1_emu->Write(); jetET60_HE1_emu->Write(); jetET90_HE1_emu->Write(); jetET120_HE1_emu->Write(); jetET180_HE1_emu->Write();
    jetET12_HE2_emu->Write(); jetET35_HE2_emu->Write(); jetET60_HE2_emu->Write(); jetET90_HE2_emu->Write(); jetET120_HE2_emu->Write(); jetET180_HE2_emu->Write();
    jetET12_HE_emu->Write(); jetET35_HE_emu->Write(); jetET60_HE_emu->Write(); jetET90_HE_emu->Write(); jetET120_HE_emu->Write(); jetET180_HE_emu->Write();

    refMET_PF_emu->Write(); refMET_Calo_emu->Write();
    MET_50U_PF_emu->Write(); MET_100U_PF_emu->Write(); MET_120U_PF_emu->Write(); MET_150U_PF_emu->Write();
    MET_50U_Calo_emu->Write(); MET_100U_Calo_emu->Write(); MET_120U_Calo_emu->Write(); MET_150U_Calo_emu->Write();
    // resolutions
    hresMET_PF_emu->Write(); hresMET_Calo_emu->Write(); hresJet_Incl_emu->Write();
    hresJet_HB_emu->Write();
    hresJet_HE1_emu->Write();
    hresJet_HE2_emu->Write();
    hresJet_HE_emu->Write();
    h_resMET1_PF_emu->Write();h_resMET2_PF_emu->Write();h_resMET3_PF_emu->Write();h_resMET4_PF_emu->Write();h_resMET5_PF_emu->Write();h_resMET6_PF_emu->Write();h_resMET7_PF_emu->Write();h_resMET8_PF_emu->Write();h_resMET9_PF_emu->Write();h_resMET10_PF_emu->Write();
    h_resMET1_Calo_emu->Write();h_resMET2_Calo_emu->Write();h_resMET3_Calo_emu->Write();h_resMET4_Calo_emu->Write();h_resMET5_Calo_emu->Write();h_resMET6_Calo_emu->Write();h_resMET7_Calo_emu->Write();h_resMET8_Calo_emu->Write();h_resMET9_Calo_emu->Write();h_resMET10_Calo_emu->Write();
    h_resJet1_emu->Write();h_resJet2_emu->Write();h_resJet3_emu->Write();h_resJet4_emu->Write();h_resJet5_emu->Write();h_resJet6_emu->Write();h_resJet7_emu->Write();h_resJet8_emu->Write();h_resJet9_emu->Write();h_resJet10_emu->Write();
  }


  if (hwOn){
    // ecal/hcal TPs
    hcalTP_hw->Write();
    ecalTP_hw->Write();
    // l1 quantities
    l1jetET1_hw->Write(); l1jetET2_hw->Write(); l1jetET3_hw->Write(); l1jetET4_hw->Write();
    l1ET_hw->Write(); l1MET_hw->Write(); l1METHF_hw->Write(); l1HT_hw->Write(); l1MHT_hw->Write();
    // efficiencies
    refJetET_Incl_hw->Write(); refmJetET_Incl_hw->Write();
    refJetET_HB_hw->Write(); refmJetET_HB_hw->Write();
    refJetET_HE1_hw->Write(); refmJetET_HE1_hw->Write();
    refJetET_HE2_hw->Write(); refmJetET_HE2_hw->Write();
    refJetET_HE_hw->Write(); refmJetET_HE_hw->Write();
    jetET12_hw->Write(); jetET35_hw->Write(); jetET60_hw->Write(); jetET90_hw->Write(); jetET120_hw->Write(); jetET180_hw->Write();
    jetET12_HB_hw->Write(); jetET35_HB_hw->Write(); jetET60_HB_hw->Write(); jetET90_HB_hw->Write(); jetET120_HB_hw->Write(); jetET180_HB_hw->Write();
    jetET12_HE1_hw->Write(); jetET35_HE1_hw->Write(); jetET60_HE1_hw->Write(); jetET90_HE1_hw->Write(); jetET120_HE1_hw->Write(); jetET180_HE1_hw->Write();
    jetET12_HE2_hw->Write(); jetET35_HE2_hw->Write(); jetET60_HE2_hw->Write(); jetET90_HE2_hw->Write(); jetET120_HE2_hw->Write(); jetET180_HE2_hw->Write();
    jetET12_HE_hw->Write(); jetET35_HE_hw->Write(); jetET60_HE_hw->Write(); jetET90_HE_hw->Write(); jetET120_HE_hw->Write(); jetET180_HE_hw->Write();

    refMET_PF_hw->Write(); refMET_Calo_hw->Write();
    MET_50U_PF_hw->Write(); MET_100U_PF_hw->Write(); MET_120U_PF_hw->Write(); MET_150U_PF_hw->Write();
    MET_50U_Calo_hw->Write(); MET_100U_Calo_hw->Write(); MET_120U_Calo_hw->Write(); MET_150U_Calo_hw->Write();
    // resolutions
    hresMET_PF_hw->Write(); hresMET_Calo_hw->Write(); hresJet_Incl_hw->Write();
    hresJet_HB_hw->Write();
    hresJet_HE1_hw->Write();
    hresJet_HE2_hw->Write();
    hresJet_HE_hw->Write();
    h_resMET1_PF_hw->Write();h_resMET2_PF_hw->Write();h_resMET3_PF_hw->Write();h_resMET4_PF_hw->Write();h_resMET5_PF_hw->Write();h_resMET6_PF_hw->Write();h_resMET7_PF_hw->Write();h_resMET8_PF_hw->Write();h_resMET9_PF_hw->Write();h_resMET10_PF_hw->Write();
    h_resMET1_Calo_hw->Write();h_resMET2_Calo_hw->Write();h_resMET3_Calo_hw->Write();h_resMET4_Calo_hw->Write();h_resMET5_Calo_hw->Write();h_resMET6_Calo_hw->Write();h_resMET7_Calo_hw->Write();h_resMET8_Calo_hw->Write();h_resMET9_Calo_hw->Write();h_resMET10_Calo_hw->Write();
    h_resJet1_hw->Write();h_resJet2_hw->Write();h_resJet3_hw->Write();h_resJet4_hw->Write();h_resJet5_hw->Write();h_resJet6_hw->Write();h_resJet7_hw->Write();h_resJet8_hw->Write();h_resJet9_hw->Write();h_resJet10_hw->Write();
  }


  
  std::cout << "Done " << nentries
	    << " events and (goodLumiEventCount = " << goodLumiEventCount 
	    << std::endl;
  printf("nEntries %lld,  \ngoodLumiEventCount %d, \nnEventsSelectedLevel1 %d, \nnEventsSelectedLevel2 %d, \nnEventsSelectedLevel3 %d,  \nnEventsSelectedLevel4 %d, \nnEventsSelectedLevel5 %d, \nnEventsSelectedLevel6 %d \n",
	 nentries,goodLumiEventCount,
	 nEventsSelectedLevel1, nEventsSelectedLevel2, nEventsSelectedLevel3,
	 nEventsSelectedLevel4, nEventsSelectedLevel5, nEventsSelectedLevel6);
    
}//closes the function 'rates'
