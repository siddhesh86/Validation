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
  std::vector<std::string> jetTypes = {"JetEt50","JetEt64","JetEt76","JetEt92","JetEt112","JetEt180"};
  std::vector<std::string> sumrefTypes = {"RefMET"};
  std::vector<std::string> sumTypes = {"MET30","MET40","MET50","MET70","MET100"};
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
  std::map<std::string, TH1F*> metHists_def;
  std::map<std::string, TH1F*> metHists_new_cond;

  std::map<std::string, TGraphAsymmErrors*> jeteffHists_def;
  std::map<std::string, TGraphAsymmErrors*> jeteffHists_new_cond;
  std::map<std::string, TGraphAsymmErrors*> meteffHists_def;
  std::map<std::string, TGraphAsymmErrors*> meteffHists_new_cond;


  //-----------------------------------------------------------------------
  // L1 Jet efficiencies
  //-----------------------------------------------------------------------

  int rebinF=2;
  
  TH1F *jetrefHists_def=dynamic_cast<TH1F*>(files.at(0)->Get("RefmJet"));jetrefHists_def->Rebin(rebinF);
  TH1F *jetrefHists_new_cond=dynamic_cast<TH1F*>(files.at(1)->Get("RefmJet"));jetrefHists_new_cond->Rebin(rebinF);
  
  std::vector<TCanvas*> tcanvases;
  
  for (auto jetType : jetTypes) {

    tcanvases.push_back(new TCanvas);
    tcanvases.back()->SetWindowSize(tcanvases.back()->GetWw(), 1.3*tcanvases.back()->GetWh());
    gPad->SetGridx(); gPad->SetGridy();
    std::string histName(jetType);

    jetHists_def[jetType] = dynamic_cast<TH1F*>(files.at(0)->Get(histName.c_str()));
    jetHists_new_cond[jetType] = dynamic_cast<TH1F*>(files.at(1)->Get(histName.c_str()));

    jetHists_def[jetType]->Rebin(rebinF);
    jetHists_new_cond[jetType]->Rebin(rebinF);

    TGraphAsymmErrors *Eff1 = new TGraphAsymmErrors();
    TGraphAsymmErrors *Eff2 = new TGraphAsymmErrors();
    
    Eff1->BayesDivide(jetHists_def[jetType],jetrefHists_def);
    Eff2->BayesDivide(jetHists_new_cond[jetType],jetrefHists_new_cond);

    jeteffHists_def[jetType] = Eff1;
    jeteffHists_new_cond[jetType] = Eff2;
    
    jeteffHists_def[jetType]->SetMarkerColor(kBlack);
    jeteffHists_new_cond[jetType]->SetMarkerColor(kRed);

    jeteffHists_def[jetType]->SetMarkerSize(0.8);
    jeteffHists_new_cond[jetType]->SetMarkerSize(0.8);
    
    jeteffHists_def[jetType]->Draw("AP");
    jeteffHists_new_cond[jetType]->Draw("P");
    jeteffHists_def[jetType]->GetXaxis()->SetTitle("offline Jet E_{T} (GeV)");
    jeteffHists_def[jetType]->GetYaxis()->SetTitle("Efficiency");

    tcanvases.back()->Print(Form("plots/%sjetEffs_emu.pdf", jetType.c_str()));
    
  }

  //-----------------------------------------------------------------------
  // L1 ETM efficiencies
  //-----------------------------------------------------------------------

  rebinF=10;
  TH1F *metrefHists_def=dynamic_cast<TH1F*>(files.at(0)->Get("RefMET"));metrefHists_def->Rebin(rebinF);
  TH1F *metrefHists_new_cond=dynamic_cast<TH1F*>(files.at(1)->Get("RefMET"));metrefHists_new_cond->Rebin(rebinF);
  
   for (auto metType : sumTypes) {

    tcanvases.push_back(new TCanvas);
    tcanvases.back()->SetWindowSize(tcanvases.back()->GetWw(), 1.3*tcanvases.back()->GetWh());
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
    
    meteffHists_def[metType]->SetMarkerColor(kBlack);
    meteffHists_new_cond[metType]->SetMarkerColor(kRed);

    meteffHists_def[metType]->SetMarkerSize(0.8);
    meteffHists_new_cond[metType]->SetMarkerSize(0.8);
    
    meteffHists_def[metType]->Draw("AP");
    meteffHists_new_cond[metType]->Draw("P");
    meteffHists_def[metType]->GetXaxis()->SetTitle("offline MET (GeV)");
    meteffHists_def[metType]->GetYaxis()->SetTitle("Efficiency");

    tcanvases.back()->Print(Form("plots/%smetEffs_emu.pdf", metType.c_str()));
    

  }
   
   //-----------------------------------------------------------------------
   // L1 Jet resolution summary plots
   //-----------------------------------------------------------------------
  
  TF1 *fgaus = new TF1("g1","gaus");//,-2.,2.);
  fgaus->SetRange(-1.,1.);
  
  // // Jet resolution
  std::vector<TCanvas*> mycanvases;
  TH2F *resJet_def = dynamic_cast<TH2F*>(files.at(0)->Get("hresJet"));resJet_def->RebinX(4);
  //  TH2F *resJet_new_cond = dynamic_cast<TH2F*>(files.at(1)->Get("hresJet"));resJet_new_cond->RebinX(4);
  resJet_def->FitSlicesY(fgaus);
  TH1D *resJet_def_1 = (TH1D*)gDirectory->Get("hresJet_1");
  TH1D *resJet_def_2 = (TH1D*)gDirectory->Get("hresJet_2");
  gROOT->cd();

  TH2F *resJet_new_cond = dynamic_cast<TH2F*>(files.at(1)->Get("hresJet"));resJet_new_cond->RebinX(4);
  resJet_new_cond->FitSlicesY(fgaus);
  TH1D *resJet_new_cond_1 = (TH1D*)gDirectory->Get("hresJet_1");
  TH1D *resJet_new_cond_2 = (TH1D*)gDirectory->Get("hresJet_2");
  gROOT->cd();
  
  mycanvases.push_back(new TCanvas);
  mycanvases.back()->SetWindowSize(mycanvases.back()->GetWw(), 1.3*mycanvases.back()->GetWh());

  gPad->SetGridx(); gPad->SetGridy();
  
  resJet_def_1->Draw("");
  resJet_def_1->GetXaxis()->SetTitle("offline Jet E_{T} (GeV)");
  // resJet_def_1->GetYaxis()->SetTitle("gaussian #mu");
  resJet_def_1->GetYaxis()->SetRangeUser(-1.,1.);
  resJet_def_1->SetMarkerSize(0.5);
  resJet_def_1->SetMarkerStyle(24);
 
  resJet_new_cond_1->Draw("same");
  resJet_new_cond_1->SetLineColor(2);
  resJet_new_cond_1->SetMarkerColor(2);
  resJet_new_cond_1->SetMarkerSize(0.5);
  resJet_new_cond_1->SetMarkerStyle(20);
  mycanvases.back()->Print(Form("plots/%s_emu.pdf", "resJet_mean"));

  mycanvases.push_back(new TCanvas);
  mycanvases.back()->SetWindowSize(mycanvases.back()->GetWw(), 1.3*mycanvases.back()->GetWh());

  gPad->SetGridx(); gPad->SetGridy();
  
  resJet_def_2->Draw("");
  resJet_def_2->GetXaxis()->SetTitle("offline Jet E_{T} (GeV)");
  // resJet_def_2->GetYaxis()->SetTitle("gaussian #sigma");
  resJet_def_2->GetYaxis()->SetRangeUser(0.,0.9);
  resJet_def_2->SetMarkerSize(0.5);
  resJet_def_2->SetMarkerStyle(24);
 
  resJet_new_cond_2->Draw("same");
  resJet_new_cond_2->SetLineColor(2);
  resJet_new_cond_2->SetMarkerColor(2);
  resJet_new_cond_2->SetMarkerSize(0.5);
  resJet_new_cond_2->SetMarkerStyle(20);

  mycanvases.back()->Print(Form("plots/%s_emu.pdf", "resJet_sigma"));

  //-----------------------------------------------------------------------
  // L1 ETM resolution summary plots
  //-----------------------------------------------------------------------
  
  TF1 *fgaus0 = new TF1("g0","gaus");//,-2.,2.);
  fgaus0->SetRange(-1.,3.);
  
  std::vector<TCanvas*> mycanvases2;
  TH2F *resMET_def = dynamic_cast<TH2F*>(files.at(0)->Get("hResMET"));resMET_def->RebinX(5);
  files.at(0)->cd();
  resMET_def->FitSlicesY(fgaus0);//,1,80);//,20);

  TH2F *resMET_new_cond = dynamic_cast<TH2F*>(files.at(1)->Get("hResMET"));resMET_new_cond->RebinX(5);
  files.at(1)->cd();
  resMET_new_cond->FitSlicesY(fgaus0);//,1,80);//,20);
  
  mycanvases2.push_back(new TCanvas);
  mycanvases2.back()->SetWindowSize(mycanvases2.back()->GetWw(), 1.3*mycanvases2.back()->GetWh());

  gPad->SetGridx(); gPad->SetGridy();

  TH1D *resMET_def_1 = (TH1D*)files.at(0)->Get("hResMET_1");
  resMET_def_1->Draw("");
  resMET_def_1->GetXaxis()->SetTitle("offline MET (GeV)");
  resMET_def_1->GetYaxis()->SetRangeUser(-2.,2);
  resMET_def_1->SetMarkerSize(0.5);
  resMET_def_1->SetMarkerStyle(24);
  resMET_def_1->SetMarkerColor(1);
  
  TH1D *resMET_new_cond_1 = (TH1D*)files.at(1)->Get("hResMET_1");
  resMET_new_cond_1->Draw("same");
  resMET_new_cond_1->SetLineColor(2);
  resMET_new_cond_1->SetMarkerColor(2);
  resMET_new_cond_1->SetMarkerSize(0.5);
  resMET_new_cond_1->SetMarkerStyle(20);
  
  mycanvases2.back()->Print(Form("plots/%s_emu.pdf", "resMET_mean"));

  mycanvases2.push_back(new TCanvas);
  mycanvases2.back()->SetWindowSize(mycanvases2.back()->GetWw(), 1.3*mycanvases2.back()->GetWh());

  gPad->SetGridx(); gPad->SetGridy();

  TH1D *resMET_def_2 = (TH1D*)files.at(0)->Get("hResMET_2");
  resMET_def_2->Draw("");
  resMET_def_2->GetXaxis()->SetTitle("offline MET (GeV)");
  resMET_def_2->GetYaxis()->SetRangeUser(0.,1.);
  resMET_def_2->SetMarkerSize(0.5);
  resMET_def_2->SetMarkerStyle(24);
  resMET_def_2->SetMarkerColor(1);

  TH1D *resMET_new_cond_2 = (TH1D*)files.at(1)->Get("hResMET_2");
  resMET_new_cond_2->Draw("same");
  resMET_new_cond_2->SetLineColor(2);
  resMET_new_cond_2->SetMarkerColor(2);
  resMET_new_cond_2->SetMarkerSize(0.5);
  resMET_new_cond_2->SetMarkerStyle(20);
  
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
    rcanvases.back()->SetWindowSize(rcanvases.back()->GetWw(), 1.3*rcanvases.back()->GetWh());
    
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
    canvases.back()->SetWindowSize(canvases.back()->GetWw(), 1.3*canvases.back()->GetWh());
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
    canvases.back()->Print(Form("plots/%s_emu.pdf", iplot.first.c_str()));
  }

 

  return 0;
}
