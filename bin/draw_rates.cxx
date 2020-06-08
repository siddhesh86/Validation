#include "PhysicsTools/Utilities/macros/setTDRStyle.C"

#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TFile.h"
#include "TLegend.h"
#include "TROOT.h"
#include "TColor.h"

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <boost/algorithm/string/replace.hpp>

int main()
{
  // include comparisons between HW and data TPs
  bool includeHW = true;
  int rebinFactor = 2;
  bool compare_def_hw_only = true;

  setTDRStyle();
  gROOT->ForceStyle();

  // default, then new conditions
  std::vector<std::string> filenames = {"rates_def.root", "rates_new_cond.root"};
  std::vector<std::string> rateTypes = {"singleJetRates_emu", "doubleJetRates_emu", "tripleJetRates_emu", "quadJetRates_emu",
					"singleEgRates_emu", "singleISOEgRates_emu", "doubleEgRates_emu", "doubleISOEgRates_emu",
					"singleTauRates_emu", "singleISOTauRates_emu", "doubleTauRates_emu", "doubleISOTauRates_emu",
					"htSumRates_emu", "etSumRates_emu", "metSumRates_emu", "metHFSumRates_emu",
					"singleJetRates_HB_emu", "singleJetRates_HE1_emu", "singleJetRates_HE2a_emu", "singleJetRates_HE2b_emu", "singleJetRates_HF_emu"};
  
  std::vector<std::string> sumTypes = {"htSum_emu", "etSum_emu", "metSum_emu", "mhtSum_emu", "metHFSum_emu"};

  //std::vector<std::string> puRangeNames = {"_lowPU", "_midPU", "_highPU", ""};
  std::vector<std::string> puRangeNames = {""};

  std::map<std::string, int> histColor;

  //std::vector<std::vector<int> > puRanges = {{30,46}, {47,63}, {64,80}, {0,100}};
  std::vector<std::vector<int> > puRanges = {{0,100}};

  histColor["singleJetRates_emu"] = histColor["singleEgRates_emu"] = histColor["singleTauRates_emu"] = histColor["etSumRates_emu"] = histColor["metSumRates_emu"] = histColor["etSum_emu"] = histColor["metSum_emu"] = histColor["metSumRates_emu"] = histColor["singleJetRates_HB_emu"] = kRed;
  histColor["doubleJetRates_emu"] = histColor["singleISOEgRates_emu"] = histColor["singleISOTauRates_emu"] = histColor["htSumRates_emu"] = histColor["metHFSumRates_emu"] = histColor["htSum_emu"] = histColor["metHFSum_emu"] = histColor["mhtSum_emu"] = histColor["singleJetRates_HE1_emu"] = kBlue;
  histColor["tripleJetRates_emu"] = histColor["doubleEgRates_emu"] = histColor["doubleTauRates_emu"] = histColor["singleJetRates_HE2a_emu"] = kGreen;
  histColor["quadJetRates_emu"] = histColor["doubleISOEgRates_emu"] = histColor["doubleISOTauRates_emu"] = histColor["singleJetRates_HE2b_emu"] = kBlack;
  histColor["singleJetRates_HF_emu"] = kOrange;
  
  
  std::map<std::string, TH1D*> rateHists_hw;
  std::map<std::string, TH1D*> rateHists_def;
  std::map<std::string, TH1D*> rateHists_new_cond;
  std::map<std::string, TH1D*> sumHists_def;
  std::map<std::string, TH1D*> sumHists_new_cond;
  std::map<std::string, TH1D*> rateHistsRatio;
  
  std::vector<TFile*> files;
  for(auto file : filenames) {
    files.push_back(TFile::Open(file.c_str()));
  }

  for(auto sumType : sumTypes) {
    std::string histName(sumType);

    TH2F* deftemp = dynamic_cast<TH2F*>(files.at(0)->Get(histName.c_str()));
    TH2F* newtemp = dynamic_cast<TH2F*>(files.at(1)->Get(histName.c_str()));
    for (unsigned int irange = 0; irange < puRanges.size(); irange++) {

        std::string custom = puRangeNames[irange]; std::vector<int> bounds = puRanges[irange];

        std::string newName = histName + custom;
        int lowBin = deftemp->GetYaxis()->FindBin(bounds[0]); int hiBin = deftemp->GetYaxis()->FindBin(bounds[1]);
        sumHists_def[newName] = deftemp->ProjectionX((newName+"_def").c_str(), lowBin, hiBin, "");
        sumHists_def[newName]->Rebin(10);
        sumHists_def[newName]->SetLineColor(histColor[histName]);

        sumHists_new_cond[newName] = newtemp->ProjectionX((newName+"_new").c_str(), lowBin, hiBin, "");
        sumHists_new_cond[newName]->Rebin(10);
        sumHists_new_cond[newName]->SetLineColor(histColor[histName]);
    }

  }
  for(auto rateType : rateTypes) {

      std::string histName(rateType);
      std::string histNameHw(histName);
      boost::replace_all(histNameHw, "emu", "hw");
            
      TH2F* deftemp = dynamic_cast<TH2F*>(files.at(0)->Get(histName.c_str()));
      TH2F* newtemp = dynamic_cast<TH2F*>(files.at(1)->Get(histName.c_str()));
      for (unsigned int irange = 0; irange < puRanges.size(); irange++) {

          std::string custom = puRangeNames[irange]; std::vector<int> bounds = puRanges[irange];

          std::string newName = histName + custom; std::string newNameHw = histNameHw + custom;
          int lowBin = deftemp->GetYaxis()->FindBin(bounds[0]); int hiBin = deftemp->GetYaxis()->FindBin(bounds[1]);

          rateHists_def[newName] = deftemp->ProjectionX((newName+"_def").c_str(), lowBin, hiBin, "");
          rateHists_def[newName]->Rebin(rebinFactor);
          rateHists_def[newName]->SetLineColor(histColor[histName]);
          rateHists_def[newName]->SetMaximum(1e11);
          rateHists_def[newName]->SetMinimum(1e2);

          rateHists_new_cond[newName] = newtemp->ProjectionX((newName+"_new").c_str(), lowBin, hiBin, "");
          rateHists_new_cond[newName]->Rebin(rebinFactor);
          rateHists_new_cond[newName]->SetLineColor(histColor[histName]);
          rateHists_new_cond[newName]->SetMaximum(1e11);
          rateHists_new_cond[newName]->SetMinimum(1e2);

          TString name(rateHists_new_cond[newName]->GetName());
          name += "_ratio";

          if(includeHW) {
            newNameHw += "Rates_hw";
	    TH2F* hwtemp = dynamic_cast<TH2F*>(files.at(0)->Get(histNameHw.c_str()));
	    rateHists_hw[newName] = hwtemp->ProjectionX((newNameHw).c_str(), lowBin, hiBin, "");
	    rateHists_hw[newName]->Rebin(rebinFactor);
	    rateHists_hw[newName]->SetLineColor(histColor[rateType]);
	    rateHistsRatio[newName] = dynamic_cast<TH1D*>(rateHists_def[newName]->Clone(name));
	    rateHistsRatio[newName]->Divide(rateHists_hw[newName]); 
	  }
          else {
            rateHistsRatio[newName] = dynamic_cast<TH1D*>(rateHists_new_cond[newName]->Clone(name));
            rateHistsRatio[newName]->Divide(rateHists_def[newName]);

            rateHistsRatio[newName] = dynamic_cast<TH1D*>(rateHists_new_cond[newName]->Clone(name));
            rateHistsRatio[newName]->Divide(rateHists_def[newName]);

          }
          rateHistsRatio[newName]->SetMinimum(0.01);    
          rateHistsRatio[newName]->SetMaximum(1.39);    
          rateHistsRatio[newName]->SetLineWidth(2);    

      }

  }

  for(auto nameHist : rateHists_new_cond)
     nameHist.second->SetLineWidth(2);

  if (includeHW) {
    for(auto pair : rateHists_hw) {
      if ( ! compare_def_hw_only) {
	pair.second->SetLineStyle(kDashed);
      } else {
	pair.second->SetLineWidth(2);
      }
    }
     
  }
  for(auto sumHist : sumHists_def)
    sumHist.second->SetLineStyle(kDotted);

  for(auto nameHist : rateHists_def)
    nameHist.second->SetLineStyle(kDotted);

  std::vector<std::string> jetPlots = {"singleJetRates_emu", "doubleJetRates_emu", "tripleJetRates_emu", "quadJetRates_emu"};
  std::vector<std::string> egPlots = {"singleEgRates_emu", "singleISOEgRates_emu", "doubleEgRates_emu", "doubleISOEgRates_emu"};
  std::vector<std::string> tauPlots = {"singleTauRates_emu", "singleISOTauRates_emu", "doubleTauRates_emu", "doubleISOTauRates_emu"};
  std::vector<std::string> scalarSumPlots = {"etSumRates_emu", "htSumRates_emu"};
  std::vector<std::string> vectorSumPlots = {"metSumRates_emu", "metHFSumRates_emu"};
  std::vector<std::string> jetInEtaBinsPlots = {"singleJetRates_HB_emu", "singleJetRates_HE1_emu", "singleJetRates_HE2a_emu", "singleJetRates_HE2b_emu", "singleJetRates_HF_emu"};

  std::vector<std::string> sumPlots = {"etSum_emu", "htSum_emu", "mhtSum_emu", "metSum_emu", "metHFSum_emu"};
  std::vector<TCanvas*> canvases;
  std::vector<TPad*> pad0;
  std::vector<TPad*> pad1;
  std::vector<TPad*> pad2;

  std::map<std::string, std::vector<std::string> > plots;
  plots["jetRates_emu"] = jetPlots;
  plots["egRates_emu"] = egPlots;
  plots["tauRates_emu"] = tauPlots;
  plots["scalarSumRates_emu"] = scalarSumPlots;
  plots["vectorSumRates_emu"] = vectorSumPlots;
  plots["sums_emu"] = sumPlots;
  plots["jetRates_InEtaBins_emu"] = jetInEtaBinsPlots;

  for(auto iplot : plots["sums_emu"]) {

    for (auto rname : puRangeNames) {
      canvases.push_back(new TCanvas);
      canvases.back()->SetWindowSize(canvases.back()->GetWw(), canvases.back()->GetWh());
      pad0.push_back(new TPad("pad0", "pad0", 0, 0, 1, 1));
      pad0.back()->SetLogy();
      pad0.back()->SetGrid();
      pad0.back()->Draw();
  
      pad0.back()->cd();

      std::string theName = iplot+rname;
  
      sumHists_def[theName]->Draw("hist");

      TLegend *leg = nullptr;
      leg = new TLegend(0.5, 0.85, 0.94, 0.93);

      sumHists_def[theName]->Draw("hist same");
      sumHists_new_cond[theName]->Draw("hist same");
      TString name(sumHists_def[theName]->GetName());

      leg->AddEntry(sumHists_def[theName], name + " (current)", "L");
      leg->AddEntry(sumHists_new_cond[theName], name + " (new)", "L"); 

      leg->SetBorderSize(0);
      leg->Draw();

      canvases.back()->Print(Form("plots/%s.png", (theName.c_str())));
      canvases.back()->Print(Form("plots/%s.pdf", (theName.c_str())));
    }
  }

  for(auto iplot : plots) {

    if (iplot.first.find("sums") != std::string::npos) { continue; }

    for (auto rname : puRangeNames) {
      std::cout << "histo 1st: " << iplot.second.front()+rname << std::endl;
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
      pad1.back()->SetBottomMargin(0.02); 
      rateHists_def[iplot.second.front()+rname]->GetXaxis()->SetLabelSize(0.0);
      if (iplot.second.front().find("Rates") != std::string::npos) {
	rateHists_def[iplot.second.front()+rname]->GetYaxis()->SetTitle("Rate (Hz)");
      }
      rateHists_def[iplot.second.front()+rname]->Draw("hist");

      TLegend *leg = nullptr;
      //if (iplot.first.find("scalarSum") != std::string::npos) {
      //   leg = new TLegend(0.24, 0.16, 0.94, 0.34);
      //} else {
         leg = new TLegend(0.24, 0.75, 0.94, 0.93);
      //}
      leg->SetNColumns(2);

      for(auto hist : iplot.second) {
        std::string theName = hist+rname;
	std::cout << "\t histos: " << theName << std::endl;
        rateHists_def[theName]->Draw("hist same");
        if(includeHW) rateHists_hw[theName]->Draw("hist same");
        if ( ! (includeHW && compare_def_hw_only) ) rateHists_new_cond[theName]->Draw("hist same");
        TString name(rateHists_def[theName]->GetName());

        leg->AddEntry(rateHists_def[theName], name + " (current)", "L");
        if(includeHW) leg->AddEntry(rateHists_hw[theName], name + " (hw)", "L");
        if ( ! (includeHW && compare_def_hw_only) ) leg->AddEntry(rateHists_new_cond[theName], name + " (new)", "L"); 
      }
      leg->SetBorderSize(0);
      leg->Draw();
  
      pad2.back()->cd();
      pad2.back()->SetTopMargin(0.05);
      pad2.back()->SetBottomMargin(0.3);
      auto xLabSize = rateHistsRatio[iplot.second.front()+rname]->GetXaxis()->GetLabelSize();
      auto xTitleSize = rateHistsRatio[iplot.second.front()+rname]->GetXaxis()->GetTitleSize();
      auto yLabSize = rateHistsRatio[iplot.second.front()+rname]->GetYaxis()->GetLabelSize();
      auto yTitleSize = rateHistsRatio[iplot.second.front()+rname]->GetYaxis()->GetTitleSize();
      auto yOffSet = rateHistsRatio[iplot.second.front()+rname]->GetYaxis()->GetTitleOffset();

      rateHistsRatio[iplot.second.front()+rname]->GetXaxis()->SetTitleSize(0.7*xTitleSize/0.3);
      rateHistsRatio[iplot.second.front()+rname]->GetXaxis()->SetLabelSize(0.7*xLabSize/0.3);

      rateHistsRatio[iplot.second.front()+rname]->GetYaxis()->SetTitleSize(0.7*yTitleSize/0.3);
      rateHistsRatio[iplot.second.front()+rname]->GetYaxis()->SetLabelSize(0.7*yLabSize/0.3);

      rateHistsRatio[iplot.second.front()+rname]->GetYaxis()->SetTitleOffset(0.3*yOffSet/0.7);

      rateHistsRatio[iplot.second.front()+rname]->Draw("hist");
      if(includeHW) rateHistsRatio[iplot.second.front()+rname]->GetYaxis()->SetTitle("Current/HW");
      else rateHistsRatio[iplot.second.front()+rname]->GetYaxis()->SetTitle("New / Default");
      for(auto hist : iplot.second) {
        std::string theName = hist+rname;
        rateHistsRatio[theName]->Draw("hist same");
      }

      if(includeHW) {
	canvases.back()->Print(Form("plots/%sRates_hw.png", (iplot.first+rname).c_str()));
	canvases.back()->Print(Form("plots/%sRates_hw.pdf", (iplot.first+rname).c_str()));
      }
      else {
          canvases.back()->Print(Form("plots/%s.png", (iplot.first+rname).c_str()));
	  canvases.back()->Print(Form("plots/%s.pdf", (iplot.first+rname).c_str()));
      } 
    }
  }

  return 0;
}
