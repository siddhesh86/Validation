#include "PhysicsTools/Utilities/macros/setTDRStyle.C"

#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
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
  int rebinFactor = 1;

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

  std::vector<std::string> sumrefTypes = {"RefMET"};
  std::vector<std::string> sumTypes = {"MET50","MET100","MET120","MET150"};
  // std::vector<std::string> resTypes = {"hresJet","hResMET"};
  
  std::vector<std::string> resTypes = {"hresJet1","hresJet2","hresJet3","hresJet4","hresJet5","hresJet6","hresJet7","hresJet8","hresJet9","hresJet10",
				       "hresMET1","hresMET2","hresMET3","hresMET4","hresMET5","hresMET6","hresMET7","hresMET8","hresMET9","hresMET10"};
    
  std::map<std::string, int> histColor;
  histColor["singleJet"] = histColor["etSum"] = histColor["metSum"] = kRed;
  histColor["doubleJet"] = histColor["htSum"] = histColor["metHFSum"] = kBlue;
  histColor["tripleJet"] = kGreen;
  histColor["quadJet"] = kBlack;

  histColor["hresJet"] = kBlack;
  histColor["hResMET"] = kBlue;

  std::map<std::string, TH1F*> effHists_def;
  std::map<std::string, TH1F*> effHists_new_cond;
  std::map<std::string, TH1F*> effHists_hw;
  std::map<std::string, TH1F*> effHistsRatio;
  
  std::vector<TFile*> files;
  for(auto file : filenames) {
    files.push_back(TFile::Open(file.c_str()));
  }
  for(auto l1Type : l1Types) {
    std::string histName(l1Type);
    std::string histNameHw(histName);
    //    histName += "Effs_emu";
    //  histNameHw += "Effs_hw";
    effHists_def[l1Type] = dynamic_cast<TH1F*>(files.at(0)->Get(histName.c_str()));
    effHists_hw[l1Type] = dynamic_cast<TH1F*>(files.at(0)->Get(histNameHw.c_str()));
    effHists_new_cond[l1Type] = dynamic_cast<TH1F*>(files.at(1)->Get(histName.c_str()));
    
    effHists_def[l1Type]->Rebin(4);
    // effHists_hw[l1Type]->Rebin(4);
    effHists_new_cond[l1Type]->Rebin(4);

    effHists_def[l1Type]->SetLineColor(histColor[l1Type]);
    effHists_hw[l1Type]->SetLineColor(histColor[l1Type]);
    effHists_new_cond[l1Type]->SetLineColor(histColor[l1Type]);
    TString name(effHists_new_cond[l1Type]->GetName());
    name += "_ratio";
    if(includeHW) {
      effHistsRatio[l1Type] = dynamic_cast<TH1F*>(effHists_def[l1Type]->Clone(name));
      effHistsRatio[l1Type]->Divide(effHists_hw[l1Type]);
    }
    else {
      effHistsRatio[l1Type] = dynamic_cast<TH1F*>(effHists_new_cond[l1Type]->Clone(name));
      effHistsRatio[l1Type]->Divide(effHists_def[l1Type]);
    }
    effHistsRatio[l1Type]->SetMinimum(0.6);    
    effHistsRatio[l1Type]->SetMaximum(1.4);    
    effHistsRatio[l1Type]->SetLineWidth(2);    
  }
  for(auto pair : effHists_new_cond) pair.second->SetLineWidth(2);
  for(auto pair : effHists_hw) pair.second->SetLineStyle(kDashed);
  for(auto pair : effHists_def) pair.second->SetLineStyle(kDotted);

  // Efficiencies
  std::map<std::string, TH1F*> jetHists_def;
  std::map<std::string, TH1F*> jetHists_new_cond;
  std::map<std::string, TH1F*> jetrefHists_def;
  std::map<std::string, TH1F*> jetrefHists_new_cond;
  std::map<std::string, TH1F*> metHists_def;
  std::map<std::string, TH1F*> metHists_new_cond;

  std::map<std::string, TGraphAsymmErrors*> jeteffHists_def;
  std::map<std::string, TGraphAsymmErrors*> jeteffHists_new_cond;
  std::map<std::string, TGraphAsymmErrors*> meteffHists_def;
  std::map<std::string, TGraphAsymmErrors*> meteffHists_new_cond;


  //-----------------------------------------------------------------------
  // L1 Jet efficiencies
  //-----------------------------------------------------------------------

  int rebinF=5;
  
  std::vector<TCanvas*> tcanvases;
  
  auto mSize = 0.8; auto lWidth = 2.; auto lMargin = 0.20; auto xOff = 1.14; auto yOff = 1.40;
  auto yMinS = -0.9; auto yMaxS = 0.9;
  auto yMinR = -0.05; auto yMaxR = 1.05;
  auto yResTitle = "#sigma#left(#frac{online - offline}{offline}#right)"; auto ySclTitle = "#mu#left(#frac{online - offline}{offline}#right)";
  auto metXTitle = "Offline MET [GeV]"; auto jetXTitle = "Offline Jet E_{T} [GeV]";

  gStyle->SetEndErrorSize(0.0);

  for (auto& [region, histos] : jetTypes) {

      std::string refName = "RefmJet_"; refName += region;

      jetrefHists_def[region]=dynamic_cast<TH1F*>(files.at(0)->Get(refName.c_str()));jetrefHists_def[region]->Rebin(rebinF);
      jetrefHists_new_cond[region]=dynamic_cast<TH1F*>(files.at(1)->Get(refName.c_str()));jetrefHists_new_cond[region]->Rebin(rebinF);

      for (auto& jetType : histos) {
          TLegend* jetLegend = new TLegend(0.71, 0.3, 0.91, 0.45);

           std::string histName;
           if (region == "Incl") { histName = jetType; }
           else { histName = std::string(jetType) + std::string("_") + std::string(region); }

           tcanvases.push_back(new TCanvas);
           tcanvases.back()->SetLogx();
           tcanvases.back()->SetWindowSize(tcanvases.back()->GetWw(), 1.*tcanvases.back()->GetWh());
           gPad->SetGridx(); gPad->SetGridy();

           jetHists_def[histName] = dynamic_cast<TH1F*>(files.at(0)->Get(histName.c_str()));
           jetHists_new_cond[histName] = dynamic_cast<TH1F*>(files.at(1)->Get(histName.c_str()));

           jetHists_def[histName]->Rebin(rebinF);
           jetHists_new_cond[histName]->Rebin(rebinF);

           TGraphAsymmErrors *Eff1 = new TGraphAsymmErrors();
           TGraphAsymmErrors *Eff2 = new TGraphAsymmErrors();
           
           Eff1->BayesDivide(jetHists_def[histName],jetrefHists_def[region]);
           Eff2->BayesDivide(jetHists_new_cond[histName],jetrefHists_new_cond[region]);

           jeteffHists_def[histName] = Eff1;
           jeteffHists_new_cond[histName] = Eff2;
           
           jeteffHists_def[histName]->GetYaxis()->SetRangeUser(yMinR, yMaxR);
           jeteffHists_new_cond[histName]->GetYaxis()->SetRangeUser(yMinR, yMaxR);

           jeteffHists_def[histName]->SetMarkerColor(kBlack);
           jeteffHists_new_cond[histName]->SetMarkerColor(kRed);

           jeteffHists_def[histName]->SetLineColor(kBlack);
           jeteffHists_new_cond[histName]->SetLineColor(kRed);

           jeteffHists_def[histName]->SetMarkerSize(mSize);
           jeteffHists_new_cond[histName]->SetMarkerSize(mSize);
           
           jeteffHists_def[histName]->SetLineWidth(lWidth);
           jeteffHists_new_cond[histName]->SetLineWidth(lWidth);

           jetLegend->AddEntry(jeteffHists_def[histName], "Default", "EP");
           jetLegend->AddEntry(jeteffHists_new_cond[histName], "New", "EP");

           jeteffHists_def[histName]->Draw("AP");
           jeteffHists_new_cond[histName]->Draw("P");
           jeteffHists_def[histName]->SetTitle("");
           jeteffHists_new_cond[histName]->SetTitle("");
           jeteffHists_def[histName]->GetXaxis()->SetTitle(jetXTitle);
           jeteffHists_def[histName]->GetXaxis()->SetTitleOffset(1.17*jeteffHists_def[histName]->GetXaxis()->GetTitleOffset());
            jetLegend->Draw("SAME");
           jeteffHists_def[histName]->GetYaxis()->SetTitle("Efficiency");

           tcanvases.back()->Print(Form("plots/%sjetEffs_emu_%s.pdf", jetType.c_str(), region.c_str()));
      }
    
  }

  //-----------------------------------------------------------------------
  // L1 ETM efficiencies
  //-----------------------------------------------------------------------

  rebinF=10;
  TH1F *metrefHists_def=dynamic_cast<TH1F*>(files.at(0)->Get("RefMET"));metrefHists_def->Rebin(rebinF);
  TH1F *metrefHists_new_cond=dynamic_cast<TH1F*>(files.at(1)->Get("RefMET"));metrefHists_new_cond->Rebin(rebinF);
  
   for (auto metType : sumTypes) {

    TLegend* metLegend = new TLegend(0.17, 0.77, 0.37, 0.93);

    tcanvases.push_back(new TCanvas);
    tcanvases.back()->SetLogx();
    tcanvases.back()->SetWindowSize(tcanvases.back()->GetWw(), 1.*tcanvases.back()->GetWh());
    gPad->SetGridx(); gPad->SetGridy();
    std::string histName(metType);

    metHists_def[metType] = dynamic_cast<TH1F*>(files.at(0)->Get(histName.c_str()));
    metHists_new_cond[metType] = dynamic_cast<TH1F*>(files.at(1)->Get(histName.c_str()));

    metHists_def[metType]->Rebin(rebinF);
    metHists_new_cond[metType]->Rebin(rebinF);

    TGraphAsymmErrors *Eff1 = new TGraphAsymmErrors();
    TGraphAsymmErrors *Eff2 = new TGraphAsymmErrors();
    
    Eff1->BayesDivide(metHists_def[metType],metrefHists_def);
    Eff2->BayesDivide(metHists_new_cond[metType],metrefHists_new_cond);

    meteffHists_def[metType] = Eff1;
    meteffHists_new_cond[metType] = Eff2;
    
    meteffHists_def[metType]->GetYaxis()->SetRangeUser(yMinR,yMaxR);
    meteffHists_new_cond[metType]->GetYaxis()->SetRangeUser(yMinR,yMaxR);

    meteffHists_def[metType]->SetMarkerColor(kBlack);
    meteffHists_new_cond[metType]->SetMarkerColor(kRed);

    meteffHists_def[metType]->SetLineColor(kBlack);
    meteffHists_new_cond[metType]->SetLineColor(kRed);

    meteffHists_def[metType]->SetLineWidth(lWidth);
    meteffHists_new_cond[metType]->SetLineWidth(lWidth);

    meteffHists_def[metType]->SetMarkerSize(mSize);
    meteffHists_new_cond[metType]->SetMarkerSize(mSize);

    metLegend->AddEntry(meteffHists_def[metType], "Default", "EP");
    metLegend->AddEntry(meteffHists_new_cond[metType], "New", "EP");
   
    meteffHists_def[metType]->Draw("AP");
    meteffHists_new_cond[metType]->Draw("P");
    meteffHists_def[metType]->SetTitle("");
    meteffHists_new_cond[metType]->SetTitle("");
    meteffHists_def[metType]->GetXaxis()->SetTitle(metXTitle);
    meteffHists_def[metType]->GetXaxis()->SetTitleOffset(1.17*meteffHists_def[metType]->GetXaxis()->GetTitleOffset());
    metLegend->Draw("SAME");

    meteffHists_def[metType]->GetYaxis()->SetTitle("Efficiency");

    tcanvases.back()->Print(Form("plots/%smetEffs_emu.pdf", metType.c_str()));
    

  }
   
   //-----------------------------------------------------------------------
   // L1 Jet resolution summary plots
   //-----------------------------------------------------------------------
  
  TF1 *fgaus = new TF1("g1","gaus");//,-2.,2.);
  fgaus->SetRange(-1.,1.);
  
  // // Jet resolution
  std::vector<TCanvas*> mycanvases_1;   std::vector<TCanvas*> mycanvases_2;
  std::vector<TH2F*> resHistos2D_def;   std::vector<TH2F*> resHistos2D_new_cond;
  std::vector<TH1D*> resHistos1D_1_def; std::vector<TH1D*> resHistos1D_1_new_cond;
  std::vector<TH1D*> resHistos1D_2_def; std::vector<TH1D*> resHistos1D_2_new_cond;
  std::vector<std::string> regions = {"Incl", "HE", "HB", "HE2", "HE1"}; 

  for (auto& region : regions) {

      std::string histo2Dname = "hresJet_" + region;
      std::string histo1Dname_1 = histo2Dname + "_1";
      std::string histo1Dname_2 = histo2Dname + "_2";

      std::string histo1Dname_1_new_cond = histo1Dname_1 + "_" + region + "_new_cond";
      std::string histo1Dname_2_new_cond = histo1Dname_2 + "_" + region + "_new_cond";

      std::string histo1Dname_1_def = histo1Dname_1 + "_" + region + "_def";
      std::string histo1Dname_2_def = histo1Dname_2 + "_" + region + "_def";

      std::string meanName = "resJet_mean_" + region;
      std::string sigmaName = "resJet_sigma_" + region;

      resHistos2D_def.push_back(dynamic_cast<TH2F*>(files.at(0)->Get(histo2Dname.c_str()))); resHistos2D_def.back()->RebinX(10);
      resHistos2D_def.back()->FitSlicesY(fgaus);
      resHistos1D_1_def.push_back((TH1D*)gDirectory->Get(histo1Dname_1.c_str())); resHistos1D_1_def.back()->SetName(histo1Dname_1_def.c_str());
      resHistos1D_2_def.push_back((TH1D*)gDirectory->Get(histo1Dname_2.c_str())); resHistos1D_2_def.back()->SetName(histo1Dname_2_def.c_str());
      gROOT->cd();

      resHistos2D_new_cond.push_back(dynamic_cast<TH2F*>(files.at(1)->Get(histo2Dname.c_str()))); resHistos2D_new_cond.back()->RebinX(10);
      resHistos2D_new_cond.back()->FitSlicesY(fgaus);
      resHistos1D_1_new_cond.push_back((TH1D*)gDirectory->Get(histo1Dname_1.c_str())); resHistos1D_1_new_cond.back()->SetName(histo1Dname_1_new_cond.c_str());
      resHistos1D_2_new_cond.push_back((TH1D*)gDirectory->Get(histo1Dname_2.c_str())); resHistos1D_2_new_cond.back()->SetName(histo1Dname_2_new_cond.c_str());
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

      mycanvases_1.back()->Print(Form("plots/%s_emu.pdf", meanName.c_str()));

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

      mycanvases_2.back()->Print(Form("plots/%s_emu.pdf", sigmaName.c_str()));

  }

  //-----------------------------------------------------------------------
  // L1 ETM resolution summary plots
  //-----------------------------------------------------------------------
  
  TF1 *fgaus0 = new TF1("g0","gaus");//,-2.,2.);
  fgaus0->SetRange(-1.,3.);
  
  std::vector<TCanvas*> mycanvases2;
  TH2F *resMET_def = dynamic_cast<TH2F*>(files.at(0)->Get("hResMET"));resMET_def->RebinX(10);
  files.at(0)->cd();
  resMET_def->FitSlicesY(fgaus0);//,1,80);//,20);

  TH2F *resMET_new_cond = dynamic_cast<TH2F*>(files.at(1)->Get("hResMET"));resMET_new_cond->RebinX(10);
  files.at(1)->cd();
  resMET_new_cond->FitSlicesY(fgaus0);//,1,80);//,20);
  
  mycanvases2.push_back(new TCanvas);
  mycanvases2.back()->SetWindowSize(mycanvases2.back()->GetWw(), 1.*mycanvases2.back()->GetWh());

  gPad->SetGridx(); gPad->SetGridy();
  gPad->SetLeftMargin(lMargin);

  TH1D *resMET_def_1 = (TH1D*)files.at(0)->Get("hResMET_1");
  resMET_def_1->Draw("");
  resMET_def_1->SetTitle("");

  resMET_def_1->GetXaxis()->SetTitle(metXTitle);
  resMET_def_1->GetXaxis()->SetNdivisions(5,10,0);

  resMET_def_1->GetXaxis()->SetTitleOffset(xOff*resMET_def_1->GetXaxis()->GetTitleOffset());
  resMET_def_1->GetYaxis()->SetTitleOffset(yOff*resMET_def_1->GetYaxis()->GetTitleOffset());


  resMET_def_1->GetYaxis()->SetTitle(ySclTitle);
  resMET_def_1->GetYaxis()->SetRangeUser(-2.25,2.25);
  resMET_def_1->SetMarkerSize(mSize);
  resMET_def_1->SetLineWidth(lWidth);
  resMET_def_1->SetMarkerStyle(24);
  resMET_def_1->SetMarkerColor(1);
  
  TH1D *resMET_new_cond_1 = (TH1D*)files.at(1)->Get("hResMET_1");
  resMET_new_cond_1->Draw("same");
  resMET_new_cond_1->SetLineColor(2);
  resMET_new_cond_1->SetLineWidth(lWidth);
  resMET_new_cond_1->SetMarkerColor(2);
  resMET_new_cond_1->SetMarkerSize(mSize);
  resMET_new_cond_1->SetMarkerStyle(20);
  
  TLegend* resLegend1 = new TLegend(0.23, 0.17, 0.43, 0.32);
  resLegend1->AddEntry(resMET_def_1, "Default", "EP");
  resLegend1->AddEntry(resMET_new_cond_1, "New", "EP");
  resLegend1->Draw("SAME");

  mycanvases2.back()->Print(Form("plots/%s_emu.pdf", "resMET_mean"));

  mycanvases2.push_back(new TCanvas);
  mycanvases2.back()->SetWindowSize(mycanvases2.back()->GetWw(), 1.*mycanvases2.back()->GetWh());

  gPad->SetGridx(); gPad->SetGridy();
  gPad->SetLeftMargin(lMargin);

  TH1D *resMET_def_2 = (TH1D*)files.at(0)->Get("hResMET_2");
  resMET_def_2->Draw("");
  resMET_def_2->SetTitle("");

  resMET_def_2->GetXaxis()->SetTitle(metXTitle);
  resMET_def_2->GetXaxis()->SetNdivisions(5,10,0);
  resMET_def_2->GetXaxis()->SetTitleOffset(xOff*resMET_def_2->GetXaxis()->GetTitleOffset());
  resMET_def_2->GetYaxis()->SetTitleOffset(yOff*resMET_def_2->GetYaxis()->GetTitleOffset());


  resMET_def_2->GetYaxis()->SetTitle(yResTitle);
  resMET_def_2->GetYaxis()->SetRangeUser(yMinR, yMaxR);
  resMET_def_2->SetMarkerSize(mSize);
  resMET_def_2->SetLineWidth(lWidth);
  resMET_def_2->SetMarkerStyle(24);
  resMET_def_2->SetMarkerColor(1);

  TH1D *resMET_new_cond_2 = (TH1D*)files.at(1)->Get("hResMET_2");
  resMET_new_cond_2->Draw("same");
  resMET_new_cond_2->SetLineColor(2);
  resMET_new_cond_2->SetMarkerColor(2);
  resMET_new_cond_2->SetMarkerSize(mSize);
  resMET_new_cond_2->SetLineWidth(lWidth);
  resMET_new_cond_2->SetMarkerStyle(20);
  
  TLegend* resLegend2 = new TLegend(0.23, 0.17, 0.43, 0.32);
  resLegend2->AddEntry(resMET_def_2, "Default", "EP");
  resLegend2->AddEntry(resMET_new_cond_2, "New", "EP");
  resLegend2->Draw("SAME");

  mycanvases2.back()->Print(Form("plots/%s_emu.pdf", "resMET_sigma"));
  
  //-----------------------------------------------------------------------
  // Resolution in ET bins plots
  //-----------------------------------------------------------------------
  
  std::map<std::string, TH1F*> resHists_def;
  std::map<std::string, TH1F*> resHists_new_cond;
  std::map<std::string, TH1F*> resHistsRatio;
  
  std::vector<TCanvas*> rcanvases;
  for(auto rType : resTypes) {
    std::string histName(rType);
    // std::string histNameHw(histName);
    //    histName += "Effs_emu";
    //  histNameHw += "Effs_hw";
    rcanvases.push_back(new TCanvas);
    rcanvases.back()->SetWindowSize(rcanvases.back()->GetWw(), 1.*rcanvases.back()->GetWh());
    
    resHists_def[rType] = dynamic_cast<TH1F*>(files.at(0)->Get(histName.c_str()));
    //  resHists_hw[rType] = dynamic_cast<TH1F*>(files.at(0)->Get(histNameHw.c_str()));
    resHists_new_cond[rType] = dynamic_cast<TH1F*>(files.at(1)->Get(histName.c_str()));
    
    resHists_def[rType]->Rebin(rebinFactor);
    //  resHists_hw[rType]->Rebin(rebinFactor);
    resHists_new_cond[rType]->Rebin(rebinFactor);

    resHists_def[rType]->SetLineColor(kBlack); //histColor[rType]);
    //   resHists_hw[rType]->SetLineColor(histColor[rType]);
    resHists_new_cond[rType]->SetLineColor(kRed); //histColor[rType]);

    resHists_def[rType]->Fit("gaus","R+","hist",-2.,2.); //Draw("hist");
    resHists_def[rType]->GetYaxis()->SetRangeUser(0.,1.4*resHists_def[rType]->GetMaximum());
    TF1 *f1 = resHists_def[rType]->GetFunction("gaus"); //->SetLineColor(kBlack);
    f1->SetLineColor(kBlack);
    f1->Draw("SAME");
    
    resHists_new_cond[rType]->Fit("gaus","R+","histsame",-2.,2.);
    TF1 *f2 = resHists_new_cond[rType]->GetFunction("gaus"); //->SetLineColor(kBlack);
    f2->SetLineColor(kRed);
    f2->Draw("SAME");

    rcanvases.back()->Print(Form("plots/%sbin_emu.pdf", rType.c_str()));
    
  }
  for(auto pair : resHists_new_cond) pair.second->SetLineWidth(2);
  // for(auto pair : resHists_hw) pair.second->SetLineStyle(kDashed);
  for(auto pair : resHists_def) pair.second->SetLineStyle(kDotted);
  
  //-----------------------------------------------------------------------
  // Standard L1 quantities
  //-----------------------------------------------------------------------
  
  std::vector<std::string> jetPlots = {"singleJet", "doubleJet", "tripleJet", "quadJet"};
  std::vector<std::string> scalarSumPlots = {"etSum", "htSum"};
  std::vector<std::string> vectorSumPlots = {"metSum", "metHFSum"};

  
  std::vector<TCanvas*> canvases;
  std::vector<TPad*> pad1;
  std::vector<TPad*> pad2;
  std::map<std::string, std::vector<std::string> > plots;
  plots["jet"] = jetPlots;
  plots["scalarSum"] = scalarSumPlots;
  plots["vectorSum"] = vectorSumPlots;
  // plots["resolution"] = resTypes;

  for(auto iplot : plots) {

    canvases.push_back(new TCanvas);
    canvases.back()->SetWindowSize(canvases.back()->GetWw(), 1.*canvases.back()->GetWh());
    pad1.push_back(new TPad("pad1", "pad1", 0, 0.3, 1, 1));
    pad1.back()->SetLogy();
    pad1.back()->SetGrid();
    pad1.back()->Draw();
    pad2.push_back(new TPad("pad2", "pad2", 0, 0, 1, 0.3));
    pad2.back()->SetGrid();
    pad2.back()->Draw();
    
    pad1.back()->cd();

    effHists_def[iplot.second.front()]->Draw("hist");
    effHists_def[iplot.second.front()]->GetYaxis()->SetRangeUser(0.1,20.4*effHists_def[iplot.second.front()]->GetMaximum());
      
    TLegend *leg = new TLegend(0.55, 0.9 - 0.1*iplot.second.size(), 0.95, 0.93);
    for(auto hist : iplot.second) {
      effHists_def[hist]->Draw("hist same");
      if(includeHW) effHists_hw[hist]->Draw("hist same");
      effHists_new_cond[hist]->Draw("hist same");
      TString name(effHists_def[hist]->GetName());
      TString nameHw(effHists_hw[hist]->GetName());
      leg->AddEntry(effHists_def[hist], name + " (current)", "L");
      if(includeHW) leg->AddEntry(effHists_hw[hist], name + " (hw)", "L");
      leg->AddEntry(effHists_new_cond[hist], name + " (new)", "L"); 
    }
    leg->SetBorderSize(0);
    leg->Draw();
    
    pad2.back()->cd();
    effHistsRatio[iplot.second.front()]->Draw("hist");
    // if(includeHW) effHistsRatio[iplot.second.front()]->GetYaxis()->SetTitle("Current/HW");
    //else
    effHistsRatio[iplot.second.front()]->GetYaxis()->SetTitle("New/Current");
    for(auto hist : iplot.second) {
      effHistsRatio[hist]->Draw("hist same");
    }

    //if(includeHW) canvases.back()->Print(Form("plots/%s_hw.pdf", iplot.first.c_str()));
    //else
    canvases.back()->SetLogx();
    canvases.back()->Print(Form("plots/%s_emu.pdf", iplot.first.c_str()));
  }

 

  return 0;
}
