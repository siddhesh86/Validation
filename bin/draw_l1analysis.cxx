#include "PhysicsTools/Utilities/macros/setTDRStyle.C"

#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TF1.h"
#include "TFile.h"
#include "TLegend.h"
#include "TROOT.h"
#include "TGraphAsymmErrors.h"

#include <map>
#include <string>
#include <vector>

int main()
{
  // include comparisons between HW and data TPs
  bool includeHW = false;
  //int rebinFactor = 1;

  setTDRStyle();
  gROOT->ForceStyle();
  
  // default, then new conditions
  std::vector<std::string> filenames = {"l1analysis_def.root", "l1analysis_new_cond.root"};
  std::vector<std::string> l1Types = {"singleJet", "doubleJet", "tripleJet", "quadJet",
					"htSum", "etSum", "metSum", "metHFSum"};
  
  std::vector<std::string> jetrefTypes = {"RefJet","RefmJet" };
  std::map<std::string, std::vector<std::string> > jetTypes;
  jetTypes["Incl"] = {"JetEt12","JetEt35","JetEt60","JetEt90","JetEt120","JetEt180"};
  jetTypes["HB"]   = {"JetEt12","JetEt35","JetEt60","JetEt90","JetEt120","JetEt180"};
  jetTypes["HE1"]  = {"JetEt12","JetEt35","JetEt60","JetEt90","JetEt120","JetEt180"};
  jetTypes["HE2"]  = {"JetEt12","JetEt35","JetEt60","JetEt90","JetEt120","JetEt180"};
  jetTypes["HE"]   = {"JetEt12","JetEt35","JetEt60","JetEt90","JetEt120","JetEt180"};

  std::vector<std::string> sumrefTypesPF = {"RefMET_PF"};
  std::vector<std::string> sumrefTypesCalo = {"RefMET_Calo"};
  std::vector<std::string> sumTypesPF = {"MET50_PF","MET100_PF","MET120_PF","MET150_PF"};
  std::vector<std::string> sumTypesCalo = {"MET50_Calo","MET100_Calo","MET120_Calo","MET150_Calo"};
  // std::vector<std::string> resTypes = {"hresJet","hResMET"};
  
  std::vector<std::string> resTypes = {"hresJet1","hresJet2","hresJet3","hresJet4","hresJet5","hresJet6","hresJet7","hresJet8","hresJet9","hresJet10",
				       "hresMET1","hresMET2","hresMET3","hresMET4","hresMET5","hresMET6","hresMET7","hresMET8","hresMET9","hresMET10"};

  std::vector<std::string> puRangeNames = {"_lowPU", "_midPU", "_highPU", ""};

  std::vector<std::vector<int> > puRanges = {{30,46}, {47,63}, {64,80}, {0,100}};
    
  std::map<std::string, int> histColor;
  histColor["singleJet"] = histColor["etSum"] = histColor["metSum"] = kRed;
  histColor["doubleJet"] = histColor["htSum"] = histColor["metHFSum"] = kBlue;
  histColor["tripleJet"] = kGreen;
  histColor["quadJet"] = kBlack;

  histColor["hresJet"] = kBlack;
  histColor["hResMET_Calo"] = histColor["hResMET_PF"] = kBlue;

  std::map<std::string, TH1D*> effHists_def;
  std::map<std::string, TH1D*> effHists_new_cond;
  std::map<std::string, TH1D*> effHists_hw;
  std::map<std::string, TH1D*> effHistsRatio;
  
  std::vector<TFile*> files;
  for(auto file : filenames) {
    files.push_back(TFile::Open(file.c_str()));
  }
  for(auto l1Type : l1Types) {
    std::string histName(l1Type);
    std::string histNameHw(histName);
    //  histName += "Effs_emu";
    //  histNameHw += "Effs_hw";

    TH2F* deftemp = dynamic_cast<TH2F*>(files.at(0)->Get(histName.c_str()));
    TH2F* newtemp = dynamic_cast<TH2F*>(files.at(1)->Get(histName.c_str()));
    for (unsigned int irange = 0; irange < puRanges.size(); irange++) {

        std::string custom = puRangeNames[irange]; std::vector<int> bounds = puRanges[irange];

        std::string newName = histName + custom;
        int lowBin = deftemp->GetYaxis()->FindBin(bounds[0]); int hiBin = deftemp->GetYaxis()->FindBin(bounds[1]);

        effHists_def[newName] = deftemp->ProjectionX((newName+"_def").c_str(), lowBin, hiBin, "");
        //effHists_hw[l1Type] = dynamic_cast<TH1F*>(files.at(0)->Get(histNameHw.c_str()));
        effHists_new_cond[newName] = newtemp->ProjectionX((newName+"_new").c_str(), lowBin, hiBin, "");
        
        effHists_def[newName]->Rebin(4);
        // effHists_hw[l1Type]->Rebin(4);
        effHists_new_cond[newName]->Rebin(4);

        effHists_def[newName]->SetLineColor(histColor[l1Type]);
        //effHists_hw[l1Type]->SetLineColor(histColor[l1Type]);
        effHists_new_cond[newName]->SetLineColor(histColor[l1Type]);
        TString name(effHists_new_cond[newName]->GetName());
        name += "_ratio";
        if(includeHW) {
          //effHistsRatio[l1Type] = dynamic_cast<TH1F*>(effHists_def[l1Type]->Clone(name));
          //effHistsRatio[l1Type]->Divide(effHists_hw[l1Type]);
        }
        else {
          effHistsRatio[newName] = dynamic_cast<TH1D*>(effHists_new_cond[newName]->Clone(name));
          effHistsRatio[newName]->Divide(effHists_def[newName]);
        }
        effHistsRatio[newName]->SetMinimum(0.6);    
        effHistsRatio[newName]->SetMaximum(1.4);    
        effHistsRatio[newName]->SetLineWidth(2);    
    }
  }

  for(auto pair : effHists_new_cond) pair.second->SetLineWidth(2);
  for(auto pair : effHists_hw) pair.second->SetLineStyle(kDashed);
  for(auto pair : effHists_def) pair.second->SetLineStyle(kDotted);

  // Efficiencies
  std::map<std::string, TH1D*> jetHists_def;
  std::map<std::string, TH1D*> jetHists_new_cond;
  std::map<std::string, TH1D*> jetrefHists_def;
  std::map<std::string, TH1D*> jetrefHists_new_cond;
  std::map<std::string, TH1D*> metHists_PF_def;
  std::map<std::string, TH1D*> metHists_PF_new_cond;
  std::map<std::string, TH1D*> metHists_Calo_def;
  std::map<std::string, TH1D*> metHists_Calo_new_cond;

  std::map<std::string, TGraphAsymmErrors*> jeteffHists_def;
  std::map<std::string, TGraphAsymmErrors*> jeteffHists_new_cond;
  std::map<std::string, TGraphAsymmErrors*> meteffHists_PF_def;
  std::map<std::string, TGraphAsymmErrors*> meteffHists_PF_new_cond;
  std::map<std::string, TGraphAsymmErrors*> meteffHists_Calo_def;
  std::map<std::string, TGraphAsymmErrors*> meteffHists_Calo_new_cond;

  //-----------------------------------------------------------------------
  // L1 Jet efficiencies
  //-----------------------------------------------------------------------

  int rebinF=5;
  
  std::vector<TCanvas*> tcanvases;
  
  auto mSize = 0.8; auto lWidth = 2.; auto lMargin = 0.20; auto xOff = 1.14; auto yOff = 1.40;
  auto yMinS = -0.9; auto yMaxS = 0.9;
  auto yMinR = -0.05; auto yMaxR = 1.05;
  auto yResTitle = "#sigma#left(#frac{online - offline}{offline}#right)"; auto ySclTitle = "#mu#left(#frac{online - offline}{offline}#right)";
  auto metXTitle = "pfMET [GeV]"; auto jetXTitle = "Offline Jet E_{T} [GeV]";

  gStyle->SetEndErrorSize(0.0);

  for (auto& [region, histos] : jetTypes) {

      std::string refName = "RefmJet_"; refName += region;

      TH2F* deftemp = dynamic_cast<TH2F*>(files.at(0)->Get(refName.c_str()));
      TH2F* newtemp = dynamic_cast<TH2F*>(files.at(1)->Get(refName.c_str()));
      for (unsigned int irange = 0; irange < puRanges.size(); irange++) {

          std::string custom = puRangeNames[irange]; std::vector<int> bounds = puRanges[irange];

          std::string newName = refName + custom;
          int lowBin = deftemp->GetYaxis()->FindBin(bounds[0]); int hiBin = deftemp->GetYaxis()->FindBin(bounds[1]);

          jetrefHists_def[newName] = deftemp->ProjectionX((newName+"_def").c_str(), lowBin, hiBin, ""); jetrefHists_def[newName]->Rebin(rebinF);
          jetrefHists_new_cond[newName] = newtemp->ProjectionX((newName+"_new").c_str(), lowBin, hiBin, ""); jetrefHists_new_cond[newName]->Rebin(rebinF);

          for (auto& jetType : histos) {
               TLegend* jetLegend = new TLegend(0.71, 0.3, 0.91, 0.45);

               std::string histName;
               if (region == "Incl") { histName = jetType; }
               else { histName = std::string(jetType) + std::string("_") + std::string(region); }

               std::string newName2 = histName + custom;
               tcanvases.push_back(new TCanvas);
               tcanvases.back()->SetWindowSize(tcanvases.back()->GetWw(), 1.*tcanvases.back()->GetWh());
               gPad->SetGridx(); gPad->SetGridy();

               TH2F* deftemp2 = dynamic_cast<TH2F*>(files.at(0)->Get(histName.c_str()));
               TH2F* newtemp2 = dynamic_cast<TH2F*>(files.at(1)->Get(histName.c_str()));

               lowBin = deftemp->GetYaxis()->FindBin(bounds[0]); hiBin = deftemp->GetYaxis()->FindBin(bounds[1]);

               jetHists_def[newName2] = deftemp2->ProjectionX((newName2+"_def").c_str(), lowBin, hiBin, ""); jetHists_def[newName2]->Rebin(rebinF);
               jetHists_new_cond[newName2] = newtemp2->ProjectionX((newName2+"_new").c_str(), lowBin, hiBin, ""); jetHists_new_cond[newName2]->Rebin(rebinF);

               TGraphAsymmErrors *Eff1 = new TGraphAsymmErrors();
               TGraphAsymmErrors *Eff2 = new TGraphAsymmErrors();
              
               Eff1->BayesDivide(jetHists_def[newName2],jetrefHists_def[newName]);
               Eff2->BayesDivide(jetHists_new_cond[newName2],jetrefHists_new_cond[newName]);

               jeteffHists_def[newName2] = Eff1;
               jeteffHists_new_cond[newName2] = Eff2;
              
               jeteffHists_def[newName2]->GetYaxis()->SetRangeUser(yMinR, yMaxR);
               jeteffHists_new_cond[newName2]->GetYaxis()->SetRangeUser(yMinR, yMaxR);

               jeteffHists_def[newName2]->SetMarkerColor(kBlack);
               jeteffHists_new_cond[newName2]->SetMarkerColor(kRed);

               jeteffHists_def[newName2]->SetLineColor(kBlack);
               jeteffHists_new_cond[newName2]->SetLineColor(kRed);

               jeteffHists_def[newName2]->SetMarkerSize(mSize);
               jeteffHists_new_cond[newName2]->SetMarkerSize(mSize);
             
               jeteffHists_def[newName2]->SetLineWidth(lWidth);
               jeteffHists_new_cond[newName2]->SetLineWidth(lWidth);

               jetLegend->AddEntry(jeteffHists_def[newName2], "Default", "EP");
               jetLegend->AddEntry(jeteffHists_new_cond[newName2], "New", "EP");

               jeteffHists_def[newName2]->Draw("AP");
               jeteffHists_new_cond[newName2]->Draw("P");
               jeteffHists_def[newName2]->SetTitle("");
               jeteffHists_new_cond[newName2]->SetTitle("");
               jeteffHists_def[newName2]->GetXaxis()->SetTitle(jetXTitle);
               jeteffHists_def[newName2]->GetXaxis()->SetTitleOffset(1.17*jeteffHists_def[newName2]->GetXaxis()->GetTitleOffset());
               jetLegend->Draw("SAME");
               jeteffHists_def[newName2]->GetYaxis()->SetTitle("Efficiency");

               tcanvases.back()->Print(Form("plots/%sjetEffs_emu_%s%s_lin.pdf", jetType.c_str(), region.c_str(), custom.c_str()));
               tcanvases.back()->SetLogx();
               tcanvases.back()->Print(Form("plots/%sjetEffs_emu_%s%s_log.pdf", jetType.c_str(), region.c_str(), custom.c_str()));

          }
      }
  }

  //-----------------------------------------------------------------------
  // L1 ETM efficiencies
  //-----------------------------------------------------------------------

  rebinF=10;

  TH2F* deftemp_PF = dynamic_cast<TH2F*>(files.at(0)->Get("RefMET_PF"));
  TH2F* newtemp_PF = dynamic_cast<TH2F*>(files.at(1)->Get("RefMET_PF"));
  for (unsigned int irange = 0; irange < puRanges.size(); irange++) {

      std::string custom = puRangeNames[irange]; std::vector<int> bounds = puRanges[irange];
      int lowBin = deftemp_PF->GetYaxis()->FindBin(bounds[0]); int hiBin = deftemp_PF->GetYaxis()->FindBin(bounds[1]);

      TH1D* metrefHists_PF_def = deftemp_PF->ProjectionX("RefMET_PF_def", lowBin, hiBin, ""); metrefHists_PF_def->Rebin(rebinF);
      TH1D* metrefHists_PF_new_cond = newtemp_PF->ProjectionX("RefMET_PF_new_cond", lowBin, hiBin, ""); metrefHists_PF_new_cond->Rebin(rebinF);

      for (auto metType : sumTypesPF) {

          TLegend* metLegend = new TLegend(0.67, 0.17, 0.87, 0.33);

          tcanvases.push_back(new TCanvas);
          tcanvases.back()->SetWindowSize(tcanvases.back()->GetWw(), 1.*tcanvases.back()->GetWh());
          gPad->SetGridx(); gPad->SetGridy();
          std::string histName(metType);

          std::string newName = histName + custom;

          TH2F* deftemp2_PF = dynamic_cast<TH2F*>(files.at(0)->Get(histName.c_str()));
          TH2F* newtemp2_PF = dynamic_cast<TH2F*>(files.at(1)->Get(histName.c_str()));

          metHists_PF_def[newName] = deftemp2_PF->ProjectionX((newName+"_PF_def").c_str(), lowBin, hiBin, ""); metHists_PF_def[newName]->Rebin(rebinF);
          metHists_PF_new_cond[newName] = newtemp2_PF->ProjectionX((newName+"_PF_new").c_str(), lowBin, hiBin, ""); metHists_PF_new_cond[newName]->Rebin(rebinF);

          TGraphAsymmErrors *Eff1 = new TGraphAsymmErrors();
          TGraphAsymmErrors *Eff2 = new TGraphAsymmErrors();
          
          Eff1->BayesDivide(metHists_PF_def[newName],metrefHists_PF_def);
          Eff2->BayesDivide(metHists_PF_new_cond[newName],metrefHists_PF_new_cond);

          meteffHists_PF_def[newName] = Eff1;
          meteffHists_PF_new_cond[newName] = Eff2;
          
          meteffHists_PF_def[newName]->GetYaxis()->SetRangeUser(yMinR,yMaxR);
          meteffHists_PF_new_cond[newName]->GetYaxis()->SetRangeUser(yMinR,yMaxR);

          meteffHists_PF_def[newName]->SetMarkerColor(kBlack);
          meteffHists_PF_new_cond[newName]->SetMarkerColor(kRed);

          meteffHists_PF_def[newName]->SetLineColor(kBlack);
          meteffHists_PF_new_cond[newName]->SetLineColor(kRed);

          meteffHists_PF_def[newName]->SetLineWidth(lWidth);
          meteffHists_PF_new_cond[newName]->SetLineWidth(lWidth);

          meteffHists_PF_def[newName]->SetMarkerSize(mSize);
          meteffHists_PF_new_cond[newName]->SetMarkerSize(mSize);

          metLegend->AddEntry(meteffHists_PF_def[newName], "Default", "EP");
          metLegend->AddEntry(meteffHists_PF_new_cond[newName], "New", "EP");
      
          meteffHists_PF_def[newName]->Draw("AP");
          meteffHists_PF_new_cond[newName]->Draw("P");
          meteffHists_PF_def[newName]->SetTitle("");
          meteffHists_PF_new_cond[newName]->SetTitle("");
          meteffHists_PF_def[newName]->GetXaxis()->SetTitle(metXTitle);
          meteffHists_PF_def[newName]->GetXaxis()->SetTitleOffset(1.17*meteffHists_PF_def[newName]->GetXaxis()->GetTitleOffset());
          metLegend->Draw("SAME");

          meteffHists_PF_def[newName]->GetYaxis()->SetTitle("Efficiency");

          tcanvases.back()->Print(Form("plots/%smetEffs%s_emu_lin.pdf", metType.c_str(), custom.c_str()));
          tcanvases.back()->SetLogx();
          tcanvases.back()->Print(Form("plots/%smetEffs%s_emu_log.pdf", metType.c_str(), custom.c_str()));
     }

  }
   
  TH2F* deftemp_Calo = dynamic_cast<TH2F*>(files.at(0)->Get("RefMET_Calo"));
  TH2F* newtemp_Calo = dynamic_cast<TH2F*>(files.at(1)->Get("RefMET_Calo"));
  for (unsigned int irange = 0; irange < puRanges.size(); irange++) {

      std::string custom = puRangeNames[irange]; std::vector<int> bounds = puRanges[irange];
      int lowBin = deftemp_Calo->GetYaxis()->FindBin(bounds[0]); int hiBin = deftemp_Calo->GetYaxis()->FindBin(bounds[1]);

      TH1D* metrefHists_Calo_def = deftemp_Calo->ProjectionX("RefMET_Calo_def", lowBin, hiBin, ""); metrefHists_Calo_def->Rebin(rebinF);
      TH1D* metrefHists_Calo_new_cond = newtemp_Calo->ProjectionX("RefMET_Calo_new_cond", lowBin, hiBin, ""); metrefHists_Calo_new_cond->Rebin(rebinF);

      for (auto metType : sumTypesCalo) {

          TLegend* metLegend = new TLegend(0.67, 0.17, 0.87, 0.33);

          tcanvases.push_back(new TCanvas);
          tcanvases.back()->SetWindowSize(tcanvases.back()->GetWw(), 1.*tcanvases.back()->GetWh());
          gPad->SetGridx(); gPad->SetGridy();
          std::string histName(metType);

          std::string newName = histName + custom;

          TH2F* deftemp2 = dynamic_cast<TH2F*>(files.at(0)->Get(histName.c_str()));
          TH2F* newtemp2 = dynamic_cast<TH2F*>(files.at(1)->Get(histName.c_str()));

          metHists_Calo_def[newName] = deftemp2->ProjectionX((newName+"_Calo_def").c_str(), lowBin, hiBin, ""); metHists_Calo_def[newName]->Rebin(rebinF);
          metHists_Calo_new_cond[newName] = newtemp2->ProjectionX((newName+"_Calo_new").c_str(), lowBin, hiBin, ""); metHists_Calo_new_cond[newName]->Rebin(rebinF);

          TGraphAsymmErrors *Eff1 = new TGraphAsymmErrors();
          TGraphAsymmErrors *Eff2 = new TGraphAsymmErrors();
          
          Eff1->BayesDivide(metHists_Calo_def[newName],metrefHists_Calo_def);
          Eff2->BayesDivide(metHists_Calo_new_cond[newName],metrefHists_Calo_new_cond);

          meteffHists_Calo_def[newName] = Eff1;
          meteffHists_Calo_new_cond[newName] = Eff2;
          
          meteffHists_Calo_def[newName]->GetYaxis()->SetRangeUser(yMinR,yMaxR);
          meteffHists_Calo_new_cond[newName]->GetYaxis()->SetRangeUser(yMinR,yMaxR);

          meteffHists_Calo_def[newName]->SetMarkerColor(kBlack);
          meteffHists_Calo_new_cond[newName]->SetMarkerColor(kRed);

          meteffHists_Calo_def[newName]->SetLineColor(kBlack);
          meteffHists_Calo_new_cond[newName]->SetLineColor(kRed);

          meteffHists_Calo_def[newName]->SetLineWidth(lWidth);
          meteffHists_Calo_new_cond[newName]->SetLineWidth(lWidth);

          meteffHists_Calo_def[newName]->SetMarkerSize(mSize);
          meteffHists_Calo_new_cond[newName]->SetMarkerSize(mSize);

          metLegend->AddEntry(meteffHists_Calo_def[newName], "Default", "EP");
          metLegend->AddEntry(meteffHists_Calo_new_cond[newName], "New", "EP");
      
          meteffHists_Calo_def[newName]->Draw("AP");
          meteffHists_Calo_new_cond[newName]->Draw("P");
          meteffHists_Calo_def[newName]->SetTitle("");
          meteffHists_Calo_new_cond[newName]->SetTitle("");
          meteffHists_Calo_def[newName]->GetXaxis()->SetTitle(metXTitle);
          meteffHists_Calo_def[newName]->GetXaxis()->SetTitleOffset(1.17*meteffHists_Calo_def[newName]->GetXaxis()->GetTitleOffset());
          metLegend->Draw("SAME");

          meteffHists_Calo_def[newName]->GetYaxis()->SetTitle("Efficiency");

          tcanvases.back()->Print(Form("plots/%smetEffs%s_Calo_emu_lin.pdf", metType.c_str(), custom.c_str()));
          tcanvases.back()->SetLogx();
          tcanvases.back()->Print(Form("plots/%smetEffs%s_Calo_emu_log.pdf", metType.c_str(), custom.c_str()));
     }

  }

   //-----------------------------------------------------------------------
   // L1 Jet resolution summary plots
   //-----------------------------------------------------------------------
  
  TF1 *fgaus = new TF1("g1","gaus");//,-2.,2.);
  fgaus->SetRange(-1.,1.);
  
  // // Jet resolution
  std::vector<TCanvas*> mycanvases_1;   std::vector<TCanvas*> mycanvases_2;
  std::vector<TH2D*> resHistos2D_def;   std::vector<TH2D*> resHistos2D_new_cond;
  std::vector<TH1D*> resHistos1D_1_def; std::vector<TH1D*> resHistos1D_1_new_cond;
  std::vector<TH1D*> resHistos1D_2_def; std::vector<TH1D*> resHistos1D_2_new_cond;
  std::vector<std::string> regions = {"Incl", "HE", "HB", "HE2", "HE1"}; 

  for (auto& region : regions) {

      std::string histo2Dname = "hresJet_" + region;
      std::string histo1Dname_1 = histo2Dname + "_1"; std::string histo1Dname_yx_1 = histo2Dname + "_yx_1";
      std::string histo1Dname_2 = histo2Dname + "_2"; std::string histo1Dname_yx_2 = histo2Dname + "_yx_2";

      std::string histo1Dname_1_new_cond = histo1Dname_1 + "_" + region + "_new_cond";
      std::string histo1Dname_2_new_cond = histo1Dname_2 + "_" + region + "_new_cond";

      std::string histo1Dname_1_def = histo1Dname_1 + "_" + region + "_def";
      std::string histo1Dname_2_def = histo1Dname_2 + "_" + region + "_def";

      std::string meanName = "resJet_mean_" + region;
      std::string sigmaName = "resJet_sigma_" + region;

      TH3F* deftemp = dynamic_cast<TH3F*>(files.at(0)->Get(histo2Dname.c_str()));
      TH3F* newtemp = dynamic_cast<TH3F*>(files.at(1)->Get(histo2Dname.c_str()));

      for (unsigned int irange = 0; irange < puRanges.size(); irange++) {

          std::string custom = puRangeNames[irange]; std::vector<int> bounds = puRanges[irange];
          int lowBin = deftemp->GetZaxis()->FindBin(bounds[0]); int hiBin = deftemp->GetZaxis()->FindBin(bounds[1]);

          deftemp->GetZaxis()->SetRange(lowBin, hiBin); 
          newtemp->GetZaxis()->SetRange(lowBin, hiBin);

          resHistos2D_def.push_back(dynamic_cast<TH2D*>(deftemp->Project3D("yx"))); resHistos2D_def.back()->RebinX(10);
          resHistos2D_def.back()->FitSlicesY(fgaus);

          resHistos1D_1_def.push_back((TH1D*)gDirectory->Get((histo1Dname_yx_1).c_str())); resHistos1D_1_def.back()->SetName((histo1Dname_1_def+custom).c_str());
          resHistos1D_2_def.push_back((TH1D*)gDirectory->Get((histo1Dname_yx_2).c_str())); resHistos1D_2_def.back()->SetName((histo1Dname_2_def+custom).c_str());

          gROOT->cd();

          resHistos2D_new_cond.push_back(dynamic_cast<TH2D*>(newtemp->Project3D("yx"))); resHistos2D_new_cond.back()->RebinX(10);
          resHistos2D_new_cond.back()->FitSlicesY(fgaus);
          resHistos1D_1_new_cond.push_back((TH1D*)gDirectory->Get((histo1Dname_yx_1).c_str())); resHistos1D_1_new_cond.back()->SetName((histo1Dname_1_new_cond+custom).c_str());
          resHistos1D_2_new_cond.push_back((TH1D*)gDirectory->Get((histo1Dname_yx_2).c_str())); resHistos1D_2_new_cond.back()->SetName((histo1Dname_2_new_cond+custom).c_str());
          gROOT->cd();
  
          mycanvases_1.push_back(new TCanvas);
          mycanvases_1.back()->SetWindowSize(mycanvases_1.back()->GetWw(), 1.*mycanvases_1.back()->GetWh());

          gPad->SetGridx(); gPad->SetGridy();
          gPad->SetLeftMargin(lMargin);
   
          resHistos1D_1_def.back()->Draw("");
          resHistos1D_1_def.back()->SetTitle("");
          resHistos1D_1_def.back()->GetXaxis()->SetNdivisions(5,10,0);
          resHistos1D_1_def.back()->GetXaxis()->SetTitle(jetXTitle);
          resHistos1D_1_def.back()->GetXaxis()->SetTitleOffset(xOff*resHistos1D_1_def.back()->GetXaxis()->GetTitleOffset());
          resHistos1D_1_def.back()->GetYaxis()->SetTitleOffset(yOff*resHistos1D_1_def.back()->GetYaxis()->GetTitleOffset());

          resHistos1D_1_def.back()->GetYaxis()->SetTitle(ySclTitle);
          resHistos1D_1_def.back()->GetYaxis()->SetRangeUser(yMinS, yMaxS);
          resHistos1D_1_def.back()->SetMarkerSize(mSize);
          resHistos1D_1_def.back()->SetLineWidth(lWidth);
          resHistos1D_1_def.back()->SetMarkerStyle(24);

          resHistos1D_1_new_cond.back()->Draw("same");
          resHistos1D_1_new_cond.back()->SetLineColor(2);
          resHistos1D_1_new_cond.back()->SetLineWidth(lWidth);
          resHistos1D_1_new_cond.back()->SetMarkerColor(2);
          resHistos1D_1_new_cond.back()->SetMarkerSize(mSize);
          resHistos1D_1_new_cond.back()->SetMarkerStyle(20);

          TLegend* resLegend1 = new TLegend(0.23, 0.17, 0.43, 0.32);
          resLegend1->AddEntry(resHistos1D_1_def.back(), "Default", "EP");
          resLegend1->AddEntry(resHistos1D_1_new_cond.back(), "New", "EP");
          resLegend1->Draw("SAME");

          mycanvases_1.back()->Print(Form("plots/%s_emu.pdf", (meanName+custom).c_str()));

          mycanvases_2.push_back(new TCanvas);
          mycanvases_2.back()->SetWindowSize(mycanvases_2.back()->GetWw(), 1.*mycanvases_2.back()->GetWh());

          gPad->SetGridx(); gPad->SetGridy();
          gPad->SetLeftMargin(lMargin);
 
          resHistos1D_2_def.back()->Draw("");
          resHistos1D_2_def.back()->SetTitle("");
          resHistos1D_2_def.back()->GetXaxis()->SetTitle(jetXTitle);
          resHistos1D_2_def.back()->GetXaxis()->SetNdivisions(5,10,0);
          resHistos1D_2_def.back()->GetXaxis()->SetTitleOffset(xOff*resHistos1D_2_def.back()->GetXaxis()->GetTitleOffset());
          resHistos1D_2_def.back()->GetYaxis()->SetTitleOffset(yOff*resHistos1D_2_def.back()->GetYaxis()->GetTitleOffset());

          resHistos1D_2_def.back()->GetYaxis()->SetTitle(yResTitle);
          resHistos1D_2_def.back()->GetYaxis()->SetRangeUser(yMinR,yMaxR);
          resHistos1D_2_def.back()->SetMarkerSize(mSize);
          resHistos1D_2_def.back()->SetLineWidth(lWidth);
          resHistos1D_2_def.back()->SetMarkerStyle(24);

          resHistos1D_2_new_cond.back()->Draw("same");
          resHistos1D_2_new_cond.back()->SetLineColor(2);
          resHistos1D_2_new_cond.back()->SetLineWidth(lWidth);
          resHistos1D_2_new_cond.back()->SetMarkerColor(2);
          resHistos1D_2_new_cond.back()->SetMarkerSize(mSize);
          resHistos1D_2_new_cond.back()->SetMarkerStyle(20);

          TLegend* resLegend2 = new TLegend(0.33, 0.75, 0.53, 0.90);
          resLegend2->AddEntry(resHistos1D_2_def.back(), "Default", "EP");
          resLegend2->AddEntry(resHistos1D_2_new_cond.back(), "New", "EP");
          resLegend2->Draw("SAME");

          mycanvases_2.back()->Print(Form("plots/%s_emu.pdf", (sigmaName+custom).c_str()));
      }
  }

  //-----------------------------------------------------------------------
  // L1 ETM resolution summary plots
  //-----------------------------------------------------------------------
  
  TF1 *fgaus0 = new TF1("g0","gaus");//,-2.,2.);
  fgaus0->SetRange(-1.,3.);
  
  TH3F* dtemp_Calo = dynamic_cast<TH3F*>(files.at(0)->Get("hResMET_Calo"));
  TH3F* ntemp_Calo = dynamic_cast<TH3F*>(files.at(1)->Get("hResMET_Calo"));
  for (unsigned int irange = 0; irange < puRanges.size(); irange++) {

      std::string custom = puRangeNames[irange]; std::vector<int> bounds = puRanges[irange];
      int lowBin = dtemp_Calo->GetZaxis()->FindBin(bounds[0]); int hiBin = dtemp_Calo->GetZaxis()->FindBin(bounds[1]);

      dtemp_Calo->GetZaxis()->SetRange(lowBin, hiBin); 
      ntemp_Calo->GetZaxis()->SetRange(lowBin, hiBin);

      std::vector<TCanvas*> mycanvases2;
      TH2D *resMET_Calo_def = dynamic_cast<TH2D*>(dtemp_Calo->Project3D("yx"));resMET_Calo_def->RebinX(10);
      files.at(0)->cd();
      resMET_Calo_def->FitSlicesY(fgaus0);//,1,80);//,20);

      TH2D *resMET_Calo_new_cond = dynamic_cast<TH2D*>(ntemp_Calo->Project3D("yx"));resMET_Calo_new_cond->RebinX(10);
      files.at(1)->cd();
      resMET_Calo_new_cond->FitSlicesY(fgaus0);//,1,80);//,20);
      
      mycanvases2.push_back(new TCanvas);
      mycanvases2.back()->SetWindowSize(mycanvases2.back()->GetWw(), 1.*mycanvases2.back()->GetWh());

      gPad->SetGridx(); gPad->SetGridy();
      gPad->SetLeftMargin(lMargin);

      TH1D *resMET_Calo_def_1 = (TH1D*)files.at(0)->Get("hResMET_Calo_yx_1");
      resMET_Calo_def_1->Draw("");
      resMET_Calo_def_1->SetTitle("");

      resMET_Calo_def_1->GetXaxis()->SetTitle(metXTitle);
      resMET_Calo_def_1->GetXaxis()->SetNdivisions(5,10,0);

      resMET_Calo_def_1->GetXaxis()->SetTitleOffset(xOff*resMET_Calo_def_1->GetXaxis()->GetTitleOffset());
      resMET_Calo_def_1->GetYaxis()->SetTitleOffset(yOff*resMET_Calo_def_1->GetYaxis()->GetTitleOffset());


      resMET_Calo_def_1->GetYaxis()->SetTitle(ySclTitle);
      resMET_Calo_def_1->GetYaxis()->SetRangeUser(-2.25,2.25);
      resMET_Calo_def_1->SetMarkerSize(mSize);
      resMET_Calo_def_1->SetLineWidth(lWidth);
      resMET_Calo_def_1->SetMarkerStyle(24);
      resMET_Calo_def_1->SetMarkerColor(1);
      
      TH1D *resMET_Calo_new_cond_1 = (TH1D*)files.at(1)->Get("hResMET_Calo_yx_1");
      resMET_Calo_new_cond_1->Draw("same");
      resMET_Calo_new_cond_1->SetLineColor(2);
      resMET_Calo_new_cond_1->SetLineWidth(lWidth);
      resMET_Calo_new_cond_1->SetMarkerColor(2);
      resMET_Calo_new_cond_1->SetMarkerSize(mSize);
      resMET_Calo_new_cond_1->SetMarkerStyle(20);
      
      TLegend* resLegend1 = new TLegend(0.23, 0.17, 0.43, 0.32);
      resLegend1->AddEntry(resMET_Calo_def_1, "Default", "EP");
      resLegend1->AddEntry(resMET_Calo_new_cond_1, "New", "EP");
      resLegend1->Draw("SAME");

      mycanvases2.back()->Print(Form("plots/%s_emu.pdf", ("resMET_Calo_mean"+custom).c_str()));

      mycanvases2.push_back(new TCanvas);
      mycanvases2.back()->SetWindowSize(mycanvases2.back()->GetWw(), 1.*mycanvases2.back()->GetWh());

      gPad->SetGridx(); gPad->SetGridy();
      gPad->SetLeftMargin(lMargin);

      TH1D *resMET_Calo_def_2 = (TH1D*)files.at(0)->Get("hResMET_Calo_yx_2");
      resMET_Calo_def_2->Draw("");
      resMET_Calo_def_2->SetTitle("");

      resMET_Calo_def_2->GetXaxis()->SetTitle(metXTitle);
      resMET_Calo_def_2->GetXaxis()->SetNdivisions(5,10,0);
      resMET_Calo_def_2->GetXaxis()->SetTitleOffset(xOff*resMET_Calo_def_2->GetXaxis()->GetTitleOffset());
      resMET_Calo_def_2->GetYaxis()->SetTitleOffset(yOff*resMET_Calo_def_2->GetYaxis()->GetTitleOffset());


      resMET_Calo_def_2->GetYaxis()->SetTitle(yResTitle);
      resMET_Calo_def_2->GetYaxis()->SetRangeUser(yMinR, yMaxR);
      resMET_Calo_def_2->SetMarkerSize(mSize);
      resMET_Calo_def_2->SetLineWidth(lWidth);
      resMET_Calo_def_2->SetMarkerStyle(24);
      resMET_Calo_def_2->SetMarkerColor(1);

      TH1D *resMET_Calo_new_cond_2 = (TH1D*)files.at(1)->Get("hResMET_Calo_yx_2");
      resMET_Calo_new_cond_2->Draw("same");
      resMET_Calo_new_cond_2->SetLineColor(2);
      resMET_Calo_new_cond_2->SetMarkerColor(2);
      resMET_Calo_new_cond_2->SetMarkerSize(mSize);
      resMET_Calo_new_cond_2->SetLineWidth(lWidth);
      resMET_Calo_new_cond_2->SetMarkerStyle(20);
      
      TLegend* resLegend2 = new TLegend(0.23, 0.17, 0.43, 0.32);
      resLegend2->AddEntry(resMET_Calo_def_2, "Default", "EP");
      resLegend2->AddEntry(resMET_Calo_new_cond_2, "New", "EP");
      resLegend2->Draw("SAME");

      mycanvases2.back()->Print(Form("plots/%s_emu.pdf", ("resMET_Calo_sigma"+custom).c_str()));
  }
  
  TH3F* dtemp_PF = dynamic_cast<TH3F*>(files.at(0)->Get("hResMET_PF"));
  TH3F* ntemp_PF = dynamic_cast<TH3F*>(files.at(1)->Get("hResMET_PF"));
  for (unsigned int irange = 0; irange < puRanges.size(); irange++) {

      std::string custom = puRangeNames[irange]; std::vector<int> bounds = puRanges[irange];
      int lowBin = dtemp_PF->GetZaxis()->FindBin(bounds[0]); int hiBin = dtemp_PF->GetZaxis()->FindBin(bounds[1]);

      dtemp_PF->GetZaxis()->SetRange(lowBin, hiBin); 
      ntemp_PF->GetZaxis()->SetRange(lowBin, hiBin);

      std::vector<TCanvas*> mycanvases2;
      TH2D *resMET_PF_def = dynamic_cast<TH2D*>(dtemp_PF->Project3D("yx"));resMET_PF_def->RebinX(10);
      files.at(0)->cd();
      resMET_PF_def->FitSlicesY(fgaus0);//,1,80);//,20);

      TH2D *resMET_PF_new_cond = dynamic_cast<TH2D*>(ntemp_PF->Project3D("yx"));resMET_PF_new_cond->RebinX(10);
      files.at(1)->cd();
      resMET_PF_new_cond->FitSlicesY(fgaus0);//,1,80);//,20);
      
      mycanvases2.push_back(new TCanvas);
      mycanvases2.back()->SetWindowSize(mycanvases2.back()->GetWw(), 1.*mycanvases2.back()->GetWh());

      gPad->SetGridx(); gPad->SetGridy();
      gPad->SetLeftMargin(lMargin);

      TH1D *resMET_PF_def_1 = (TH1D*)files.at(0)->Get("hResMET_PF_yx_1");
      resMET_PF_def_1->Draw("");
      resMET_PF_def_1->SetTitle("");

      resMET_PF_def_1->GetXaxis()->SetTitle(metXTitle);
      resMET_PF_def_1->GetXaxis()->SetNdivisions(5,10,0);

      resMET_PF_def_1->GetXaxis()->SetTitleOffset(xOff*resMET_PF_def_1->GetXaxis()->GetTitleOffset());
      resMET_PF_def_1->GetYaxis()->SetTitleOffset(yOff*resMET_PF_def_1->GetYaxis()->GetTitleOffset());


      resMET_PF_def_1->GetYaxis()->SetTitle(ySclTitle);
      resMET_PF_def_1->GetYaxis()->SetRangeUser(-2.25,2.25);
      resMET_PF_def_1->SetMarkerSize(mSize);
      resMET_PF_def_1->SetLineWidth(lWidth);
      resMET_PF_def_1->SetMarkerStyle(24);
      resMET_PF_def_1->SetMarkerColor(1);
      
      TH1D *resMET_PF_new_cond_1 = (TH1D*)files.at(1)->Get("hResMET_PF_yx_1");
      resMET_PF_new_cond_1->Draw("same");
      resMET_PF_new_cond_1->SetLineColor(2);
      resMET_PF_new_cond_1->SetLineWidth(lWidth);
      resMET_PF_new_cond_1->SetMarkerColor(2);
      resMET_PF_new_cond_1->SetMarkerSize(mSize);
      resMET_PF_new_cond_1->SetMarkerStyle(20);
      
      TLegend* resLegend1 = new TLegend(0.23, 0.17, 0.43, 0.32);
      resLegend1->AddEntry(resMET_PF_def_1, "Default", "EP");
      resLegend1->AddEntry(resMET_PF_new_cond_1, "New", "EP");
      resLegend1->Draw("SAME");

      mycanvases2.back()->Print(Form("plots/%s_emu.pdf", ("resMET_PF_mean"+custom).c_str()));

      mycanvases2.push_back(new TCanvas);
      mycanvases2.back()->SetWindowSize(mycanvases2.back()->GetWw(), 1.*mycanvases2.back()->GetWh());

      gPad->SetGridx(); gPad->SetGridy();
      gPad->SetLeftMargin(lMargin);

      TH1D *resMET_PF_def_2 = (TH1D*)files.at(0)->Get("hResMET_PF_yx_2");
      resMET_PF_def_2->Draw("");
      resMET_PF_def_2->SetTitle("");

      resMET_PF_def_2->GetXaxis()->SetTitle(metXTitle);
      resMET_PF_def_2->GetXaxis()->SetNdivisions(5,10,0);
      resMET_PF_def_2->GetXaxis()->SetTitleOffset(xOff*resMET_PF_def_2->GetXaxis()->GetTitleOffset());
      resMET_PF_def_2->GetYaxis()->SetTitleOffset(yOff*resMET_PF_def_2->GetYaxis()->GetTitleOffset());


      resMET_PF_def_2->GetYaxis()->SetTitle(yResTitle);
      resMET_PF_def_2->GetYaxis()->SetRangeUser(yMinR, yMaxR);
      resMET_PF_def_2->SetMarkerSize(mSize);
      resMET_PF_def_2->SetLineWidth(lWidth);
      resMET_PF_def_2->SetMarkerStyle(24);
      resMET_PF_def_2->SetMarkerColor(1);

      TH1D *resMET_PF_new_cond_2 = (TH1D*)files.at(1)->Get("hResMET_PF_yx_2");
      resMET_PF_new_cond_2->Draw("same");
      resMET_PF_new_cond_2->SetLineColor(2);
      resMET_PF_new_cond_2->SetMarkerColor(2);
      resMET_PF_new_cond_2->SetMarkerSize(mSize);
      resMET_PF_new_cond_2->SetLineWidth(lWidth);
      resMET_PF_new_cond_2->SetMarkerStyle(20);
      
      TLegend* resLegend2 = new TLegend(0.23, 0.17, 0.43, 0.32);
      resLegend2->AddEntry(resMET_PF_def_2, "Default", "EP");
      resLegend2->AddEntry(resMET_PF_new_cond_2, "New", "EP");
      resLegend2->Draw("SAME");

      mycanvases2.back()->Print(Form("plots/%s_emu.pdf", ("resMET_PF_sigma"+custom).c_str()));
  }

  //-----------------------------------------------------------------------
  // Resolution in ET bins plots
  //-----------------------------------------------------------------------
  
  //std::map<std::string, TH1F*> resHists_def;
  //std::map<std::string, TH1F*> resHists_new_cond;
  //std::map<std::string, TH1F*> resHistsRatio;
  //
  //std::vector<TCanvas*> rcanvases;
  //for(auto rType : resTypes) {
  //  std::string histName(rType);
  //  // std::string histNameHw(histName);
  //  //    histName += "Effs_emu";
  //  //  histNameHw += "Effs_hw";
  //  rcanvases.push_back(new TCanvas);
  //  rcanvases.back()->SetWindowSize(rcanvases.back()->GetWw(), 1.*rcanvases.back()->GetWh());
  //  
  //  resHists_def[rType] = dynamic_cast<TH1F*>(files.at(0)->Get(histName.c_str()));
  //  //  resHists_hw[rType] = dynamic_cast<TH1F*>(files.at(0)->Get(histNameHw.c_str()));
  //  resHists_new_cond[rType] = dynamic_cast<TH1F*>(files.at(1)->Get(histName.c_str()));
  //  
  //  resHists_def[rType]->Rebin(rebinFactor);
  //  //  resHists_hw[rType]->Rebin(rebinFactor);
  //  resHists_new_cond[rType]->Rebin(rebinFactor);

  //  resHists_def[rType]->SetLineColor(kBlack); //histColor[rType]);
  //  //   resHists_hw[rType]->SetLineColor(histColor[rType]);
  //  resHists_new_cond[rType]->SetLineColor(kRed); //histColor[rType]);

  //  resHists_def[rType]->Fit("gaus","R+","hist",-2.,2.); //Draw("hist");
  //  resHists_def[rType]->GetYaxis()->SetRangeUser(0.,1.4*resHists_def[rType]->GetMaximum());
  //  TF1 *f1 = resHists_def[rType]->GetFunction("gaus"); //->SetLineColor(kBlack);
  //  f1->SetLineColor(kBlack);
  //  f1->Draw("SAME");
  //  
  //  resHists_new_cond[rType]->Fit("gaus","R+","histsame",-2.,2.);
  //  TF1 *f2 = resHists_new_cond[rType]->GetFunction("gaus"); //->SetLineColor(kBlack);
  //  f2->SetLineColor(kRed);
  //  f2->Draw("SAME");

  //  rcanvases.back()->Print(Form("plots/%sbin_emu.pdf", rType.c_str()));
  //  
  //}
  //for(auto pair : resHists_new_cond) pair.second->SetLineWidth(2);
  //// for(auto pair : resHists_hw) pair.second->SetLineStyle(kDashed);
  //for(auto pair : resHists_def) pair.second->SetLineStyle(kDotted);
  
  //-----------------------------------------------------------------------
  // Standard L1 quantities
  //-----------------------------------------------------------------------
  
  //std::vector<std::string> jetPlots = {"singleJet", "doubleJet", "tripleJet", "quadJet"};
  //std::vector<std::string> scalarSumPlots = {"etSum", "htSum"};
  //std::vector<std::string> vectorSumPlots = {"metSum", "metHFSum"};

  //
  //std::vector<TCanvas*> canvases;
  //std::vector<TPad*> pad1;
  //std::vector<TPad*> pad2;
  //std::map<std::string, std::vector<std::string> > plots;
  //plots["jet"] = jetPlots;
  //plots["scalarSum"] = scalarSumPlots;
  //plots["vectorSum"] = vectorSumPlots;
  //// plots["resolution"] = resTypes;

  //for(auto iplot : plots) {

  //  canvases.push_back(new TCanvas);
  //  canvases.back()->SetWindowSize(canvases.back()->GetWw(), 1.*canvases.back()->GetWh());
  //  pad1.push_back(new TPad("pad1", "pad1", 0, 0.3, 1, 1));
  //  pad1.back()->SetLogy();
  //  pad1.back()->SetGrid();
  //  pad1.back()->Draw();
  //  pad2.push_back(new TPad("pad2", "pad2", 0, 0, 1, 0.3));
  //  pad2.back()->SetGrid();
  //  pad2.back()->Draw();
  //  
  //  pad1.back()->cd();

  //  effHists_def[iplot.second.front()]->Draw("hist");
  //  effHists_def[iplot.second.front()]->GetYaxis()->SetRangeUser(0.1,20.4*effHists_def[iplot.second.front()]->GetMaximum());
  //    
  //  TLegend *leg = new TLegend(0.55, 0.9 - 0.1*iplot.second.size(), 0.95, 0.93);
  //  for(auto hist : iplot.second) {
  //    effHists_def[hist]->Draw("hist same");
  //    if(includeHW) effHists_hw[hist]->Draw("hist same");
  //    effHists_new_cond[hist]->Draw("hist same");
  //    TString name(effHists_def[hist]->GetName());
  //    TString nameHw(effHists_hw[hist]->GetName());
  //    leg->AddEntry(effHists_def[hist], name + " (current)", "L");
  //    if(includeHW) leg->AddEntry(effHists_hw[hist], name + " (hw)", "L");
  //    leg->AddEntry(effHists_new_cond[hist], name + " (new)", "L"); 
  //  }
  //  leg->SetBorderSize(0);
  //  leg->Draw();
  //  
  //  pad2.back()->cd();
  //  effHistsRatio[iplot.second.front()]->Draw("hist");
  //  // if(includeHW) effHistsRatio[iplot.second.front()]->GetYaxis()->SetTitle("Current/HW");
  //  //else
  //  effHistsRatio[iplot.second.front()]->GetYaxis()->SetTitle("New/Current");
  //  for(auto hist : iplot.second) {
  //    effHistsRatio[hist]->Draw("hist same");
  //  }

  //  //if(includeHW) canvases.back()->Print(Form("plots/%s_hw.pdf", iplot.first.c_str()));
  //  //else
  //  canvases.back()->SetLogx();
  //  canvases.back()->Print(Form("plots/%s_emu.pdf", iplot.first.c_str()));
  //}

 

  return 0;
}
