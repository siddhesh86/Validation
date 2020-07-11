#include "TMath.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TChain.h"
#include "TAttMarker.h"
#include <TCanvas.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <sstream>      // std::ostringstream
#include <glob.h> // glob(), globfree()
#include <stdexcept>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
//#include <io.h>
#include <stdio.h>

// Following headers help decode L1T ntuples
#include "L1Trigger/L1TNtuples/interface/L1AnalysisEventDataFormat.h"
#include "L1Trigger/L1TNtuples/interface/L1AnalysisL1UpgradeDataFormat.h"
#include "L1Trigger/L1TNtuples/interface/L1AnalysisRecoVertexDataFormat.h"
#include "L1Trigger/L1TNtuples/interface/L1AnalysisCaloTPDataFormat.h"
#include "L1Trigger/L1TNtuples/interface/L1AnalysisL1CaloTowerDataFormat.h"
#include "L1Trigger/L1TNtuples/interface/L1AnalysisRecoJetDataFormat.h"
#include "L1Trigger/L1TNtuples/interface/L1AnalysisRecoMetDataFormat.h"

#define N_IETA_BINS 80
#define N_IPHI_BINS 72
#define N_ETT_BINS 50
#define IETA_MIN -40.0
#define IETA_MAX 40.0
#define IPHI_MAX 72.0
#define ETT_RANGE 1000
#define N_INDIV_EVENTS 20


long l1NTuple_FileSizeThrsh = 0; //45000; // in bytes
Long64_t nEventsToAnalyze = 40000; // -1 to disable
int      runToanalyze = -1; //322106; // -1 to disable; 2018:  322106



// Number of events that pass any cuts, for normalising later
int nPassing = 0;

// 1d formatter
void formatPlot1D(TH1D* plot1d, int colour){
    plot1d->GetXaxis()->SetTitleOffset(1.2);
    plot1d->GetYaxis()->SetTitleOffset(1.4);
    plot1d->SetMinimum(0.);
    plot1d->SetLineColor(colour);
    plot1d->SetLineWidth(2);
    plot1d->Scale(1. / (double) nPassing);
    plot1d->Draw("HIST");
    plot1d->SetStats(false);
}

