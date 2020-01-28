#include "PhysicsTools/Utilities/macros/setTDRStyle.C"

#include "TCanvas.h"
#include "TH1.h"
#include "TFile.h"
#include "TLegend.h"
#include "TROOT.h"
#include "TColor.h"

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
  std::vector<std::string> rateTypes = {"singleJetRates_emu", "doubleJetRates_emu", "tripleJetRates_emu", "quadJetRates_emu", "singleJetRates_emu_HB", "singleJetRates_emu_HE1", "singleJetRates_emu_HE2",
                    "singleJet12Rates_emu_ieta", "singleJet35Rates_emu_ieta", "singleJet60Rates_emu_ieta", "singleJet90Rates_emu_ieta", "singleJet120Rates_emu_ieta", "singleJet180Rates_emu_ieta",
                    "doubleJet12Rates_emu_ieta", "doubleJet35Rates_emu_ieta", "doubleJet60Rates_emu_ieta", "doubleJet90Rates_emu_ieta", "doubleJet120Rates_emu_ieta", "doubleJet180Rates_emu_ieta",
                    "tripleJet12Rates_emu_ieta", "tripleJet35Rates_emu_ieta", "tripleJet60Rates_emu_ieta", "tripleJet90Rates_emu_ieta", "tripleJet120Rates_emu_ieta", "tripleJet180Rates_emu_ieta",
                    "quadJet12Rates_emu_ieta", "quadJet35Rates_emu_ieta", "quadJet60Rates_emu_ieta", "quadJet90Rates_emu_ieta", "quadJet120Rates_emu_ieta", "quadJet180Rates_emu_ieta",
                    "singleJet12Rates_emu_sub", "singleJet35Rates_emu_sub", "singleJet60Rates_emu_sub", "singleJet90Rates_emu_sub", "singleJet120Rates_emu_sub", "singleJet180Rates_emu_sub",
                    "doubleJet12Rates_emu_sub", "doubleJet35Rates_emu_sub", "doubleJet60Rates_emu_sub", "doubleJet90Rates_emu_sub", "doubleJet120Rates_emu_sub", "doubleJet180Rates_emu_sub",
                    "tripleJet12Rates_emu_sub", "tripleJet35Rates_emu_sub", "tripleJet60Rates_emu_sub", "tripleJet90Rates_emu_sub", "tripleJet120Rates_emu_sub", "tripleJet180Rates_emu_sub",
                    "quadJet12Rates_emu_sub", "quadJet35Rates_emu_sub", "quadJet60Rates_emu_sub", "quadJet90Rates_emu_sub", "quadJet120Rates_emu_sub", "quadJet180Rates_emu_sub",
					"singleEgRates_emu", "singleISOEgRates_emu", "doubleEgRates_emu", "doubleISOEgRates_emu",
					"singleTauRates_emu", "singleISOTauRates_emu", "doubleTauRates_emu", "doubleISOTauRates_emu",
					"htSumRates_emu", "etSumRates_emu", "metSumRates_emu", "metHFSumRates_emu"};

  std::vector<std::string> sumTypes = {"htSum_emu", "etSum_emu", "metSum_emu", "mhtSum_emu", "metHFSum_emu"};

  std::map<std::string, int> histColor;
  histColor["singleJet12Rates_emu_ieta"]  = histColor["singleJet35Rates_emu_ieta"]  = histColor["singleJet60Rates_emu_ieta"]  = histColor["singleJet90Rates_emu_ieta"]  = histColor["singleJet120Rates_emu_ieta"] = histColor["singleJet180Rates_emu_ieta"] = histColor["singleJetRates_emu_HE2"] = kRed;
  histColor["doubleJet12Rates_emu_ieta"]  = histColor["doubleJet35Rates_emu_ieta"]  = histColor["doubleJet60Rates_emu_ieta"]  = histColor["doubleJet90Rates_emu_ieta"]  = histColor["doubleJet120Rates_emu_ieta"] = histColor["doubleJet180Rates_emu_ieta"] = histColor["singleJetRates_emu_HE1"] = kBlue;
  histColor["tripleJet12Rates_emu_ieta"]  = histColor["tripleJet35Rates_emu_ieta"]  = histColor["tripleJet60Rates_emu_ieta"]  = histColor["tripleJet90Rates_emu_ieta"]  = histColor["tripleJet120Rates_emu_ieta"] = histColor["tripleJet180Rates_emu_ieta"] = kGreen;
  histColor["quadJet12Rates_emu_ieta"]    = histColor["quadJet35Rates_emu_ieta"]    = histColor["quadJet60Rates_emu_ieta"]    = histColor["quadJet90Rates_emu_ieta"]    = histColor["quadJet120Rates_emu_ieta"]   = histColor["quadJet180Rates_emu_ieta"]   = histColor["singleJetRates_emu_HB"] = kBlack;

  histColor["singleJet12Rates_emu_sub"]  = histColor["singleJet35Rates_emu_sub"]  = histColor["singleJet60Rates_emu_sub"]  = histColor["singleJet90Rates_emu_sub"]  = histColor["singleJet120Rates_emu_sub"] = histColor["singleJet180Rates_emu_sub"] = kRed;
  histColor["doubleJet12Rates_emu_sub"]  = histColor["doubleJet35Rates_emu_sub"]  = histColor["doubleJet60Rates_emu_sub"]  = histColor["doubleJet90Rates_emu_sub"]  = histColor["doubleJet120Rates_emu_sub"] = histColor["doubleJet180Rates_emu_sub"] = kBlue;
  histColor["tripleJet12Rates_emu_sub"]  = histColor["tripleJet35Rates_emu_sub"]  = histColor["tripleJet60Rates_emu_sub"]  = histColor["tripleJet90Rates_emu_sub"]  = histColor["tripleJet120Rates_emu_sub"] = histColor["tripleJet180Rates_emu_sub"] = kGreen;
  histColor["quadJet12Rates_emu_sub"]    = histColor["quadJet35Rates_emu_sub"]    = histColor["quadJet60Rates_emu_sub"]    = histColor["quadJet90Rates_emu_sub"]    = histColor["quadJet120Rates_emu_sub"]   = histColor["quadJet180Rates_emu_sub"]   = kBlack;

  histColor["singleJetRates_emu"] = histColor["singleEgRates_emu"] = histColor["singleTauRates_emu"] = histColor["etSumRates_emu"] = histColor["metSumRates_emu"] = histColor["etSum_emu"] = histColor["metSum_emu"] = histColor["metSumRates_emu"] = kRed;
  histColor["doubleJetRates_emu"] = histColor["singleISOEgRates_emu"] = histColor["singleISOTauRates_emu"] = histColor["htSumRates_emu"] = histColor["metHFSumRates_emu"] = histColor["htSum_emu"] = histColor["metHFSum_emu"] = histColor["mhtSum_emu"] = kBlue;
  histColor["tripleJetRates_emu"] = histColor["doubleEgRates_emu"] = histColor["doubleTauRates_emu"] = kGreen;
  histColor["quadJetRates_emu"] = histColor["doubleISOEgRates_emu"] = histColor["doubleISOTauRates_emu"] = kBlack;

  std::map<std::string, TH1F*> rateHists_hw;
  std::map<std::string, TH1F*> rateHists_def;
  std::map<std::string, TH1F*> rateHists_new_cond;
  std::map<std::string, TH1F*> sumHists_def;
  std::map<std::string, TH1F*> sumHists_new_cond;
  std::map<std::string, TH1F*> rateHistsRatio;
  
  std::vector<TFile*> files;
  for(auto file : filenames) {
    files.push_back(TFile::Open(file.c_str()));
  }

  for(auto sumType : sumTypes) {
    std::string histName(sumType);

    sumHists_def[histName] = dynamic_cast<TH1F*>(files.at(0)->Get(histName.c_str()));
    sumHists_def[histName]->Rebin(10);
    sumHists_def[histName]->SetLineColor(histColor[histName]);
    //sumHists_def[histName]->SetMaximum(1e9);
    //sumHists_def[histName]->SetMinimum(1e1);

    sumHists_new_cond[histName] = dynamic_cast<TH1F*>(files.at(1)->Get(histName.c_str())); 
    sumHists_new_cond[histName]->Rebin(10);
    sumHists_new_cond[histName]->SetLineColor(histColor[histName]);
    //sumHists_new_cond[histName]->SetMaximum(1e9);
    //sumHists_new_cond[histName]->SetMinimum(1e1);

  }
  for(auto rateType : rateTypes) {
      std::string histName(rateType);
      std::string histNameHw(histName);

      rateHists_def[histName] = dynamic_cast<TH1F*>(files.at(0)->Get(histName.c_str()));
      rateHists_def[histName]->Rebin(rebinFactor);
      rateHists_def[histName]->SetLineColor(histColor[histName]);
      rateHists_def[histName]->SetMaximum(1e9);
      rateHists_def[histName]->SetMinimum(1e1);

      rateHists_new_cond[histName] = dynamic_cast<TH1F*>(files.at(1)->Get(histName.c_str())); 
      rateHists_new_cond[histName]->Rebin(rebinFactor);
      rateHists_new_cond[histName]->SetLineColor(histColor[histName]);
      rateHists_new_cond[histName]->SetMaximum(1e8);
      rateHists_new_cond[histName]->SetMinimum(1e1);

      TString name(rateHists_new_cond[histName]->GetName());
      name += "_ratio";

      if(includeHW) {
        histNameHw += "Rates_hw";
        rateHists_hw[rateType] = dynamic_cast<TH1F*>(files.at(0)->Get(histNameHw.c_str()));
        rateHists_hw[rateType]->Rebin(rebinFactor);
        rateHists_hw[rateType]->SetLineColor(histColor[rateType]);

        rateHistsRatio[rateType] = dynamic_cast<TH1F*>(rateHists_def[rateType]->Clone(name));
        rateHistsRatio[rateType]->Divide(rateHists_hw[rateType]);
      }
      else {
        rateHistsRatio[histName] = dynamic_cast<TH1F*>(rateHists_new_cond[histName]->Clone(name));
        rateHistsRatio[histName]->Divide(rateHists_def[histName]);

        rateHistsRatio[histName] = dynamic_cast<TH1F*>(rateHists_new_cond[histName]->Clone(name));
        rateHistsRatio[histName]->Divide(rateHists_def[histName]);

      }
      rateHistsRatio[histName]->SetMinimum(0.0);    
      rateHistsRatio[histName]->SetMaximum(1.4);    
      rateHistsRatio[histName]->SetLineWidth(2);    

  }

  for(auto nameHist : rateHists_new_cond)
     nameHist.second->SetLineWidth(2);

  if (includeHW) {
     for(auto pair : rateHists_hw) 
       pair.second->SetLineStyle(kDashed);
  }
  for(auto sumHist : sumHists_def)
    sumHist.second->SetLineStyle(kDotted);

  for(auto nameHist : rateHists_def)
    nameHist.second->SetLineStyle(kDotted);

  std::vector<std::string> jetPlots = {"singleJetRates_emu", "doubleJetRates_emu", "tripleJetRates_emu", "quadJetRates_emu"};
  std::vector<std::string> jetPlots_sub = {"singleJetRates_emu_HB", "singleJetRates_emu_HE1", "singleJetRates_emu_HE2"};
  std::vector<std::string> jet12Plots_ieta = {"singleJet12Rates_emu_ieta", "doubleJet12Rates_emu_ieta", "tripleJet12Rates_emu_ieta", "quadJet12Rates_emu_ieta"};
  std::vector<std::string> jet35Plots_ieta = {"singleJet35Rates_emu_ieta", "doubleJet35Rates_emu_ieta", "tripleJet35Rates_emu_ieta", "quadJet35Rates_emu_ieta"};
  std::vector<std::string> jet60Plots_ieta = {"singleJet60Rates_emu_ieta", "doubleJet60Rates_emu_ieta", "tripleJet60Rates_emu_ieta", "quadJet60Rates_emu_ieta"};
  std::vector<std::string> jet90Plots_ieta = {"singleJet90Rates_emu_ieta", "doubleJet90Rates_emu_ieta", "tripleJet90Rates_emu_ieta", "quadJet90Rates_emu_ieta"};
  std::vector<std::string> jet120Plots_ieta = {"singleJet120Rates_emu_ieta", "doubleJet120Rates_emu_ieta", "tripleJet120Rates_emu_ieta", "quadJet120Rates_emu_ieta"};
  std::vector<std::string> jet180Plots_ieta = {"singleJet180Rates_emu_ieta", "doubleJet180Rates_emu_ieta", "tripleJet180Rates_emu_ieta", "quadJet180Rates_emu_ieta"};
  std::vector<std::string> jet12Plots_sub = {"singleJet12Rates_emu_sub", "doubleJet12Rates_emu_sub", "tripleJet12Rates_emu_sub", "quadJet12Rates_emu_sub"};
  std::vector<std::string> jet35Plots_sub = {"singleJet35Rates_emu_sub", "doubleJet35Rates_emu_sub", "tripleJet35Rates_emu_sub", "quadJet35Rates_emu_sub"};
  std::vector<std::string> jet60Plots_sub = {"singleJet60Rates_emu_sub", "doubleJet60Rates_emu_sub", "tripleJet60Rates_emu_sub", "quadJet60Rates_emu_sub"};
  std::vector<std::string> jet90Plots_sub = {"singleJet90Rates_emu_sub", "doubleJet90Rates_emu_sub", "tripleJet90Rates_emu_sub", "quadJet90Rates_emu_sub"};
  std::vector<std::string> jet120Plots_sub = {"singleJet120Rates_emu_sub", "doubleJet120Rates_emu_sub", "tripleJet120Rates_emu_sub", "quadJet120Rates_emu_sub"};
  std::vector<std::string> jet180Plots_sub = {"singleJet180Rates_emu_sub", "doubleJet180Rates_emu_sub", "tripleJet180Rates_emu_sub", "quadJet180Rates_emu_sub"};
  std::vector<std::string> egPlots = {"singleEgRates_emu", "singleISOEgRates_emu", "doubleEgRates_emu", "doubleISOEgRates_emu"};
  std::vector<std::string> tauPlots = {"singleTauRates_emu", "singleISOTauRates_emu", "doubleTauRates_emu", "doubleISOTauRates_emu"};
  std::vector<std::string> scalarSumPlots = {"etSumRates_emu", "htSumRates_emu"};
  std::vector<std::string> vectorSumPlots = {"metSumRates_emu", "metHFSumRates_emu"};

  std::vector<std::string> sumPlots = {"etSum_emu", "htSum_emu", "mhtSum_emu", "metSum_emu", "metHFSum_emu"};
  std::vector<TCanvas*> canvases;
  std::vector<TPad*> pad0;
  std::vector<TPad*> pad1;
  std::vector<TPad*> pad2;

  std::map<std::string, std::vector<std::string> > plots;
  plots["jetRates_emu"] = jetPlots;
  plots["jetRates_emu_sub"] = jetPlots_sub;
  plots["jet12Rates_emu_ieta"] = jet12Plots_ieta;
  plots["jet35Rates_emu_ieta"] = jet35Plots_ieta;
  plots["jet60Rates_emu_ieta"] = jet60Plots_ieta;
  plots["jet90Rates_emu_ieta"] = jet90Plots_ieta;
  plots["jet120Rates_emu_ieta"] = jet120Plots_ieta;
  plots["jet180Rates_emu_ieta"] = jet180Plots_ieta;
  plots["jet12Rates_emu_sub"] = jet12Plots_sub;
  plots["jet35Rates_emu_sub"] = jet35Plots_sub;
  plots["jet60Rates_emu_sub"] = jet60Plots_sub;
  plots["jet90Rates_emu_sub"] = jet90Plots_sub;
  plots["jet120Rates_emu_sub"] = jet120Plots_sub;
  plots["jet180Rates_emu_sub"] = jet180Plots_sub;
  plots["egRates_emu"] = egPlots;
  plots["tauRates_emu"] = tauPlots;
  plots["scalarSumRates_emu"] = scalarSumPlots;
  plots["vectorSumRates_emu"] = vectorSumPlots;
  plots["sums_emu"] = sumPlots;

  for(auto iplot : plots["sums_emu"]) {

    canvases.push_back(new TCanvas);
    canvases.back()->SetWindowSize(canvases.back()->GetWw(), canvases.back()->GetWh());
    pad0.push_back(new TPad("pad0", "pad0", 0, 0, 1, 1));
    pad0.back()->SetLogy();
    pad0.back()->SetGrid();
    pad0.back()->Draw();
  
    pad0.back()->cd();
  
    sumHists_def[iplot]->Draw("hist");

    TLegend *leg = nullptr;
    leg = new TLegend(0.5, 0.85, 0.94, 0.93);

    sumHists_def[iplot]->Draw("hist same");
    sumHists_new_cond[iplot]->Draw("hist same");
    TString name(sumHists_def[iplot]->GetName());

    leg->AddEntry(sumHists_def[iplot], name + " (current)", "L");
    leg->AddEntry(sumHists_new_cond[iplot], name + " (new)", "L"); 

    leg->SetBorderSize(0);
    leg->Draw();

    canvases.back()->Print(Form("plots/%s.pdf", iplot.c_str()));
  }

  for(auto iplot : plots) {

    if (iplot.first.find("sums") != std::string::npos) { continue; }

    std::cout << iplot.first << std::endl;
    
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
  
    rateHists_def[iplot.second.front()]->Draw("hist");

    TLegend *leg = nullptr;
    if (iplot.first.find("scalarSum") != std::string::npos) {
       leg = new TLegend(0.24, 0.16, 0.94, 0.34);
    } else {
       leg = new TLegend(0.24, 0.75, 0.94, 0.93);
    }
    leg->SetNColumns(2);

    for(auto hist : iplot.second) {

      //if (hist.find("singleJetRates_emu") != std::string::npos || hist.find("metSumRates_emu") != std::string::npos) {
      //    std::cout << "DOING RATE MATCHING FOR " << hist << std::endl;
      //    for (int ibin = 0; ibin < rateHists_def[hist]->GetXaxis()->GetNbins(); ibin++) {
      //        double pfa2Rate = rateHists_def[hist]->GetBinContent(ibin);
      //        double pfa2Thresh = rateHists_def[hist]->GetXaxis()->GetBinCenter(ibin);
      //        if (rateHists_def[hist]->GetXaxis()->GetBinCenter(ibin) > 180) { break; }
      //        double rateDiff = 10e10; double pfa3pRate = 0; double thresholdMatch = 0.0; double rateMatch = 0.0;
      //        for (int jbin = 0; jbin < rateHists_new_cond[hist]->GetXaxis()->GetNbins(); jbin++) {
      //            pfa3pRate = rateHists_new_cond[hist]->GetBinContent(jbin);
      //            if (fabs(pfa2Rate - pfa3pRate) < rateDiff) {
      //               rateDiff = fabs(pfa2Rate - pfa3pRate);
      //               rateMatch = pfa3pRate;
      //               thresholdMatch = rateHists_new_cond[hist]->GetXaxis()->GetBinCenter(jbin);
      //            }
      //        }

      //        if (pfa2Thresh == 12.5 || pfa2Thresh == 35.5 || pfa2Thresh == 60.5 || pfa2Thresh == 90.5 || pfa2Thresh == 120.5 || pfa2Thresh == 180.5) {  
      //            std::cout << "DEF THESHOLD: " << rateHists_def[hist]->GetXaxis()->GetBinCenter(ibin) << " GeV ==> RATE: " << pfa2Rate << " NEW THRESHOLD: " << thresholdMatch << " ==> RATE: " << rateMatch << std::endl;
      //        }
      //    }
      //}

      rateHists_def[hist]->Draw("hist same");
      if(includeHW) rateHists_hw[hist]->Draw("hist same");
      rateHists_new_cond[hist]->Draw("hist same");
      TString name(rateHists_def[hist]->GetName());

      leg->AddEntry(rateHists_def[hist], name + " (current)", "L");
      if(includeHW) leg->AddEntry(rateHists_hw[hist], name + " (hw)", "L");
      leg->AddEntry(rateHists_new_cond[hist], name + " (new)", "L"); 
    }
    leg->SetBorderSize(0);
    leg->Draw();
  
    pad2.back()->cd();
    rateHistsRatio[iplot.second.front()]->Draw("hist");
    if(includeHW) rateHistsRatio[iplot.second.front()]->GetYaxis()->SetTitle("Current/HW");
    else rateHistsRatio[iplot.second.front()]->GetYaxis()->SetTitle("New/Current");
    for(auto hist : iplot.second) {
      rateHistsRatio[hist]->Draw("hist same");
    }

    if(includeHW) canvases.back()->Print(Form("plots/%sRates_hw.pdf", iplot.first.c_str()));
    else {
        canvases.back()->Print(Form("plots/%s.pdf", iplot.first.c_str()));
    }
  }

  return 0;
}
