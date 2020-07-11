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


double getHistogramMinAbvZero(TH1D *h) {
  double ymin = h->GetMaximum();

  for (int i=1; i<= h->GetNbinsX(); i++) {
    double y = h->GetBinContent(i);
    if (y > 0.0 and y < ymin) {
      ymin = y;
    }
  }

  return ymin;
}


int main()
{
  
  setTDRStyle();
  gROOT->ForceStyle();
  gStyle->SetOptTitle(0);
  gStyle->SetPadLeftMargin(0.14);
  
  //std::vector<std::string> filenames      = {"histo_CaloTP_2018_Hardware.root", "histo_CaloTP_2018_PFA2.root"};
  //std::vector<std::string> filenames      = {"histo_CaloTP_2018_PFA2.root", "histo_CaloTP_2018_PFA1p.root", "histo_CaloTP_2018_PFA1.root"};
  
  //std::vector<std::string> filenames      = {"histo_CaloTP_Run3_PFA2.root", "histo_CaloTP_Run3_PFA1p.root", "histo_CaloTP_Run3_PFA1.root"};


  //std::vector<std::string> filenames      = {"histo_CaloTP_2018_PFA2.root", "histo_CaloTP_Run3_PFA2.root"};
  //std::vector<std::string> filenames      = {"histo_CaloTP_2018_PFA1p.root", "histo_CaloTP_Run3_PFA1p.root"};
  std::vector<std::string> filenames      = {"histo_CaloTP_2018_PFA1.root", "histo_CaloTP_Run3_PFA1.root"};
  
  
  std::vector<std::string> histosToCompare = {"ecalTPEta", "hcalTPEta", "ecalTPETEta", "hcalTPETEta"};
  
  //std::map<std::string, int> histColor;
  std::vector<int> histColor = {1, 2, 4, 5, 6};


  std::vector<TFile*> files;
  for(auto file : filenames) {
    files.push_back(TFile::Open(file.c_str()));
  }

  //TCanvas *canvas = new TCanvas;
  //canvas->SetWindowSize(1.1*canvas->GetWw(), 1.3*canvas->GetWh());
  std::vector<TCanvas*> tcanvases;
  std::vector<TPad*> pad0;
  std::vector<TPad*> pad1;
  std::vector<TPad*> pad2;
  
  for (auto sHisto : histosToCompare) {
    printf("\n\nsHisto %s",sHisto.c_str());

    int iFile = 0;
    std::string sFileTags("");
    TLegend *leg = new TLegend(0.5, 0.85, 0.99, 0.99);

    std::vector<std::string>  histoTags; 
    std::map<std::string, TH1D*> histos;
    std::map<std::string, TH1D*> hRatios;
    
    double ymin_plot = 1000.;
    double ymax_plot = 0.;
    double ymin_ratio = 1000.;
    double ymax_ratio = 0.;

    //canvas->Clear();
    tcanvases.push_back(new TCanvas);
    tcanvases.back()->SetWindowSize(tcanvases.back()->GetWw(), 1.3*tcanvases.back()->GetWh());
    pad1.push_back(new TPad("pad1", "pad1", 0, 0.3, 1, 1));
    pad1.back()->SetGrid();
    pad1.back()->Draw();
    pad2.push_back(new TPad("pad2", "pad2", 0, 0, 1, 0.3));
    pad2.back()->SetGrid();
    pad2.back()->Draw();
	       
    pad1.back()->SetBottomMargin(0.02);
    
    pad2.back()->SetTopMargin(0.05);
    pad2.back()->SetBottomMargin(0.3);
      
    for(auto sFile : filenames) {
      
      std::string sFileTag(sFile);
      boost::replace_all(sFileTag, ".root", "");
      boost::replace_all(sFileTag, "histo_CaloTP_", "");
      sFileTags += "_" + sFileTag;
      printf("\nsFile %s,   sFileTag %s \n",sFile.c_str(),sFileTag.c_str());
      histoTags.push_back(sFileTags);
	
      TFile *tFile = TFile::Open(sFile.c_str());
      
      TH1D *hist = dynamic_cast<TH1D*>(tFile->Get(sHisto.c_str()));
      printf("%s: %s: nBins %d\n",sFile.c_str(),sHisto.c_str(), hist->GetNbinsX());

      hist->SetTitle("");
      hist->SetLineColor(histColor[iFile]);
      hist->SetMarkerColor(histColor[iFile]);
      
      //canvas->cd();
      pad1.back()->cd();

      histos[histoTags.back()] = hist;
      
      if (iFile == 0) {
	double ymax = hist->GetMaximum();
	double ymin = hist->GetMinimum();
	//if (ymin < ymin_plot) {
	// ymin_plot = ymin;
	//}
	if (ymax > ymax_ratio) {
	  ymax_plot = ymax;
	}
	printf("Plot: y(%f, %f) y-set(%f, %f)\n",ymin,ymax, ymin_plot,ymax_plot);
	histos[histoTags.back()]->GetYaxis()->SetRangeUser(ymin_plot, 1.3*ymax_plot);
	
	histos[histoTags.back()]->Draw();

      }
      else {
	histos[histoTags.back()]->Draw("same");
      }

      leg->AddEntry(histos[histoTags.back()], sFileTag.c_str(), "EP");


      pad2.back()->cd();

      if (iFile > 0) { // plot ratio histogram w.r.t. hist[0]
	TH1D *hRatio = (TH1D*)hist->Clone(Form("Ratio_%s_%s",sHisto.c_str(),sFileTag.c_str()));

	hRatio->Divide(histos[histoTags.at(0)]);


	auto xLabSize = hRatio->GetXaxis()->GetLabelSize();
	auto xTitleSize = hRatio->GetXaxis()->GetTitleSize();
	auto yLabSize = hRatio->GetYaxis()->GetLabelSize();
	auto yTitleSize = hRatio->GetYaxis()->GetTitleSize();
	auto yOffSet = hRatio->GetYaxis()->GetTitleOffset();

	hRatio->GetXaxis()->SetTitleSize(0.7*xTitleSize/0.3);
	hRatio->GetXaxis()->SetLabelSize(0.7*xLabSize/0.3);

	hRatio->GetYaxis()->SetTitleSize(0.7*yTitleSize/0.3);
	hRatio->GetYaxis()->SetLabelSize(0.7*yLabSize/0.3);

	hRatio->GetYaxis()->SetTitleOffset(0.3*yOffSet/0.7);

	hRatio->GetYaxis()->SetNdivisions(505);

	

	std::string sN = "ReEmu scheme";
	std::string sD(histoTags.at(0)); boost::replace_first(sD, "_", "");	
	hRatio->GetYaxis()->SetTitle(Form("#frac{%s}{%s}",sN.c_str(), sD.c_str()));

	hRatios[histoTags.back()] = hRatio;
		
	double ymax = hRatio->GetMaximum();
	double ymin = getHistogramMinAbvZero(hRatio);
	if (ymin < ymin_ratio) {
	  ymin_ratio = ymin;
	}
	if (ymax > ymax_ratio) {
	  ymax_ratio = ymax;
	}
	printf("Ratio: y(%f, %f) y-set(%f, %f)\n",ymin,ymax, ymin_ratio,ymax_ratio);
	//hRatio->GetYaxis()->SetRangeUser(0.8, 1.2);
	hRatios[histoTags.at(1)]->GetYaxis()->SetRangeUser(0.7*ymin_ratio, 1.3*ymax_ratio);
	
	if (iFile == 1) {
	  hRatios[histoTags.back()]->Draw();
	} else {
	  hRatios[histoTags.back()]->Draw("same");
	}


      }
      
      
      
      
      iFile++;
    }

    pad1.back()->cd();
    leg->SetBorderSize(0);
    leg->SetTextSize(0.04); 
    leg->Draw();
    
    tcanvases.back()->Print(Form("plots/%s_%s.png",sHisto.c_str(),sFileTags.c_str()));
    tcanvases.back()->Print(Form("plots/%s_%s.pdf",sHisto.c_str(),sFileTags.c_str()));
  }
}
