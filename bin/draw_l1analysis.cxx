#include "PhysicsTools/Utilities/macros/setTDRStyle.C"

#include "TCanvas.h"
#include "TH1.h"
#include "TFile.h"
#include "TLegend.h"
#include "TROOT.h"

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
  std::vector<std::string> filenames = {"l1analysis_def.root", "l1analysis_linear_luts.root"};
  std::vector<std::string> effTypes = {"singleJet", "doubleJet", "tripleJet", "quadJet",
					"htSum", "etSum", "metSum", "metHFSum"};
  std::map<std::string, int> histColor;
  histColor["singleJet"] = histColor["etSum"] = histColor["metSum"] = kRed;
  histColor["doubleJet"] = histColor["htSum"] = histColor["metHFSum"] = kBlue;
  histColor["tripleJet"] = kGreen;
  histColor["quadJet"] = kBlack;

  std::map<std::string, TH1F*> effHists_def;
  std::map<std::string, TH1F*> effHists_new_cond;
  std::map<std::string, TH1F*> effHists_hw;
  std::map<std::string, TH1F*> effHistsRatio;
  
  std::vector<TFile*> files;
  for(auto file : filenames) {
    files.push_back(TFile::Open(file.c_str()));
  }
  for(auto effType : effTypes) {
    std::string histName(effType);
    std::string histNameHw(histName);
    //    histName += "Effs_emu";
    //  histNameHw += "Effs_hw";
    effHists_def[effType] = dynamic_cast<TH1F*>(files.at(0)->Get(histName.c_str()));
    effHists_hw[effType] = dynamic_cast<TH1F*>(files.at(0)->Get(histNameHw.c_str()));
    effHists_new_cond[effType] = dynamic_cast<TH1F*>(files.at(1)->Get(histName.c_str()));
    
    effHists_def[effType]->Rebin(rebinFactor);
    effHists_hw[effType]->Rebin(rebinFactor);
    effHists_new_cond[effType]->Rebin(rebinFactor);

    effHists_def[effType]->SetLineColor(histColor[effType]);
    effHists_hw[effType]->SetLineColor(histColor[effType]);
    effHists_new_cond[effType]->SetLineColor(histColor[effType]);
    TString name(effHists_new_cond[effType]->GetName());
    name += "_ratio";
    if(includeHW) {
      effHistsRatio[effType] = dynamic_cast<TH1F*>(effHists_def[effType]->Clone(name));
      effHistsRatio[effType]->Divide(effHists_hw[effType]);
    }
    else {
      effHistsRatio[effType] = dynamic_cast<TH1F*>(effHists_new_cond[effType]->Clone(name));
      effHistsRatio[effType]->Divide(effHists_def[effType]);
    }
    effHistsRatio[effType]->SetMinimum(0.6);    
    effHistsRatio[effType]->SetMaximum(1.4);    
    effHistsRatio[effType]->SetLineWidth(2);    
  }
  for(auto pair : effHists_new_cond) pair.second->SetLineWidth(2);
  for(auto pair : effHists_hw) pair.second->SetLineStyle(kDashed);
  for(auto pair : effHists_def) pair.second->SetLineStyle(kDotted);

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
    if(includeHW) effHistsRatio[iplot.second.front()]->GetYaxis()->SetTitle("Current/HW");
    else effHistsRatio[iplot.second.front()]->GetYaxis()->SetTitle("New/Current");
    for(auto hist : iplot.second) {
      effHistsRatio[hist]->Draw("hist same");
    }

    if(includeHW) canvases.back()->Print(Form("plots/%sEffs_hw.pdf", iplot.first.c_str()));
    else canvases.back()->Print(Form("plots/%sEffs_emu.pdf", iplot.first.c_str()));
  }

  return 0;
}
