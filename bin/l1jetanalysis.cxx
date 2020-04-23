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
#include "L1Trigger/L1TNtuples/interface/L1AnalysisRecoMetFilterDataFormat.h"

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

void jetanalysis(bool newConditions, const std::string& inputFileDirectory){
  
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
    if (recoOn) {
        recoTree->Add(inputFile.c_str());
        metfilterTree->Add(inputFile.c_str());
    }

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

    L1Analysis::L1AnalysisRecoJetDataFormat    *jet_ = new L1Analysis::L1AnalysisRecoJetDataFormat();
    recoTree->SetBranchAddress("Jet", &jet_);
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

    std::string axR = ";Threshold E_{T} (GeV);rate (Hz)";
    std::string axD = ";E_{T} (GeV);events/bin";
    std::string metD = ";MET (GeV);events/bin";
    
    //make histos
    // TH1F *NCenJets = new TH1F("NCenJets","n",20,-0.5,19.5);

    std::vector<double> jetThresholds; std::vector<double> metThresholds;
    if (!newConditions) {
        jetThresholds = {12.0, 35.0, 60.0, 90.0, 120.0, 180.0};
        metThresholds = {50.0, 100.0, 120.0, 180.0};
    }
    else if (inputFileDirectory.find("PFA1p")) {
        jetThresholds = {12.0, 35.0, 60.0, 90.0, 120.0, 180.0};
        metThresholds = {50.0, 100.0, 120.0, 180.0};

        //jetThresholds = {12.0, 24.0, 50.0, 76.0, 101.0, 173.0};
        //metThresholds = {36.0, 72.0, 85.0, 106.0};
    }
    else if (inputFileDirectory.find("PFA3p")) {
        jetThresholds = {12.0, 35.0, 60.0, 90.0, 120.0, 180.0};
        metThresholds = {50.0, 100.0, 120.0, 180.0};

        //jetThresholds = {12.0, 30.0, 54.0, 81.0, 107.0, 174.0};
        //metThresholds = {41.0, 82.0, 96.0, 121.0};
    }
    // Jets
    TH1F *refJetET_Incl = new TH1F("RefJet_Incl", "all Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi);
    TH1F *refmJetET_Incl = new TH1F("RefmJet_Incl", "all matched Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi);

    TH1F *refJetET_HB = new TH1F("RefJet_HB", "all Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi);
    TH1F *refmJetET_HB = new TH1F("RefmJet_HB", "all matched Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi);

    TH1F *refJetET_HE1 = new TH1F("RefJet_HE1", "all Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi);
    TH1F *refmJetET_HE1 = new TH1F("RefmJet_HE1", "all matched Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi);
    
    TH1F *refJetET_HE2 = new TH1F("RefJet_HE2", "all Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi);
    TH1F *refmJetET_HE2 = new TH1F("RefmJet_HE2", "all matched Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi);

    TH1F *refJetET_HE = new TH1F("RefJet_HE", "all Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi);
    TH1F *refmJetET_HE = new TH1F("RefmJet_HE", "all matched Jet1 E_{T} (GeV)",nJetBins, jetLo, jetHi);

    TH1F *jetET12   = new TH1F( "JetEt12" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *jetET35   = new TH1F( "JetEt35" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *jetET60   = new TH1F( "JetEt60" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *jetET90   = new TH1F( "JetEt90" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *jetET120  = new TH1F( "JetEt120" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *jetET180  = new TH1F( "JetEt180" , axD.c_str(),nJetBins, jetLo, jetHi);

    TH1F *jetET12_HB   = new TH1F( "JetEt12_HB" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *jetET35_HB   = new TH1F( "JetEt35_HB" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *jetET60_HB   = new TH1F( "JetEt60_HB" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *jetET90_HB   = new TH1F( "JetEt90_HB" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *jetET120_HB  = new TH1F( "JetEt120_HB" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *jetET180_HB  = new TH1F( "JetEt180_HB" , axD.c_str(),nJetBins, jetLo, jetHi);

    TH1F *jetET12_HE1   = new TH1F( "JetEt12_HE1" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *jetET35_HE1   = new TH1F( "JetEt35_HE1" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *jetET60_HE1   = new TH1F( "JetEt60_HE1" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *jetET90_HE1   = new TH1F( "JetEt90_HE1" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *jetET120_HE1  = new TH1F( "JetEt120_HE1" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *jetET180_HE1  = new TH1F( "JetEt180_HE1" , axD.c_str(),nJetBins, jetLo, jetHi);

    TH1F *jetET12_HE2   = new TH1F( "JetEt12_HE2" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *jetET35_HE2   = new TH1F( "JetEt35_HE2" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *jetET60_HE2   = new TH1F( "JetEt60_HE2" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *jetET90_HE2   = new TH1F( "JetEt90_HE2" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *jetET120_HE2  = new TH1F( "JetEt120_HE2" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *jetET180_HE2  = new TH1F( "JetEt180_HE2" , axD.c_str(),nJetBins, jetLo, jetHi);

    TH1F *jetET12_HE   = new TH1F( "JetEt12_HE" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *jetET35_HE   = new TH1F( "JetEt35_HE" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *jetET60_HE   = new TH1F( "JetEt60_HE" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *jetET90_HE   = new TH1F( "JetEt90_HE" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *jetET120_HE  = new TH1F( "JetEt120_HE" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *jetET180_HE  = new TH1F( "JetEt180_HE" , axD.c_str(),nJetBins, jetLo, jetHi);
   
    TH1F *l1jetET1 = new TH1F( "singleJet" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *l1jetET2 = new TH1F( "doubleJet" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *l1jetET3 = new TH1F( "tripleJet" , axD.c_str(),nJetBins, jetLo, jetHi);
    TH1F *l1jetET4 = new TH1F( "quadJet" , axD.c_str(),nJetBins, jetLo, jetHi);

    // and Sums
    TH1F *refMET  = new TH1F("RefMET",metD.c_str(), nMetSumBins, metSumLo, metSumHi);
    TH1F *MET_150U = new TH1F("MET150",metD.c_str(), nMetSumBins, metSumLo, metSumHi);
    TH1F *MET_120U  = new TH1F("MET120",metD.c_str(), nMetSumBins, metSumLo, metSumHi);
    TH1F *MET_100U  = new TH1F("MET100",metD.c_str(), nMetSumBins, metSumLo, metSumHi);
    TH1F *MET_50U  = new TH1F("MET50",metD.c_str(), nMetSumBins, metSumLo, metSumHi);

    TH1F *l1ET = new TH1F("etSum","L1 sumET (GeV)",nEtSumBins,etSumLo,etSumHi);
    TH1F *l1MET = new TH1F("metSum","L1 MET (GeV)",nMetSumBins,metSumLo,metSumHi);
    TH1F *l1METHF = new TH1F("metHFSum","L1 METHF (GeV)",nMetSumBins,metSumLo,metSumHi);
    TH1F *l1HT = new TH1F("htSum","L1 HT (GeV)",nHtSumBins,htSumLo,htSumHi);
    TH1F *l1MHT = new TH1F("mhtSum","L1 MHT (GeV)",nMhtSumBins,mhtSumLo,mhtSumHi);

    // resolution histograms
    TH2F *hresJet_Incl = new TH2F("hresJet_Incl","",nJetBins, jetLo, jetHi,100,-5,5);
    TH2F *hresMET = new TH2F("hResMET","",nMetSumBins,metSumLo,metSumHi,100,-5,5);

    TH2F *hresJet_HB = new TH2F("hresJet_HB","",nJetBins, jetLo, jetHi,100,-5,5);
    TH2F *hresJet_HE1 = new TH2F("hresJet_HE1","",nJetBins, jetLo, jetHi,100,-5,5);
    TH2F *hresJet_HE2 = new TH2F("hresJet_HE2","",nJetBins, jetLo, jetHi,100,-5,5);
    TH2F *hresJet_HE = new TH2F("hresJet_HE","",nJetBins, jetLo, jetHi,100,-5,5);
   
    TH1F *h_resMET1 = new TH1F("hresMET1","",100,-5,5) ;
    TH1F *h_resMET2 = new TH1F("hresMET2","",100,-5,5) ;
    TH1F *h_resMET3 = new TH1F("hresMET3","",100,-5,5) ;
    TH1F *h_resMET4 = new TH1F("hresMET4","",100,-5,5) ;
    TH1F *h_resMET5 = new TH1F("hresMET5","",100,-5,5) ;
    TH1F *h_resMET6 = new TH1F("hresMET6","",100,-5,5) ;
    TH1F *h_resMET7 = new TH1F("hresMET7","",100,-5,5) ;
    TH1F *h_resMET8 = new TH1F("hresMET8","",100,-5,5) ;
    TH1F *h_resMET9 = new TH1F("hresMET9","",100,-5,5) ;
    TH1F *h_resMET10 = new TH1F("hresMET10","",100,-5,5) ;
    
    TH1F *h_resJet1 = new TH1F("hresJet1","",100,-5,5) ;
    TH1F *h_resJet2 = new TH1F("hresJet2","",100,-5,5) ;
    TH1F *h_resJet3 = new TH1F("hresJet3","",100,-5,5) ;
    TH1F *h_resJet4 = new TH1F("hresJet4","",100,-5,5) ;
    TH1F *h_resJet5 = new TH1F("hresJet5","",100,-5,5) ;
    TH1F *h_resJet6 = new TH1F("hresJet6","",100,-5,5) ;
    TH1F *h_resJet7 = new TH1F("hresJet7","",100,-5,5) ;
    TH1F *h_resJet8 = new TH1F("hresJet8","",100,-5,5) ;
    TH1F *h_resJet9 = new TH1F("hresJet9","",100,-5,5) ;
    TH1F *h_resJet10 = new TH1F("hresJet10","",100,-5,5) ;
      
    // hcal/ecal TPs
    TH1F* hcalTP_emu = new TH1F("hcalTP_emu", ";TP E_{T}; # Entries", nTpBins, tpLo, tpHi);
    TH1F* ecalTP_emu = new TH1F("ecalTP_emu", ";TP E_{T}; # Entries", nTpBins, tpLo, tpHi);

    // TH1F* hcalTP_hw = new TH1F("hcalTP_hw", ";TP E_{T}; # Entries", nTpBins, tpLo, tpHi);
    // TH1F* ecalTP_hw = new TH1F("ecalTP_hw", ";TP E_{T}; # Entries", nTpBins, tpLo, tpHi);

    /////////////////////////////////
    // loop through all the entries//
    /////////////////////////////////
    for (Long64_t jentry=0; jentry<nentries; jentry++){
        if((jentry%10000)==0) std::cout << "Done " << jentry  << " events of " << nentries << std::endl;

        //lumi break clause
        eventTree->GetEntry(jentry);
        //skip the corresponding event
        if (!isGoodLumiSection(event_->lumi)) continue;
        goodLumiEventCount++;

        //do routine for L1 emulator quantites
        if (emuOn){

            treeL1TPemu->GetEntry(jentry);
            double tpEt(0.);
            
            for(int i=0; i < l1TPemu_->nHCALTP; i++){
                tpEt = l1TPemu_->hcalTPet[i];
                hcalTP_emu->Fill(tpEt);
            }
            for(int i=0; i < l1TPemu_->nECALTP; i++){
                tpEt = l1TPemu_->ecalTPet[i];
                ecalTP_emu->Fill(tpEt);
            }

            treeL1emu->GetEntry(jentry);
            // get jetEt*, egEt*, tauEt, htSum, mhtSum, etSum, metSum
            // ALL EMU OBJECTS HAVE BX=0...
            double jetEt_1(0.);
            if (l1emu_->nJets>0) {
                jetEt_1 = l1emu_->jetEt[0];
                l1jetET1->Fill(jetEt_1);
            }
            double jetEt_2(0.);
            if (l1emu_->nJets>1) {
                jetEt_2 = l1emu_->jetEt[1];
                l1jetET2->Fill(jetEt_2);
            }
            double jetEt_3(0.);
            if (l1emu_->nJets>2) {
                jetEt_3 = l1emu_->jetEt[2];
                l1jetET3->Fill(jetEt_3);
            }
            double jetEt_4(0.);
            if (l1emu_->nJets>3) {
                jetEt_4 = l1emu_->jetEt[3];
                l1jetET4->Fill(jetEt_4);
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

            l1ET->Fill(etSum);
            l1MET->Fill(metSum);
            l1METHF->Fill(metHFSum);
            l1HT->Fill(htSum);
            l1MHT->Fill(mhtSum);
        
            // stuff for efficiencies and resolution
            if (recoOn) {
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
                float rMET = met_->pfMetNoMu;
                refMET->Fill( rMET );
                if( metSum > metThresholds[0]) { MET_50U->Fill(rMET);}
                if( metSum > metThresholds[1]) { MET_100U->Fill(rMET);}
                if( metSum > metThresholds[2]) { MET_120U->Fill(rMET);}
                if( metSum > metThresholds[3]) { MET_150U->Fill(rMET);}

                // met resolution
                float resMET = (metSum-rMET)/rMET;
                hresMET->Fill(rMET, resMET);
                
                if (rMET<20.) h_resMET1->Fill(resMET);
                if (rMET>=20. && rMET<40.) h_resMET2->Fill(resMET);
                if (rMET>=40. && rMET<60.) h_resMET3->Fill(resMET);
                if (rMET>=60. && rMET<80.) h_resMET4->Fill(resMET);
                if (rMET>=80. && rMET<100.) h_resMET5->Fill(resMET);
                if (rMET>=100. && rMET<120.) h_resMET6->Fill(resMET);
                if (rMET>=120. && rMET<140.) h_resMET7->Fill(resMET);
                if (rMET>=140. && rMET<180.) h_resMET8->Fill(resMET);
                if (rMET>=180. && rMET<250.) h_resMET9->Fill(resMET);
                if (rMET>=250. && rMET<500.) h_resMET10->Fill(resMET);
                
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

                    double jetEta = abs(jet_->eta[jetIdx]);

                    refJetET_Incl->Fill(jet_->etCorr[jetIdx]);
                    if      (jetEta >= 0 && jetEta < 1.392)     { refJetET_HB->Fill(jet_->etCorr[jetIdx]); }
                    else if (jetEta >= 1.392 && jetEta < 1.74) { refJetET_HE1->Fill(jet_->etCorr[jetIdx]); refJetET_HE->Fill(jet_->etCorr[jetIdx]);} 
                    else if (jetEta >= 1.74 && jetEta < 3.0) { refJetET_HE2->Fill(jet_->etCorr[jetIdx]); refJetET_HE->Fill(jet_->etCorr[jetIdx]);}
                    
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
                        
                        float resJet=(l1emu_->jetEt[l1jetIdx]-jet_->etCorr[jetIdx])/jet_->etCorr[jetIdx];
                        refmJetET_Incl->Fill(jet_->etCorr[jetIdx]);
                        hresJet_Incl->Fill(jet_->etCorr[jetIdx],resJet);

                        if (l1emu_->jetEt[l1jetIdx]>jetThresholds[0]) jetET12->Fill(jet_->etCorr[jetIdx]);
                        if (l1emu_->jetEt[l1jetIdx]>jetThresholds[1]) jetET35->Fill(jet_->etCorr[jetIdx]);
                        if (l1emu_->jetEt[l1jetIdx]>jetThresholds[2]) jetET60->Fill(jet_->etCorr[jetIdx]);
                        if (l1emu_->jetEt[l1jetIdx]>jetThresholds[3]) jetET90->Fill(jet_->etCorr[jetIdx]);
                        if (l1emu_->jetEt[l1jetIdx]>jetThresholds[4]) jetET120->Fill(jet_->etCorr[jetIdx]);
                        if (l1emu_->jetEt[l1jetIdx]>jetThresholds[5]) jetET180->Fill(jet_->etCorr[jetIdx]);

                        if (jetEta >= 0 && jetEta < 1.392) {
                            refmJetET_HB->Fill(jet_->etCorr[jetIdx]);
                            hresJet_HB->Fill(jet_->etCorr[jetIdx],resJet);

                            if (l1emu_->jetEt[l1jetIdx]>jetThresholds[0]) jetET12_HB->Fill(jet_->etCorr[jetIdx]);
                            if (l1emu_->jetEt[l1jetIdx]>jetThresholds[1]) jetET35_HB->Fill(jet_->etCorr[jetIdx]);
                            if (l1emu_->jetEt[l1jetIdx]>jetThresholds[2]) jetET60_HB->Fill(jet_->etCorr[jetIdx]);
                            if (l1emu_->jetEt[l1jetIdx]>jetThresholds[3]) jetET90_HB->Fill(jet_->etCorr[jetIdx]);
                            if (l1emu_->jetEt[l1jetIdx]>jetThresholds[4]) jetET120_HB->Fill(jet_->etCorr[jetIdx]);
                            if (l1emu_->jetEt[l1jetIdx]>jetThresholds[5]) jetET180_HB->Fill(jet_->etCorr[jetIdx]);
                        } else if (jetEta >= 1.392 && jetEta < 1.74) {
                            refmJetET_HE1->Fill(jet_->etCorr[jetIdx]);
                            hresJet_HE1->Fill(jet_->etCorr[jetIdx],resJet);

                            refmJetET_HE->Fill(jet_->etCorr[jetIdx]);
                            hresJet_HE->Fill(jet_->etCorr[jetIdx],resJet);

                            if (l1emu_->jetEt[l1jetIdx]>jetThresholds[0]) {jetET12_HE1->Fill(jet_->etCorr[jetIdx]);  jetET12_HE->Fill(jet_->etCorr[jetIdx]);}
                            if (l1emu_->jetEt[l1jetIdx]>jetThresholds[1]) {jetET35_HE1->Fill(jet_->etCorr[jetIdx]);  jetET35_HE->Fill(jet_->etCorr[jetIdx]);}
                            if (l1emu_->jetEt[l1jetIdx]>jetThresholds[2]) {jetET60_HE1->Fill(jet_->etCorr[jetIdx]);  jetET60_HE->Fill(jet_->etCorr[jetIdx]);}
                            if (l1emu_->jetEt[l1jetIdx]>jetThresholds[3]) {jetET90_HE1->Fill(jet_->etCorr[jetIdx]);  jetET90_HE->Fill(jet_->etCorr[jetIdx]);}
                            if (l1emu_->jetEt[l1jetIdx]>jetThresholds[4]) {jetET120_HE1->Fill(jet_->etCorr[jetIdx]); jetET120_HE->Fill(jet_->etCorr[jetIdx]);}
                            if (l1emu_->jetEt[l1jetIdx]>jetThresholds[5]) {jetET180_HE1->Fill(jet_->etCorr[jetIdx]); jetET180_HE->Fill(jet_->etCorr[jetIdx]);}

                        } else if (jetEta >= 1.74 && jetEta < 3.0) {
                            refmJetET_HE2->Fill(jet_->etCorr[jetIdx]);
                            hresJet_HE2->Fill(jet_->etCorr[jetIdx],resJet);

                            refmJetET_HE->Fill(jet_->etCorr[jetIdx]);
                            hresJet_HE->Fill(jet_->etCorr[jetIdx],resJet);

                            if (l1emu_->jetEt[l1jetIdx]>jetThresholds[0]) {jetET12_HE2->Fill(jet_->etCorr[jetIdx]);  jetET12_HE->Fill(jet_->etCorr[jetIdx]);}
                            if (l1emu_->jetEt[l1jetIdx]>jetThresholds[1]) {jetET35_HE2->Fill(jet_->etCorr[jetIdx]);  jetET35_HE->Fill(jet_->etCorr[jetIdx]);}
                            if (l1emu_->jetEt[l1jetIdx]>jetThresholds[2]) {jetET60_HE2->Fill(jet_->etCorr[jetIdx]);  jetET60_HE->Fill(jet_->etCorr[jetIdx]);}
                            if (l1emu_->jetEt[l1jetIdx]>jetThresholds[3]) {jetET90_HE2->Fill(jet_->etCorr[jetIdx]);  jetET90_HE->Fill(jet_->etCorr[jetIdx]);}
                            if (l1emu_->jetEt[l1jetIdx]>jetThresholds[4]) {jetET120_HE2->Fill(jet_->etCorr[jetIdx]); jetET120_HE->Fill(jet_->etCorr[jetIdx]);}
                            if (l1emu_->jetEt[l1jetIdx]>jetThresholds[5]) {jetET180_HE2->Fill(jet_->etCorr[jetIdx]); jetET180_HE->Fill(jet_->etCorr[jetIdx]);}
                        }

                        if (jet_->etCorr[jetIdx]<50.) h_resJet1->Fill(resJet);
                        if (jet_->etCorr[jetIdx]>=50. && jet_->etCorr[jetIdx]<100.) h_resJet2->Fill(resJet);
                        if (jet_->etCorr[jetIdx]>=100. && jet_->etCorr[jetIdx]<150.) h_resJet3->Fill(resJet);
                        if (jet_->etCorr[jetIdx]>=150. && jet_->etCorr[jetIdx]<200.) h_resJet4->Fill(resJet);
                        if (jet_->etCorr[jetIdx]>=200. && jet_->etCorr[jetIdx]<250.) h_resJet5->Fill(resJet);
                        if (jet_->etCorr[jetIdx]>=250. && jet_->etCorr[jetIdx]<300.) h_resJet6->Fill(resJet);
                        if (jet_->etCorr[jetIdx]>=300. && jet_->etCorr[jetIdx]<350.) h_resJet7->Fill(resJet);
                        if (jet_->etCorr[jetIdx]>=350. && jet_->etCorr[jetIdx]<400.) h_resJet8->Fill(resJet);
                        if (jet_->etCorr[jetIdx]>=400. && jet_->etCorr[jetIdx]<450.) h_resJet9->Fill(resJet);
                        if (jet_->etCorr[jetIdx]>=450. && jet_->etCorr[jetIdx]<500.) h_resJet10->Fill(resJet);
                      
                    } // close 'found matched l1jet'
                } // close 'at least one offline jet'
        
            }// closes if 'recoOn' is true

        }// closes if 'emuOn' is true
      
    }// closes loop through events

    //  TFile g( outputFilename.c_str() , "new");
    kk->cd();

    if (emuOn){
        // ecal/hcal TPs
        hcalTP_emu->Write();
        ecalTP_emu->Write();
        // l1 quantities
        l1jetET1->Write(); l1jetET2->Write(); l1jetET3->Write(); l1jetET4->Write();
        l1ET->Write(); l1MET->Write(); l1METHF->Write(); l1HT->Write(); l1MHT->Write();
        // efficiencies
        refJetET_Incl->Write(); refmJetET_Incl->Write();
        refJetET_HB->Write(); refmJetET_HB->Write();
        refJetET_HE1->Write(); refmJetET_HE1->Write();
        refJetET_HE2->Write(); refmJetET_HE2->Write();
        refJetET_HE->Write(); refmJetET_HE->Write();
        jetET12->Write(); jetET35->Write(); jetET60->Write(); jetET90->Write(); jetET120->Write(); jetET180->Write();
        jetET12_HB->Write(); jetET35_HB->Write(); jetET60_HB->Write(); jetET90_HB->Write(); jetET120_HB->Write(); jetET180_HB->Write();
        jetET12_HE1->Write(); jetET35_HE1->Write(); jetET60_HE1->Write(); jetET90_HE1->Write(); jetET120_HE1->Write(); jetET180_HE1->Write();
        jetET12_HE2->Write(); jetET35_HE2->Write(); jetET60_HE2->Write(); jetET90_HE2->Write(); jetET120_HE2->Write(); jetET180_HE2->Write();
        jetET12_HE->Write(); jetET35_HE->Write(); jetET60_HE->Write(); jetET90_HE->Write(); jetET120_HE->Write(); jetET180_HE->Write();

        refMET->Write();
        MET_50U->Write(); MET_100U->Write(); MET_120U->Write(); MET_150U->Write();
        // resolutions
        hresMET->Write(); hresJet_Incl->Write();
        hresJet_HB->Write();
        hresJet_HE1->Write();
        hresJet_HE2->Write();
        hresJet_HE->Write();
        h_resMET1->Write();h_resMET2->Write();h_resMET3->Write();h_resMET4->Write();h_resMET5->Write();h_resMET6->Write();h_resMET7->Write();h_resMET8->Write();h_resMET9->Write();h_resMET10->Write();
        h_resJet1->Write();h_resJet2->Write();h_resJet3->Write();h_resJet4->Write();h_resJet5->Write();h_resJet6->Write();h_resJet7->Write();h_resJet8->Write();h_resJet9->Write();h_resJet10->Write();
    }

}//closes the function 'rates'