// 2d formatter
void formatPlot2D(TH2D* plot2d){
    plot2d->GetXaxis()->SetTitleOffset(1.2);
    plot2d->GetYaxis()->SetTitleOffset(1.4);
    plot2d->Draw("colz");
    plot2d->SetStats(false);
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

bool IsPathExist(const std::string &s)
{
  struct stat stat_buf;
  return (stat (s.c_str(), &stat_buf) == 0);
}

//main plotting function
//void doCaloTPStudy(int argc, char *argv[]){
int main(int argc, char *argv[]) {
  // argv[1]: emuSchemeTag; Available tag: Hardware: to run hardware trigger; or  PFA2: ; or any
  //                        emuSchemeTag is used to make o/p root file
  // argv[2]: inputFileDirectory

  std::cout << "argc: " << argc << std::endl;
  std::string emuSchemeTag("");
  std::string inputFileDirectory("");
  
  if (argc != 3) {
    std::cout << "Usage: l1jetanalysis.exe [new/def] [path to ntuples]\n"
              << "[new/def] indicates new or default (existing) conditions" << std::endl;
    exit(1);
  } else {
    emuSchemeTag = argv[1];
    inputFileDirectory = argv[2];
    std::cout << "emuSchemeTag: " << emuSchemeTag << std::endl;
  }


  std::vector<std::string> towers;
  std::vector<std::string> hcal;
  std::vector<std::string> ecal;


  std::string inputFile(inputFileDirectory);
  inputFile += "/L1Ntuple_*.root";

  std::cout << "i/p trees: " << inputFile << std::endl;
  std::vector<std::string> vInputFiles = glob(inputFile);
  std::cout << "no. of files: " << vInputFiles.size() << std::endl;
  for (size_t i=0; i<vInputFiles.size(); i++) {
    long fileSize = GetFileSize(vInputFiles[i]);
    //printf("\t %lu %ld %s\n",i,fileSize,vInputFiles[i].c_str());
    if (fileSize < l1NTuple_FileSizeThrsh) {
      //printf("\t\t\t fileSize (%ld) < l1NTuple_FileSizeThrsh (%ld)",fileSize, l1NTuple_FileSizeThrsh);
      continue;
    }

    std::string file(vInputFiles[i]);

    towers.push_back(file);
  }

  bool hwOn = false; 
  if (emuSchemeTag.find("Hardware") != std::string::npos) {
    hwOn = true;
    printf("doCaloTPStudy running on data hardware repacking mode:  hwOn = true \n\n");
  } else {
    printf("doCaloTPStudy running on data  reemulation mode:  hwOn = false \n\n");
  }

  

  // Number of entries to run over
  Long64_t nEvents;
  
  /*
  bool isTest = true;
  // Load files
  if (isTest == false){
    // Default: USE FOR TOWERS
    towers.push_back("/eos/cms/store/group/dpg_trigger/comm_trigger/L1Trigger/treis/l1t-integration-v97p17v2-CMSSW-1000/ZeroBias/crab_l1t-integration-v97p17v2-CMSSW-1000__ZeroBias_Run2017F-v1/180302_133644/0000/L1Ntuple_*.root");
    

    // HCAL
    hcal.push_back("/eos/cms/store/group/dpg_trigger/comm_trigger/L1Trigger/safarzad/2017/ZeroBias/HighPU/noRECO-l1t-v96p27_HCAL/ZeroBias8b4e1/crab_noRECO-l1t-v96p27_HCAL__8b4e1/170922_121415/0000/L1Ntuple_*.root");
   

    // ECAL
    ecal.push_back("/eos/cms/store/group/dpg_trigger/comm_trigger/L1Trigger/safarzad/2017/ZeroBias/HighPU/noRECO-l1t-v96p27_ECAL/ZeroBias8b4e1/crab_noRECO-l1t-v96p27_ECAL__8b4e1/170922_105053/0000/L1Ntuple_*.root");
    
    nEvents = 1500000;
  }

  else {
    // Load these files instead when testing
    towers.push_back("root://eoscms.cern.ch//eos/cms/store/group/dpg_trigger/comm_trigger/L1Trigger/safarzad/2017/ZeroBias/HighPU/noRECO-l1t-v96p27_NoPUS/ZeroBias8b4e1/crab_noRECO-l1t-v96p27_NoPUS__8b4e1/170915_101410/0000/L1Ntuple_6.root");
    hcal.push_back("root://eoscms.cern.ch//eos/cms/store/group/dpg_trigger/comm_trigger/L1Trigger/safarzad/2017/ZeroBias/HighPU/noRECO-l1t-v96p27_HCAL/ZeroBias8b4e1/crab_noRECO-l1t-v96p27_HCAL__8b4e1/170922_121415/0000/L1Ntuple_6.root");
    ecal.push_back("root://eoscms.cern.ch//eos/cms/store/group/dpg_trigger/comm_trigger/L1Trigger/safarzad/2017/ZeroBias/HighPU/noRECO-l1t-v96p27_ECAL/ZeroBias8b4e1/crab_noRECO-l1t-v96p27_ECAL__8b4e1/170922_105053/0000/L1Ntuple_6.root");

    nEvents = 1000;
  }
  */

  std::cout << "Loading up the TChain..." << std::endl;
  TChain * eventTree;
  TChain * treeL1Towemu;
  TChain * treeTPemu;
  TChain * treeL1emu;
  //TChain * treeL1HCALemu;
  //TChain * treeL1ECALemu;

  eventTree = new TChain("l1EventTree/L1EventTree");
  if ( ! hwOn) {
    treeL1Towemu  = new TChain("l1CaloTowerEmuTree/L1CaloTowerTree");
    treeTPemu     = new TChain("l1CaloTowerEmuTree/L1CaloTowerTree");
    treeL1emu     = new TChain("l1UpgradeEmuTree/L1UpgradeTree");
    //treeL1HCALemu = new TChain("l1UpgradeEmuTree/L1UpgradeTree");
    //treeL1ECALemu = new TChain("l1UpgradeEmuTree/L1UpgradeTree");
  } else {
    // hardware mode
    treeL1Towemu  = new TChain("l1CaloTowerTree/L1CaloTowerTree");
    treeTPemu     = new TChain("l1CaloTowerTree/L1CaloTowerTree");
    treeL1emu     = new TChain("l1UpgradeTree/L1UpgradeTree");
    //treeL1HCALemu = new TChain("l1UpgradeTree/L1UpgradeTree");
    //treeL1ECALemu = new TChain("l1UpgradeTree/L1UpgradeTree");
  }
  
  /*
  uint minFiles = std::min( std::min( towers.size(), hcal.size() ), ecal.size() );

  for (uint i = 0; i < minFiles; ++i) {
    eventTree->Add(towers[i].c_str());
    treeL1Towemu->Add(towers[i].c_str());
    treeTPemu->Add(towers[i].c_str());
    treeL1emu->Add(towers[i].c_str());
    treeL1HCALemu->Add(hcal[i].c_str());
    treeL1ECALemu->Add(ecal[i].c_str());
  }
  */
  for (uint i = 0; i < towers.size(); ++i) {
    eventTree->Add(towers[i].c_str());
    treeL1Towemu->Add(towers[i].c_str());
    treeTPemu->Add(towers[i].c_str());
    treeL1emu->Add(towers[i].c_str());
  }
  

  L1Analysis::L1AnalysisEventDataFormat           *event_ = new L1Analysis::L1AnalysisEventDataFormat();
  L1Analysis::L1AnalysisL1CaloTowerDataFormat     *l1Towemu_ = new L1Analysis::L1AnalysisL1CaloTowerDataFormat();
  L1Analysis::L1AnalysisCaloTPDataFormat          *l1TPemu_ = new L1Analysis::L1AnalysisCaloTPDataFormat();
  L1Analysis::L1AnalysisL1UpgradeDataFormat       *l1emu_ = new L1Analysis::L1AnalysisL1UpgradeDataFormat();
  //L1Analysis::L1AnalysisL1UpgradeDataFormat       *l1HCALemu_ = new L1Analysis::L1AnalysisL1UpgradeDataFormat();
  //L1Analysis::L1AnalysisL1UpgradeDataFormat       *l1ECALemu_ = new L1Analysis::L1AnalysisL1UpgradeDataFormat();

  eventTree->SetBranchAddress("Event", &event_);
  treeL1Towemu->SetBranchAddress("L1CaloTower", &l1Towemu_);
  treeTPemu->SetBranchAddress("CaloTP", &l1TPemu_);
  treeL1emu->SetBranchAddress("L1Upgrade", &l1emu_);
  //treeL1HCALemu->SetBranchAddress("L1Upgrade", &l1HCALemu_);
  //treeL1ECALemu->SetBranchAddress("L1Upgrade", &l1ECALemu_);

  // get number of entries
  Long64_t nentriesTowers;
  //Long64_t nentriesHCAL;
  //Long64_t nentriesECAL;
  Long64_t nentries;

  nentriesTowers = treeL1emu->GetEntries();
  //nentriesHCAL = treeL1HCALemu->GetEntries();
  //nentriesECAL = treeL1ECALemu->GetEntries();

  //nentries = std::min(std::min(nentriesTowers, nentriesHCAL), nentriesECAL);
  nentries = nentriesTowers;
    
  // Initialise histograms
  TH1D* hnEventsAnalyzed = new TH1D("nEventsAnalyzed", "", 1, -0.5, 0.5);
  TH1D* hnEventsSelected = new TH1D("nEventsSelected", "", 1, -0.5, 0.5);
  

  // Tower hists
  TH1D* hAllTowEt = new TH1D("towerEt", ";Tower E_{T}; # Towers", 40, -0.5, 39.5);
  TH1D* hAllTowEta = new TH1D("towerEta", "Towers vs iEta;iEta; # Towers", N_IETA_BINS, IETA_MIN, IETA_MAX);
  TH1D* hTowTPETEta = new TH1D("towerTPETEta", "Average tower TP E_{T} vs. iEta; iEta; Average tower TP E_{T}", N_IETA_BINS, IETA_MIN, IETA_MAX);

  TH1D* hTowPhiB = new TH1D("towerPhiB", "Towers vs iPhi in Barrel;iPhi; # Towers", N_IPHI_BINS, 0., IPHI_MAX);
  TH1D* hTowPhiE = new TH1D("towerPhiE", "Towers vs iPhi in End cap;iPhi; # Towers", N_IPHI_BINS, 0., IPHI_MAX);

  TH1D* hTowTPETphiB = new TH1D("towerTPETPhiB", "Average tower TP E_{T} vs. iPhi in Barrel;iPhi; Average tower TP E_{T}", N_IPHI_BINS, 0., IPHI_MAX);
  TH1D* hTowTPETphiE = new TH1D("towerTPETPhiE", "Average tower TP E_{T} vs. iPhi in End cap;iPhi; Average tower TP E_{T}", N_IPHI_BINS, 0., IPHI_MAX);

  // HCAL hists
  TH1D* hHCALTPEt = new TH1D("hcalTPEt", "HCAL;TP E_{T}; # TPs / Event", 20, 0., 200.);
  TH1D* hHCALTPEta = new TH1D("hcalTPEta", "HCAL # TPs vs iEta;iEta; # TPs / Event", N_IETA_BINS, IETA_MIN, IETA_MAX);
  TH1D* hHCALTPETEta = new TH1D("hcalTPETEta", "Average HCAL TP E_{T} vs. iEta; iEta; Average HCAL TP E_{T}", N_IETA_BINS, IETA_MIN, IETA_MAX);

  TH1D* hHCALTPPhi = new TH1D("hcalTPPhi", "HCAL # TPs vs iPhi;iPhi; # TPs / Event", N_IPHI_BINS, 0., IPHI_MAX);
  TH1D* hHCALTPPhiHB = new TH1D("hcalTPPhiHB", "HCAL # TPs vs iPhi in HB;iPhi; # TPs / Event", N_IPHI_BINS, 0., IPHI_MAX);
  TH1D* hHCALTPPhiHE = new TH1D("hcalTPPhiHE", "HCAL # TPs vs iPhi in HE;iPhi; # TPs / Event", N_IPHI_BINS, 0., IPHI_MAX);
  TH1D* hHCALTPPhiHF = new TH1D("hcalTPPhiHF", "HCAL # TPs vs iPhi in HF;iPhi; # TPs / Event", N_IPHI_BINS, 0., IPHI_MAX);

  TH1D* hHCALTPETphi = new TH1D("hcalTPETPhi", "Average HCAL TP E_{T} vs. iPhi;iPhi; Average HCAL TP E_{T}", N_IPHI_BINS, 0., IPHI_MAX);
  TH1D* hHCALTPETphiHB = new TH1D("hcalTPETPhiHB", "Average HCAL TP E_{T} vs. iPhi in HB;iPhi; Average HCAL TP E_{T}", N_IPHI_BINS, 0., IPHI_MAX);
  TH1D* hHCALTPETphiHE = new TH1D("hcalTPETPhiHE", "Average HCAL TP E_{T} vs. iPhi in HE;iPhi; Average HCAL TP E_{T}", N_IPHI_BINS, 0., IPHI_MAX);
  TH1D* hHCALTPETphiHF = new TH1D("hcalTPETPhiHF", "Average HCAL TP E_{T} vs. iPhi in HF;iPhi; Average HCAL TP E_{T}", N_IPHI_BINS, 0., IPHI_MAX);

  TH2D* hHCALTPEtaPhi = new TH2D("hcaletaphi", "HCAL TP occupancy;TP iEta;TP iPhi", N_IETA_BINS, IETA_MIN, IETA_MAX, N_IPHI_BINS, 0., IPHI_MAX);
  TH2D* hHCALavgTPETEtaPhi = new TH2D("hcalavgtpetetaphi", "HCAL average TP E_{T} per bin;TP iEta;TP iPhi", N_IETA_BINS, IETA_MIN, IETA_MAX, N_IPHI_BINS, .0, IPHI_MAX);

  TH1D* hHCALTPPhiforTT28 = new TH1D("hcaltpphiTT28", "HCAL # TPs vs iPhi for TT28;iPhi;# TPs / Event", N_IPHI_BINS, 0., IPHI_MAX);
  TH1D* hHCALTPETphiforTT28 = new TH1D("hcaltpetphiTT28", "Average HCAL TP E_{T} vs iPhi for TT28;iPhi; Average HCAL TP E_{T}", N_IPHI_BINS, 0., IPHI_MAX);
  TH2D* hHCALavgTPETforTT28 = new TH2D("hcalavgetTT28", "HCAL average TP E_{T} per bin for TT28;TP iEta;TP iPhi", N_IETA_BINS, IETA_MIN, IETA_MAX, N_IPHI_BINS, .0, IPHI_MAX);

  // ECAL hists
  TH1D* hECALTPEt = new TH1D("ecalTPEt", "ECAL;TP E_{T}; # TPs / Event", 20, 0., 200.);
  TH1D* hECALTPEta = new TH1D("ecalTPEta", "ECAL # TPs vs iEta;iEta; # TPs / Event", N_IETA_BINS, IETA_MIN, IETA_MAX);
  TH1D* hECALTPETEta = new TH1D("ecalTPETEta", "Average ECAL TP E_{T} vs. iEta; iEta; Average ECAL TP E_{T}", N_IETA_BINS, IETA_MIN, IETA_MAX);

  TH1D* hECALTPPhi = new TH1D("ecalTPPhi", "ECAL # TPs vs iPhi;iPhi; # TPs / Event", N_IPHI_BINS, 0., IPHI_MAX);
  TH1D* hECALTPPhiEB = new TH1D("ecalTPPhiEB", "ECAL # TPs vs iPhi in EB;iPhi; # TPs / Event", N_IPHI_BINS, 0., IPHI_MAX);
  TH1D* hECALTPPhiEE = new TH1D("ecalTPPhiEE", "ECAL # TPs vs iPhi in EE;iPhi; # TPs / Event", N_IPHI_BINS, 0., IPHI_MAX);

  TH1D* hECALTPETphi = new TH1D("ecalTPETPhi", "Average ECAL TP E_{T} vs. iPhi;iPhi; Average ECAL TP E_{T}", N_IPHI_BINS, 0., IPHI_MAX);
  TH1D* hECALTPETphiEB = new TH1D("ecalTPETPhiEB", "Average ECAL TP E_{T} vs. iPhi in EB;iPhi; Average ECAL TP E_{T}", N_IPHI_BINS, 0., IPHI_MAX);
  TH1D* hECALTPETphiEE = new TH1D("ecalTPETPhiEE", "Average ECAL TP E_{T} vs. iPhi in EE;iPhi; Average ECAL TP E_{T}", N_IPHI_BINS, 0., IPHI_MAX);

  TH2D* hECALTPEtaPhi = new TH2D("ecaletaphi", "ECAL TP occupancy;TP iEta;TP iPhi", N_IETA_BINS, IETA_MIN, IETA_MAX, N_IPHI_BINS, .0, IPHI_MAX);
  TH2D* hECALavgTPETEtaPhi = new TH2D("ecalavgtpetetaphi", "ECAL average TP E_{T} per bin;TP iEta;TP iPhi", N_IETA_BINS, IETA_MIN, IETA_MAX, N_IPHI_BINS, .0, IPHI_MAX);

  TH1D* hECALTPPhiforTT28 = new TH1D("ecaltpphiTT28", "ECAL # TPs vs iPhi for TT28;iPhi;# TPs / Event", N_IPHI_BINS, 0., IPHI_MAX);
  TH1D* hECALTPETphiforTT28 = new TH1D("ecaltpetphiTT28", "Average ECAL TP E_{T} vs iPhi for TT28;iPhi; Average ECAL TP E_{T}", N_IPHI_BINS, 0., IPHI_MAX);
  TH2D* hECALavgTPETforTT28 = new TH2D("ecalavgetTT28", "ECAL average TP E_{T} per bin for TT28;TP iEta;TP iPhi", N_IETA_BINS, IETA_MIN, IETA_MAX, N_IPHI_BINS, .0, IPHI_MAX);

  // MET hists
  TH1D* hMetPhi = new TH1D("hMetPhi", ";MET Phi;# Events", N_IPHI_BINS, 0., 2*IPHI_MAX); // each bin (and therefore range) is 2*iPhi
  TH2D* hMetPhiEcalHcal = new TH2D("hMetPhiEcalHcal", "MET Phi ECAL vs HCAL;MET Phi ECAL;MET Phi HCAL", N_IPHI_BINS, 0., 2*IPHI_MAX, N_IPHI_BINS, 0., 2*IPHI_MAX);
  TH2D* hMetPhiEcalTotal = new TH2D("hMetPhiEcalTotal", "MET Phi ECAL vs total;MET Phi ECAL;MET Phi total", N_IPHI_BINS, 0., 2*IPHI_MAX, N_IPHI_BINS, 0., 2*IPHI_MAX);
  TH2D* hMetPhiHcalTotal = new TH2D("hMetPhiHcalTotal", "MET Phi HCAL vs total;MET Phi HCAL;MET Phi total", N_IPHI_BINS, 0., 2*IPHI_MAX, N_IPHI_BINS, 0., 2*IPHI_MAX);

  TH1D* hMetScal = new TH1D("hMetScal", ";MET scalar sum; # Events", 20, 100., 200);
  TH2D* hMetScalEcalHcal = new TH2D("hMetScalEcalHcal", "MET scalar ECAL vs HCAL;MET scalar ECAL;MET scalar HCAL", 40, 0., 200, 40, 0., 200);
  TH2D* hMetScalEcaltotal = new TH2D("hMetScalEcaltotal", "MET scalar ECAL vs sum;MET scalar ECAL;MET scalar sum", 40, 0., 200, 20, 100., 200);
  TH2D* hMetScalHcaltotal = new TH2D("hMetScalHcaltotal", "MET scalar HCAL vs sum;MET scalar HCAL;MET scalar sum", 40, 0., 200, 20, 100., 200);

  // ETT hists
  TH1D* hEttScal = new TH1D("hEttScal", ";ETT scalar sum; # Events", 2*N_ETT_BINS, 0., 2*ETT_RANGE);
  TH2D* hEttScalEcalHcal = new TH2D("hEttScalEcalHcal", "ETT scalar ECAL vs HCAL;ETT scalar ECAL;ETT scalar HCAL", N_ETT_BINS, 0., ETT_RANGE, N_ETT_BINS, 0., ETT_RANGE);
  TH2D* hEttScalEcaltotal = new TH2D("hEttScalEcaltotal", "ETT scalar ECAL vs sum;ETT scalar ECAL;ETT scalar sum", N_ETT_BINS, 0., ETT_RANGE, 2*N_ETT_BINS, 0., 2*ETT_RANGE);
  TH2D* hEttScalHcaltotal = new TH2D("hEttScalHcaltotal", "ETT scalar HCAL vs sum;ETT scalar HCAL;ETT scalar sum", N_ETT_BINS, 0., ETT_RANGE, 2*N_ETT_BINS, 0., 2*ETT_RANGE);

  // Histogram arrays for storing individual event information
  TH2D* hECALTPETEtaPhiIndiv[N_INDIV_EVENTS] = { NULL };
  TH2D* hHCALTPETEtaPhiIndiv[N_INDIV_EVENTS] = { NULL };

  // Main loop
  nEvents = nentries;  
  std::cout << "Tree nEntries " << nEvents << std::endl;
  //if (nEvents > 10000) nEvents = 10000;

  if (nEventsToAnalyze != -1) {
    nEvents = std::min(nEvents, nEventsToAnalyze);
  }
  std::cout << "nEvents to analyze: " << nEvents << std::endl;
  
  for (Long64_t jentry = 0; jentry < nEvents; ++jentry) {
    // initialise some variables
    int nHCALTPemu(0), nECALTPemu(0), nTowemu(-1);  // nTPs
    double hcalTPEtEm(0), ecalTPEtEm(0), towEtemu(0);  // E_T
    int hcalTPEtaEm(-1), ecalTPEtaEm(-1), towEtaemu(-1);  // iEta
    int hcalTPPhiEm(-1), ecalTPPhiEm(-1), towPhiemu(-1);  // iPhi

    double l1MetEmu(0.);
    double l1MetHCALEmu(0.);
    double l1MetECALEmu(0.);

    double l1EttEmu(0.);
    double l1EttHCALEmu(0.);
    double l1EttECALEmu(0.);

    double l1MetPhiEmu(-1.);
    double l1MetPhiHCALEmu(-1.);
    double l1MetPhiECALEmu(-1.);

    // run number
    int run(322106); // 306042

    //counter
    if( (jentry % 10000) == 0 ) std::cout << "Done " << jentry << " events of " << nEvents << std::endl;
    //if( (jentry % 1000) == 0 ) std::cout << "." << flush;

    eventTree->GetEntry(jentry);

    run = event_->run;
    int lumi = event_->lumi;
    int event = event_->event;
    if (1==0) std::cout << "run " << run << ", lumi " << lumi << " event " << event << std::endl;

    if (runToanalyze != -1 && run != runToanalyze) {
      std::cout << "run " << run << ", lumi " << lumi << " event " << event
		<< "\t run != 322106 \t\t *** ERROR ***"<< std::endl;
      exit(0);
    }

    hnEventsAnalyzed->Fill(0);
    
    treeL1Towemu->GetEntry(jentry);
    treeTPemu->GetEntry(jentry);
    treeL1emu->GetEntry(jentry);
    //treeL1HCALemu->GetEntry(jentry);
    //treeL1ECALemu->GetEntry(jentry);

    nTowemu = l1Towemu_->nTower;

    nHCALTPemu = l1TPemu_->nHCALTP;
    nECALTPemu = l1TPemu_->nECALTP;

    // Retrieve MET, ETT from emulator tree

    for (unsigned int c = 0; c < l1emu_->nSums; ++c) { // Iterates over the different sums in the event. Gets ET and MET from their sum type
      if( l1emu_->sumBx[c] != 0 ) continue;
      if( l1emu_->sumType[c] == L1Analysis::kTotalEt ) l1EttEmu = l1emu_->sumEt[c];
      else if( l1emu_->sumType[c] == L1Analysis::kMissingEt ) {
	l1MetEmu = l1emu_->sumEt[c];
	l1MetPhiEmu = l1emu_->sumIPhi[c];
      }
      if (l1EttEmu > 0. && l1MetEmu > 0. && l1MetPhiEmu > -1.)  // If values found for these variables, break out of loop. Makes execution a little faster
	break;
    }

    // Only loop over events with MET > 100 GeV and set overflow events (MET > 200 GeV) to 200 GeV
    //if (l1MetEmu < 99.9) continue;
    //else if (l1MetEmu > 200.) l1MetEmu = 200.;

    ++nPassing;

    hnEventsSelected->Fill(0);
    

    // Retrieve MET and ETT for HCAL and ECAL from emulator tree
    /*
    for (unsigned int c = 0; c < l1HCALemu_->nSums; ++c) {
      if( l1HCALemu_->sumBx[c] != 0 ) continue;
      if( l1HCALemu_->sumType[c] == L1Analysis::kTotalEt ) l1EttHCALEmu = l1HCALemu_->sumEt[c];
      else if( l1HCALemu_->sumType[c] == L1Analysis::kMissingEt ) {
	l1MetHCALEmu = l1HCALemu_->sumEt[c];
	l1MetPhiHCALEmu = l1HCALemu_->sumIPhi[c];
      }
      if (l1EttHCALEmu > 0. && l1MetHCALEmu > 0. && l1MetPhiHCALEmu > -1.)
	break;
    }

    for (unsigned int c = 0; c < l1ECALemu_->nSums; ++c) {
      if( l1ECALemu_->sumBx[c] != 0 ) continue;
      if( l1ECALemu_->sumType[c] == L1Analysis::kTotalEt ) l1EttECALEmu = l1ECALemu_->sumEt[c];
      else if( l1ECALemu_->sumType[c] == L1Analysis::kMissingEt ) {
	l1MetECALEmu = l1ECALemu_->sumEt[c];
	l1MetPhiECALEmu = l1ECALemu_->sumIPhi[c];
      }
      if (l1EttECALEmu > 0. && l1MetECALEmu > 0. && l1MetPhiECALEmu > -1.)
	break;
    }
    */
    // Fill ETT scalar histos
    hEttScal->Fill(l1EttEmu);
    hEttScalEcalHcal->Fill(l1EttECALEmu, l1EttHCALEmu);
    hEttScalEcaltotal->Fill(l1EttECALEmu, l1EttEmu);
    hEttScalHcaltotal->Fill(l1EttHCALEmu, l1EttEmu);

    // Fill MET scalar histos
    hMetScal->Fill(l1MetEmu);
    hMetScalEcalHcal->Fill(l1MetECALEmu, l1MetHCALEmu);
    hMetScalEcaltotal->Fill(l1MetECALEmu, l1MetEmu);
    hMetScalHcaltotal->Fill(l1MetHCALEmu, l1MetEmu);

    // Fill MET phi histos
    hMetPhi->Fill(l1MetPhiEmu);
    hMetPhiEcalHcal->Fill(l1MetPhiECALEmu, l1MetPhiHCALEmu);
    hMetPhiEcalTotal->Fill(l1MetPhiECALEmu, l1MetPhiEmu);
    hMetPhiHcalTotal->Fill(l1MetPhiHCALEmu, l1MetPhiEmu);

    // Initialise histograms in arrays for filling with individual event info
    if (N_INDIV_EVENTS >= nPassing) {
      printf("Plot individual evet info (max %d): event %d \n",N_INDIV_EVENTS, nPassing);
      std::ostringstream hcalHistNameStream;
      hcalHistNameStream << "hcalTPETetaphiindiv" << nPassing;
      hHCALTPETEtaPhiIndiv[(int) nPassing - 1] = new TH2D(hcalHistNameStream.str().c_str(), "HCAL TP E_{T} for single event;iEta;iPhi", N_IETA_BINS, IETA_MIN, IETA_MAX, N_IPHI_BINS, 0., IPHI_MAX);

      std::ostringstream ecalHistNameStream;
      ecalHistNameStream << "ecalTPETetaphiindiv" << nPassing;
      hECALTPETEtaPhiIndiv[(int) nPassing - 1] = new TH2D(ecalHistNameStream.str().c_str(), "ECAL TP E_{T} for single event;iEta;iPhi", N_IETA_BINS, IETA_MIN, IETA_MAX, N_IPHI_BINS, 0., IPHI_MAX);
    }

    // Retrieve HCAL objects from emulator tree

    for (uint tpIt = 0; tpIt < (uint)nHCALTPemu; ++tpIt) {
      hcalTPEtEm = l1TPemu_->hcalTPet[tpIt];
      hcalTPEtaEm = l1TPemu_->hcalTPieta[tpIt];
      hcalTPPhiEm = l1TPemu_->hcalTPiphi[tpIt];

      // Fill arrays and histos for each detector section
      if (abs(hcalTPEtaEm) <= 16) {
	hHCALTPPhiHB->Fill(hcalTPPhiEm);
	hHCALTPETphiHB->Fill(hcalTPPhiEm, hcalTPEtEm);
      }

      else if (abs(hcalTPEtaEm) > 16 && abs(hcalTPEtaEm) <= 28) {
	hHCALTPPhiHE->Fill(hcalTPPhiEm);
	hHCALTPETphiHE->Fill(hcalTPPhiEm, hcalTPEtEm);
	if (abs(hcalTPEtaEm) == 28) {
	  hHCALTPPhiforTT28->Fill(hcalTPPhiEm);
	  hHCALTPETphiforTT28->Fill(hcalTPPhiEm, hcalTPEtEm);
	  hHCALavgTPETforTT28->Fill(hcalTPEtaEm, hcalTPPhiEm, hcalTPEtEm);
	}
      }

      else {
	hHCALTPPhiHF->Fill(hcalTPPhiEm);
	hHCALTPETphiHF->Fill(hcalTPPhiEm, hcalTPEtEm);
      }

      hHCALTPEt->Fill(hcalTPEtEm);
      hHCALTPEta->Fill(hcalTPEtaEm);
      hHCALTPETEta->Fill(hcalTPEtaEm, hcalTPEtEm);
      hHCALTPPhi->Fill(hcalTPPhiEm);
      hHCALTPETphi->Fill(hcalTPPhiEm, hcalTPEtaEm);
      hHCALTPEtaPhi->Fill(hcalTPEtaEm, hcalTPPhiEm);
      // Fill eta-phi histogram with the TP ETs
      hHCALavgTPETEtaPhi->Fill(hcalTPEtaEm, hcalTPPhiEm, hcalTPEtEm);
      if (N_INDIV_EVENTS > nPassing)
	hHCALTPETEtaPhiIndiv[(int) nPassing - 1]->Fill(hcalTPEtaEm, hcalTPPhiEm, hcalTPEtEm);

    }

    // Retrieve ECAL objects from emulator tree

    for (uint tpIt = 0; tpIt < (uint)nECALTPemu; ++tpIt){
      ecalTPEtEm = l1TPemu_->ecalTPet[tpIt];
      ecalTPEtaEm = l1TPemu_->ecalTPieta[tpIt];
      ecalTPPhiEm = l1TPemu_->ecalTPiphi[tpIt];

      if (abs(ecalTPEtaEm) <= 16) {
	hECALTPPhiEB->Fill(ecalTPPhiEm);
	hECALTPETphiEB->Fill(ecalTPPhiEm, ecalTPEtEm);
      }

      else if (abs(ecalTPEtaEm) > 16 && abs(ecalTPEtaEm) <= 28) {
	hECALTPPhiEE->Fill(ecalTPPhiEm);
	hECALTPETphiEE->Fill(ecalTPPhiEm, ecalTPEtEm);
	if (abs(ecalTPEtaEm) == 28) {
	  hECALTPPhiforTT28->Fill(ecalTPPhiEm);
	  hECALTPETphiforTT28->Fill(ecalTPPhiEm, ecalTPEtEm);
	  hECALavgTPETforTT28->Fill(ecalTPEtaEm, ecalTPPhiEm, ecalTPEtEm);
	}
      }

      hECALTPEt->Fill(ecalTPEtEm);
      hECALTPEta->Fill(ecalTPEtaEm);
      hECALTPETEta->Fill(ecalTPEtaEm, ecalTPEtEm);
      hECALTPPhi->Fill(ecalTPPhiEm);
      hECALTPETphi->Fill(ecalTPPhiEm, ecalTPEtEm);
      hECALTPEtaPhi->Fill(ecalTPEtaEm, ecalTPPhiEm);
      hECALavgTPETEtaPhi->Fill(ecalTPEtaEm, ecalTPPhiEm, ecalTPEtEm);
      if (N_INDIV_EVENTS > nPassing)
	hECALTPETEtaPhiIndiv[(int) nPassing - 1]->Fill(ecalTPEtaEm, ecalTPPhiEm, ecalTPEtEm);

    }

    // Retrieve tower objects from the emulator tree

    for(uint towIt = 0; towIt < (uint)nTowemu; ++towIt){
      towEtemu  = l1Towemu_->iet[towIt];
      towEtaemu = l1Towemu_->ieta[towIt];
      towPhiemu = l1Towemu_->iphi[towIt];

      if (abs(towEtaemu) <= 16) {
	hTowPhiB->Fill(towPhiemu);
	hTowTPETphiB->Fill(towPhiemu, towEtemu);
      }

      else if (abs(towEtaemu) > 16 && abs(towEtaemu) <= 28) {
	hTowPhiE->Fill(towPhiemu);
	hTowTPETphiE->Fill(towPhiemu, towEtemu);
      } // Ignoring HF

      hAllTowEt->Fill(towEtemu);
      hAllTowEta->Fill(towEtaemu);
      hTowTPETEta->Fill(towEtaemu, towEtemu);

    }

  }

  // End event loop, now plot histos

  printf("\n\n\nnEventsAnalyzed %g \t nEventsSelected %g \n\n",
	 hnEventsAnalyzed->GetBinContent(1),
	 hnEventsSelected->GetBinContent(1)); 

  
  TFile *tFOut = new TFile(Form("histo_CaloTP_%s.root",emuSchemeTag.c_str()), "RECREATE");
  tFOut->cd();

  hnEventsAnalyzed->Write();
  hnEventsSelected->Write();

  
  TCanvas* canvas = new TCanvas("canvas", "", 550, 500);
  
  int ecalColour = 2;
  int hcalColour = 4;
  int towerColour = 3;
  
  std::string sDirOut = Form("./Plots_%s",emuSchemeTag.c_str());
  if ( ! IsPathExist(sDirOut)) {
    mkdir(sDirOut.c_str(), S_IRWXU);
  }
  std::string sDirOut1 = Form("%s/ECAL",sDirOut.c_str());
  if ( ! IsPathExist(sDirOut1)) {
    mkdir(sDirOut1.c_str(), S_IRWXU);
  }
  sDirOut1 = Form("%s/HCAL",sDirOut.c_str());
  if ( ! IsPathExist(sDirOut1)) {
    mkdir(sDirOut1.c_str(), S_IRWXU);
  }
  sDirOut1 = Form("%s/Towers",sDirOut.c_str());
  if ( ! IsPathExist(sDirOut1)) {
    mkdir(sDirOut1.c_str(), S_IRWXU);
  }
  sDirOut1 = Form("%s/Individual_Events",sDirOut.c_str());
  if ( ! IsPathExist(sDirOut1)) {
    mkdir(sDirOut1.c_str(), S_IRWXU);
  }
  
  // Clone histograms for other plots, rather than filling twice
  TH2D* hHCALavgTPETperEvEtaPhi = (TH2D*)hHCALavgTPETEtaPhi->Clone("hcalavgevetaphi");
  hHCALavgTPETperEvEtaPhi->Scale(1. / nPassing); // because formatPlot2D doesn't normalise
  hHCALavgTPETperEvEtaPhi->SetTitle("HCAL average TP E_{T} per event");

  TH2D* hECALavgTPETperEvEtaPhi = (TH2D*)hECALavgTPETEtaPhi->Clone("ecalavgevetaphi");
  hECALavgTPETperEvEtaPhi->Scale(1. / nPassing);
  hECALavgTPETperEvEtaPhi->SetTitle("ECAL average TP E_{T} per event");

  // Plot TP ET vs iEta for ECAL, HCAL and towers
  // Need to do divide before plotting occupancy otherwise normalisation is wrong
  hECALTPETEta->Divide(hECALTPEta);
  hECALTPETEta->Scale(nPassing); // Offset normalisation in formatPlot1D
  formatPlot1D(hECALTPETEta, ecalColour);
  canvas->SaveAs(Form("%s/ECAL/ECALTPETEta.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/ECAL/ECALTPETEta.png",sDirOut.c_str()));
  hECALTPETEta->Write();
  
  hHCALTPETEta->Divide(hHCALTPEta);
  hHCALTPETEta->Scale(nPassing); 
  formatPlot1D(hHCALTPETEta, hcalColour);
  canvas->SaveAs(Form("%s/HCAL/HCALTPETEta.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/HCAL/HCALTPETEta.png",sDirOut.c_str()));
  hHCALTPETEta->Write();

  hTowTPETEta->Divide(hAllTowEta);
  hTowTPETEta->Scale(nPassing);
  formatPlot1D(hTowTPETEta, towerColour);
  canvas->SaveAs(Form("%s/Towers/TowTPETEta.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/Towers/TowTPETEta.png",sDirOut.c_str()));
  hTowTPETEta->Write();
  
  // Plot nTPs vs iEta for ECAL, HCAL and towers
  formatPlot1D(hECALTPEta, ecalColour);
  canvas->SaveAs(Form("%s/ECAL/ECALTPEta.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/ECAL/ECALTPEta.png",sDirOut.c_str()));
  hECALTPEta->Write();
  
  formatPlot1D(hHCALTPEta, hcalColour);
  canvas->SaveAs(Form("%s/HCAL/HCALTPEta.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/HCAL/HCALTPEta.png",sDirOut.c_str()));
  hHCALTPEta->Write();
  
  formatPlot1D(hAllTowEta, towerColour);
  canvas->SaveAs(Form("%s/Towers/TowEta.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/Towers/TowEta.png",sDirOut.c_str()));
  hAllTowEta->Write();
  
  // Plot TP ET vs iPhi for ECAL, EB, EE, HCAL, HB, HE, HF, tower B, tower E, TT28
  hECALTPETphi->Divide(hECALTPPhi);
  hECALTPETphi->Scale(nPassing);
  formatPlot1D(hECALTPETphi, ecalColour);
  canvas->SaveAs(Form("%s/ECAL/ECALTPETphi.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/ECAL/ECALTPETphi.png",sDirOut.c_str()));
  hECALTPETphi->Write();
  
  hECALTPETphiEB->Divide(hECALTPPhiEB);
  hECALTPETphiEB->Scale(nPassing);
  formatPlot1D(hECALTPETphiEB, ecalColour);
  canvas->SaveAs(Form("%s/ECAL/ECALTPETphiEB.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/ECAL/ECALTPETphiEB.png",sDirOut.c_str()));
  hECALTPETphiEB->Write();
  
  hECALTPETphiEE->Divide(hECALTPPhiEE);
  hECALTPETphiEE->Scale(nPassing);
  formatPlot1D(hECALTPETphiEE, ecalColour);
  canvas->SaveAs(Form("%s/ECAL/ECALTPETphiEE.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/ECAL/ECALTPETphiEE.png",sDirOut.c_str()));
  hECALTPETphiEE->Write();
  
  hECALTPETphiforTT28->Divide(hECALTPPhiforTT28);
  hECALTPETphiforTT28->Scale(nPassing);
  formatPlot1D(hECALTPETphiforTT28, ecalColour);
  canvas->SaveAs(Form("%s/ECAL/ECALTPETphiforTT28.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/ECAL/ECALTPETphiforTT28.png",sDirOut.c_str()));
  hECALTPETphiforTT28->Write();
  
  hHCALTPETphi->Divide(hHCALTPPhi);
  hHCALTPETphi->Scale(nPassing);
  formatPlot1D(hHCALTPETphi, hcalColour);
  canvas->SaveAs(Form("%s/HCAL/HCALTPETphi.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/HCAL/HCALTPETphi.pdf",sDirOut.c_str()));
  hHCALTPETphi->Write();
  
  hHCALTPETphiHB->Divide(hHCALTPPhiHB);
  hHCALTPETphiHB->Scale(nPassing);
  formatPlot1D(hHCALTPETphiHB, hcalColour);
  canvas->SaveAs(Form("%s/HCAL/HCALTPETphiHB.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/HCAL/HCALTPETphiHB.png",sDirOut.c_str()));
  hHCALTPETphiHB->Write();
  
  hHCALTPETphiHE->Divide(hHCALTPPhiHE);
  hHCALTPETphiHE->Scale(nPassing);
  formatPlot1D(hHCALTPETphiHE, hcalColour);
  canvas->SaveAs(Form("%s/HCAL/HCALTPETphiHE.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/HCAL/HCALTPETphiHE.png",sDirOut.c_str()));
  hHCALTPETphiHE->Write();
  
  hHCALTPETphiHF->Divide(hHCALTPPhiHF);
  hHCALTPETphiHF->Scale(nPassing);
  formatPlot1D(hHCALTPETphiHF, hcalColour);
  canvas->SaveAs(Form("%s/HCAL/HCALTPETphiHF.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/HCAL/HCALTPETphiHF.png",sDirOut.c_str()));
  hHCALTPETphiHF->Write();
  
  hHCALTPETphiforTT28->Divide(hHCALTPPhiforTT28);
  hHCALTPETphiforTT28->Scale(nPassing);
  formatPlot1D(hHCALTPETphiforTT28, hcalColour);
  canvas->SaveAs(Form("%s/HCAL/HCALTPETphiforTT28.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/HCAL/HCALTPETphiforTT28.png",sDirOut.c_str()));
  hHCALTPETphiforTT28->Write();
  
  hTowTPETphiB->Divide(hTowPhiB);
  hTowTPETphiB->Scale(nPassing);
  formatPlot1D(hTowTPETphiB, towerColour);
  canvas->SaveAs(Form("%s/Towers/TowTPETphiB.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/Towers/TowTPETphiB.png",sDirOut.c_str()));
  hTowTPETphiB->Write();
  
  hTowTPETphiE->Divide(hTowPhiE);
  hTowTPETphiE->Scale(nPassing);
  formatPlot1D(hTowTPETphiE, towerColour);
  canvas->SaveAs(Form("%s/Towers/TowTPETphiE.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/Towers/TowTPETphiE.png",sDirOut.c_str()));
  hTowTPETphiE->Write();
  
  // Plot nTPs vs iPhi for ECAL, EB, EE, HCAL, HB, HE, HF, tower B, tower E, TT28
  formatPlot1D(hECALTPPhi, ecalColour);
  canvas->SaveAs(Form("%s/ECAL/ECALTPPhi.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/ECAL/ECALTPPhi.png",sDirOut.c_str()));
  hECALTPPhi->Write();
  
  formatPlot1D(hECALTPPhiEB, ecalColour);
  canvas->SaveAs(Form("%s/ECAL/ECALTPPhiEB.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/ECAL/ECALTPPhiEB.png",sDirOut.c_str()));
  hECALTPPhiEB->Write();
  
  formatPlot1D(hECALTPPhiEE, ecalColour);
  canvas->SaveAs(Form("%s/ECAL/ECALTPPhiEE.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/ECAL/ECALTPPhiEE.png",sDirOut.c_str()));
  hECALTPPhiEE->Write();
  
  formatPlot1D(hECALTPPhiforTT28, ecalColour);
  canvas->SaveAs(Form("%s/ECAL/ECALTPPhiforTT28.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/ECAL/ECALTPPhiforTT28.png",sDirOut.c_str()));
  hECALTPPhiforTT28->Write();
  
  formatPlot1D(hHCALTPPhi, hcalColour);
  canvas->SaveAs(Form("%s/HCAL/HCALTPPhi.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/HCAL/HCALTPPhi.png",sDirOut.c_str()));
  hHCALTPPhi->Write();
  
  formatPlot1D(hHCALTPPhiHB, hcalColour);
  canvas->SaveAs(Form("%s/HCAL/HCALTPPhiHB.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/HCAL/HCALTPPhiHB.png",sDirOut.c_str()));
  hHCALTPPhiHB->Write();
  
  formatPlot1D(hHCALTPPhiHE, hcalColour);
  canvas->SaveAs(Form("%s/HCAL/HCALTPPhiHE.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/HCAL/HCALTPPhiHE.png",sDirOut.c_str()));
  hHCALTPPhiHE->Write();
  
  formatPlot1D(hHCALTPPhiHF, hcalColour);
  canvas->SaveAs(Form("%s/HCAL/HCALTPPhiHF.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/HCAL/HCALTPPhiHF.png",sDirOut.c_str()));
  hHCALTPPhiHF->Write();
  
  formatPlot1D(hHCALTPPhiforTT28, hcalColour);
  canvas->SaveAs(Form("%s/HCAL/HCALTPPhiforTT28.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/HCAL/HCALTPPhiforTT28.png",sDirOut.c_str()));
  hHCALTPPhiforTT28->Write();
  
  formatPlot1D(hTowPhiB, towerColour);
  canvas->SaveAs(Form("%s/Towers/TowPhiBarrel.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/Towers/TowPhiBarrel.png",sDirOut.c_str()));
  hTowPhiB->Write();
  
  formatPlot1D(hTowPhiE, towerColour);
  canvas->SaveAs(Form("%s/Towers/TowPhiEndcap.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/Towers/TowPhiEndcap.png",sDirOut.c_str()));
  hTowPhiE->Write();
  
  // Plot 2D TP occupancy and ET for ECAL HCAL, TT28
  formatPlot2D(hECALTPEtaPhi);
  canvas->SaveAs(Form("%s/ECAL/ECALTPEtaPhi.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/ECAL/ECALTPEtaPhi.png",sDirOut.c_str()));
  hECALTPEtaPhi->Write();
  
  formatPlot2D(hHCALTPEtaPhi);
  canvas->SaveAs(Form("%s/HCAL/HCALTPEtaPhi.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/HCAL/HCALTPEtaPhi.png",sDirOut.c_str()));
  hHCALTPEtaPhi->Write();
  
  hECALavgTPETEtaPhi->Divide(hECALTPEtaPhi); // Normalise the TP ET by the number of TPs in the eta-phi bin
  formatPlot2D(hECALavgTPETEtaPhi);
  canvas->SaveAs(Form("%s/ECAL/ECALavgTPETEtaPhi.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/ECAL/ECALavgTPETEtaPhi.png",sDirOut.c_str()));
  hECALavgTPETEtaPhi->Write();
  
  hHCALavgTPETEtaPhi->Divide(hHCALTPEtaPhi);
  formatPlot2D(hHCALavgTPETEtaPhi);
  canvas->SaveAs(Form("%s/HCAL/HCALavgTPETEtaPhi.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/HCAL/HCALavgTPETEtaPhi.png",sDirOut.c_str()));
  hHCALavgTPETEtaPhi->Write();
  
  hECALavgTPETforTT28->Divide(hECALTPEtaPhi);   // Divide by hECALTPEtaPhiforTT28 ?? Siddhesh
  formatPlot2D(hECALavgTPETforTT28);
  canvas->SaveAs(Form("%s/ECAL/ECALavgTPETforTT28.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/ECAL/ECALavgTPETforTT28.png",sDirOut.c_str()));
  hECALavgTPETforTT28->Write();
  
  hHCALavgTPETforTT28->Divide(hHCALTPEtaPhi);   // Divide by hECALTPEtaPhiforTT28 ?? Siddhesh
  formatPlot2D(hHCALavgTPETforTT28);
  canvas->SaveAs(Form("%s/HCAL/HCALavgTPETforTT28.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/HCAL/HCALavgTPETforTT28.png",sDirOut.c_str()));
  hHCALavgTPETforTT28->Write();
  
  formatPlot2D(hECALavgTPETperEvEtaPhi);
  canvas->SaveAs(Form("%s/ECAL/ECALavgTPETperEvEtaPhi.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/ECAL/ECALavgTPETperEvEtaPhi.pgn",sDirOut.c_str()));
  hECALavgTPETperEvEtaPhi->Write();
  
  formatPlot2D(hHCALavgTPETperEvEtaPhi);
  canvas->SaveAs(Form("%s/HCAL/HCALavgTPETperEvEtaPhi.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/HCAL/HCALavgTPETperEvEtaPhi.png",sDirOut.c_str()));
  hHCALavgTPETperEvEtaPhi->Write();
  
  // Plot MET Phi 1D
  formatPlot1D(hMetPhi, towerColour);
  canvas->SaveAs(Form("%s/MetPhi.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/MetPhi.png",sDirOut.c_str()));
  hMetPhi->Write();
  
  // Plot MET Phi 2D
  formatPlot2D(hMetPhiEcalHcal);
  canvas->SaveAs(Form("%s/MetPhiEcalHcal.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/MetPhiEcalHcal.png",sDirOut.c_str()));
  hMetPhiEcalHcal->Write();
  
  formatPlot2D(hMetPhiEcalTotal);
  canvas->SaveAs(Form("%s/MetPhiEcalTotal.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/MetPhiEcalTotal.png",sDirOut.c_str()));
  hMetPhiEcalTotal->Write();
  
  formatPlot2D(hMetPhiHcalTotal);
  canvas->SaveAs(Form("%s/MetPhiHcalTotal.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/MetPhiHcalTotal.png",sDirOut.c_str()));
  hMetPhiHcalTotal->Write();
  
  // Plot MET scalar 1D
  canvas->SetLogy(1); // Sets y-axis to log(10)
  hMetScal->Scale(nPassing);
  formatPlot1D(hMetScal, towerColour);
  canvas->SaveAs(Form("%s/MetScal.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/MetScal.png",sDirOut.c_str()));
  canvas->SetLogy(0); // Revert y-axis back to linear scale
  hMetScal->Write();
  
  // Plot MET scalar 2D
  formatPlot2D(hMetScalEcalHcal);
  canvas->SaveAs(Form("%s/MetScalEcalHcal.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/MetScalEcalHcal.png",sDirOut.c_str()));
  hMetScalEcalHcal->Write();
  
  formatPlot2D(hMetScalEcaltotal);
  canvas->SaveAs(Form("%s/MetScalEcaltotal.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/MetScalEcaltotal.png",sDirOut.c_str()));
  hMetScalEcaltotal->Write();
  
  formatPlot2D(hMetScalHcaltotal);
  canvas->SaveAs(Form("%s/MetScalHcaltotal.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/MetScalHcaltotal.png",sDirOut.c_str()));
  hMetScalHcaltotal->Write();
  
  // Plot ETT scalar 1D
  hEttScal->Scale(nPassing);
  formatPlot1D(hEttScal, towerColour);
  canvas->SaveAs(Form("%s/EttScal.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/EttScal.png",sDirOut.c_str()));
  hEttScal->Write();
  
  // Plot ETT scalar 2D
  formatPlot2D(hEttScalEcalHcal);
  canvas->SaveAs(Form("%s/EttScalEcalHcal.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/EttScalEcalHcal.png",sDirOut.c_str()));
  hEttScalEcalHcal->Write();
  
  formatPlot2D(hEttScalEcaltotal);
  canvas->SaveAs(Form("%s/EttScalEcaltotal.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/EttScalEcaltotal.png",sDirOut.c_str()));
  hEttScalEcaltotal->Write();
  
  formatPlot2D(hEttScalHcaltotal);
  canvas->SaveAs(Form("%s/EttScalEHcaltotal.pdf",sDirOut.c_str()));
  canvas->SaveAs(Form("%s/EttScalEHcaltotal.png",sDirOut.c_str()));
  hEttScalHcaltotal->Write();
  
  // Plot histograms for individual events
  for (int x = 0; x < std::min(N_INDIV_EVENTS, nPassing); ++x) {
    std::cout << "histograms for individual events: " << x << std::endl;
    formatPlot2D(hECALTPETEtaPhiIndiv[x]);
    std::string saveNameE  = sDirOut + "/Individual_Events/" + std::to_string(x) + "_ECALTPETEtaPhiIndiv.pdf";
    std::string saveNameE1 = sDirOut + "/Individual_Events/" + std::to_string(x) + "_ECALTPETEtaPhiIndiv.png";
    canvas->SaveAs(saveNameE.c_str());
    canvas->SaveAs(saveNameE1.c_str());
    hECALTPETEtaPhiIndiv[x]->Write();
    
    formatPlot2D(hHCALTPETEtaPhiIndiv[x]);
    std::string saveNameH  = sDirOut + "/Individual_Events/" + std::to_string(x) + "_HCALTPETEtaPhiIndiv.pdf";
    std::string saveNameH1 = sDirOut + "/Individual_Events/" + std::to_string(x) + "_HCALTPETEtaPhiIndiv.png";
    canvas->SaveAs(saveNameH.c_str());
    canvas->SaveAs(saveNameH1.c_str());
    hHCALTPETEtaPhiIndiv[x]->Write();

    std::cout << "saveNameH1: " << saveNameH1 << std::endl;
  }

  std::cout << "\ndoCaloTPStudy.cxx done\n\n" << std::endl;
  return 0;
}
