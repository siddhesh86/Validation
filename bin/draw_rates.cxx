#include "PhysicsTools/Utilities/macros/setTDRStyle.C"

#include "TCanvas.h"
#include "TH1.h"
#include "TFile.h"
#include "TLegend.h"
#include "TROOT.h"

#include <map>
#include <string>
#include <vector>
#include <iostream>


int main()
{
  // include comparisons between HW and data TPs
  bool includeHW = false;
  int rebinFactor = 1;

  setTDRStyle();
  gROOT->ForceStyle();

  // default, then new conditions
  std::vector<std::string> filenames = {"rates_def.root", "rates_new_cond.root"};
  std::vector<std::string> rateTypes = {"singleJet", "doubleJet", "tripleJet", "quadJet",
					"singleEg", "singleISOEg", "doubleEg", "doubleISOEg",
					"singleTau", "singleISOTau", "doubleTau", "doubleISOTau",
					"htSum", "etSum", "metSum", "metHFSum"};
  std::vector<std::string> hcalRegions = {"Incl", "HB", "HE"};
  std::map<std::string, int> histColor;
  histColor["singleJet"] = histColor["singleEg"] = histColor["singleTau"] = histColor["etSum"] = histColor["metSum"] = kRed;
  histColor["doubleJet"] = histColor["singleISOEg"] = histColor["singleISOTau"] = histColor["htSum"] = histColor["metHFSum"] = kBlue;
  histColor["tripleJet"] = histColor["doubleEg"] = histColor["doubleTau"] = kGreen;
  histColor["quadJet"] = histColor["doubleISOEg"] = histColor["doubleISOTau"] = kBlack;

  std::map<std::string, std::map<std::string, TH1F*> > rateHists_hw;
  std::map<std::string, std::map<std::string, TH1F*> > rateHists_def;
  std::map<std::string, std::map<std::string, TH1F*> > rateHists_new_cond;
  std::map<std::string, std::map<std::string, TH1F*> > rateHistsRatio;
  
  std::vector<TFile*> files;
  for(auto file : filenames) {
    files.push_back(TFile::Open(file.c_str()));
  }
  for(auto rateType : rateTypes) {
    for(auto hcalRegion : hcalRegions) {
    std::string histName(rateType);
    std::string histNameHw(histName);
    histName += "Rates_emu";
    if (hcalRegion != "Incl" && rateType.find("Sum") != std::string::npos) { continue; }
    if (hcalRegion != "Incl") { histName += "_"; histName += hcalRegion; }

    rateHists_def[hcalRegion][rateType] = dynamic_cast<TH1F*>(files.at(0)->Get(histName.c_str()));
    rateHists_def[hcalRegion][rateType]->Rebin(rebinFactor);
    rateHists_def[hcalRegion][rateType]->SetLineColor(histColor[rateType]);

    rateHists_new_cond[hcalRegion][rateType] = dynamic_cast<TH1F*>(files.at(1)->Get(histName.c_str())); 
    rateHists_new_cond[hcalRegion][rateType]->Rebin(rebinFactor);
    rateHists_new_cond[hcalRegion][rateType]->SetLineColor(histColor[rateType]);
    TString name(rateHists_new_cond[hcalRegion][rateType]->GetName());
    name += "_ratio";

    if(includeHW) {
      histNameHw += "Rates_hw";
      rateHists_hw[hcalRegion][rateType] = dynamic_cast<TH1F*>(files.at(0)->Get(histNameHw.c_str()));
      rateHists_hw[hcalRegion][rateType]->Rebin(rebinFactor);
      rateHists_hw[hcalRegion][rateType]->SetLineColor(histColor[rateType]);

      rateHistsRatio[hcalRegion][rateType] = dynamic_cast<TH1F*>(rateHists_def[hcalRegion][rateType]->Clone(name));
      rateHistsRatio[hcalRegion][rateType]->Divide(rateHists_hw[hcalRegion][rateType]);
    }
    else {
      rateHistsRatio[hcalRegion][rateType] = dynamic_cast<TH1F*>(rateHists_new_cond[hcalRegion][rateType]->Clone(name));
      rateHistsRatio[hcalRegion][rateType]->Divide(rateHists_def[hcalRegion][rateType]);

      rateHistsRatio[hcalRegion][rateType] = dynamic_cast<TH1F*>(rateHists_new_cond[hcalRegion][rateType]->Clone(name));
      rateHistsRatio[hcalRegion][rateType]->Divide(rateHists_def[hcalRegion][rateType]);

    }
    rateHistsRatio[hcalRegion][rateType]->SetMinimum(0.6);    
    rateHistsRatio[hcalRegion][rateType]->SetMaximum(1.4);    
    rateHistsRatio[hcalRegion][rateType]->SetLineWidth(2);    
    }

  }

  for(auto regionHists : rateHists_new_cond)
    for(auto nameHist : regionHists.second)
        nameHist.second->SetLineWidth(2);

  if (includeHW) {
     for(auto pair : rateHists_hw) 
       for(auto pair2 : pair.second) pair2.second->SetLineStyle(kDashed);
  }

  for(auto regionHists : rateHists_def)
    for(auto nameHist : regionHists.second) nameHist.second->SetLineStyle(kDotted);

  std::vector<std::string> jetPlots = {"singleJet", "doubleJet", "tripleJet", "quadJet"};
  std::vector<std::string> egPlots = {"singleEg", "singleISOEg", "doubleEg", "doubleISOEg"};
  std::vector<std::string> tauPlots = {"singleTau", "singleISOTau", "doubleTau", "doubleISOTau"};
  std::vector<std::string> scalarSumPlots = {"etSum", "htSum"};
  std::vector<std::string> vectorSumPlots = {"metSum", "metHFSum"};

  std::map<std::string, std::vector<TCanvas*> > canvases;
  std::map<std::string, std::vector<TPad*> > pad1;
  std::map<std::string, std::vector<TPad*> > pad2;

  std::map<std::string, std::vector<std::string> > plots;
  plots["jet"] = jetPlots;
  plots["eg"] = egPlots;
  plots["tau"] = tauPlots;
  plots["scalarSum"] = scalarSumPlots;
  plots["vectorSum"] = vectorSumPlots;

  for(auto hcalRegion : hcalRegions) {  
      for(auto iplot : plots) {
        
        if (hcalRegion != "Incl" && iplot.first.find("Sum") != std::string::npos) { continue; }

        canvases[hcalRegion].push_back(new TCanvas);
        canvases[hcalRegion].back()->SetWindowSize(canvases[hcalRegion].back()->GetWw(), 1.3*canvases[hcalRegion].back()->GetWh());
        pad1[hcalRegion].push_back(new TPad("pad1", "pad1", 0, 0.3, 1, 1));
        pad1[hcalRegion].back()->SetLogy();
        pad1[hcalRegion].back()->SetGrid();
        pad1[hcalRegion].back()->Draw();
        pad2[hcalRegion].push_back(new TPad("pad2", "pad2", 0, 0, 1, 0.3));
        pad2[hcalRegion].back()->SetGrid();
        pad2[hcalRegion].back()->Draw();
       
        pad1[hcalRegion].back()->cd();
      
        rateHists_def[hcalRegion][iplot.second.front()]->Draw("hist");

        TLegend *leg = new TLegend(0.55, 0.9 - 0.1*iplot.second.size(), 0.95, 0.93);

        for(auto hist : iplot.second) {

          rateHists_def[hcalRegion][hist]->Draw("hist same");
          if(includeHW) rateHists_hw[hcalRegion][hist]->Draw("hist same");
          rateHists_new_cond[hcalRegion][hist]->Draw("hist same");
          TString name(rateHists_def[hcalRegion][hist]->GetName());

          leg->AddEntry(rateHists_def[hcalRegion][hist], name + " (current)", "L");
          if(includeHW) leg->AddEntry(rateHists_hw[hcalRegion][hist], name + " (hw)", "L");
          leg->AddEntry(rateHists_new_cond[hcalRegion][hist], name + " (new)", "L"); 
        }
        leg->SetBorderSize(0);
        leg->Draw();
     
        pad2[hcalRegion].back()->cd();
        rateHistsRatio[hcalRegion][iplot.second.front()]->Draw("hist");
        if(includeHW) rateHistsRatio[hcalRegion][iplot.second.front()]->GetYaxis()->SetTitle("Current/HW");
        else rateHistsRatio[hcalRegion][iplot.second.front()]->GetYaxis()->SetTitle("New/Current");
        for(auto hist : iplot.second) {
          rateHistsRatio[hcalRegion][hist]->Draw("hist same");
        }

        if(includeHW) canvases[hcalRegion].back()->Print(Form("plots/%sRates_hw.pdf", iplot.first.c_str()));
        else canvases[hcalRegion].back()->Print(Form("plots/%sRates_emu_%s.pdf", iplot.first.c_str(),hcalRegion.c_str()));
      }
   }

  return 0;
}
