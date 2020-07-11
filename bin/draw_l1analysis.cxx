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
#include <boost/algorithm/string/replace.hpp>


TGraphAsymmErrors* DivideGraphAsymmErr(TGraphAsymmErrors *grN, TGraphAsymmErrors *grD) {
  if (grN->GetN() != grD->GetN()) {
    //printf("\nDivideGraphAsymmErr():: grN->GetN() %d != (grD->GetN() %d  *** ERROR *** \n terminating",grN->GetN(),grD->GetN());
    //exit(0);
  }
  
  int nPts = std::min(grN->GetN(), grD->GetN());
  double *x = new double[nPts];
  double *y = new double[nPts];
  for (int i=0; i<nPts; i++) {
    double xN,yN, xD,yD;
    if (grN->GetPoint(i, xN, yN) == -1 || grD->GetPoint(i, xD, yD)) {
      //printf("DivideGraphAsymmErr():: Error in reading graph point %d \t",i);
    }
    x[i] = xN;
    if (std::abs(yD - 0) > 1e-5) y[i] = yN / yD;
    else                         y[i] = 0.;
  }
  
  TGraphAsymmErrors *gr = new TGraphAsymmErrors(nPts, x, y);
  return gr;
}
 


int main()  
{
  // include comparisons between HW and data TPs
  bool includeHW = false;  
  //int rebinFactor = 1;
  bool compare_def_hw_only = true; // true: compare hw vs defalut scheme when includeHW = true
  
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

  //std::vector<std::string> puRangeNames = {"_lowPU", "_midPU", "_highPU", ""};
  //std::vector<std::string> puRangeNames = {"_PU30To46", "_PU47To83", "_PU64To80", "_PU50To100", ""};
  std::vector<std::string> puRangeNames = {""};

  //std::vector<std::vector<int> > puRanges = {{30,46},    {47,63},     {64,80},    {50,100},     {0,100}};
  std::vector<std::vector<int> > puRanges = {{0,100}};
    
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
    std::string histName = l1Type + std::string("_emu");
    std::string histNameHw(histName);
    boost::replace_all(histNameHw, "emu", "hw");
    std::cout << "histName: " << histName << " here1 " << std::endl;
    //  histName += "Effs_emu";
    //  histNameHw += "Effs_hw";

    TH2F* deftemp = dynamic_cast<TH2F*>(files.at(0)->Get(histName.c_str()));
    TH2F* newtemp = dynamic_cast<TH2F*>(files.at(1)->Get(histName.c_str()));
    TH2F* hwtemp;
    if (includeHW) hwtemp = dynamic_cast<TH2F*>(files.at(0)->Get(histNameHw.c_str()));
    //std::cout << "histName: " << histName << " here2 " << std::endl;
    for (unsigned int irange = 0; irange < puRanges.size(); irange++) {
      std::string custom = puRangeNames[irange]; std::vector<int> bounds = puRanges[irange];

      std::string newName = histName + custom;
      int lowBin = deftemp->GetYaxis()->FindBin(bounds[0]); int hiBin = deftemp->GetYaxis()->FindBin(bounds[1]);
      effHists_def[newName] = deftemp->ProjectionX((newName+"_def").c_str(), lowBin, hiBin, "");
      if (includeHW) effHists_hw[newName] = hwtemp->ProjectionX((newName+"_hw").c_str(), lowBin, hiBin, "");
      effHists_new_cond[newName] = newtemp->ProjectionX((newName+"_new").c_str(), lowBin, hiBin, "");
	
      effHists_def[newName]->Rebin(4);
      if (includeHW) effHists_hw[newName]->Rebin(4);
      effHists_new_cond[newName]->Rebin(4);
      effHists_def[newName]->SetLineColor(histColor[l1Type]);
      if (includeHW) effHists_hw[newName]->SetLineColor(histColor[l1Type]);
      effHists_new_cond[newName]->SetLineColor(histColor[l1Type]);
      TString name(effHists_new_cond[newName]->GetName());
      name += "_ratio";
      if(includeHW) {
	effHistsRatio[newName] = dynamic_cast<TH1D*>(effHists_def[newName]->Clone(name));
	effHistsRatio[newName]->Divide(effHists_hw[newName]);
      }
      else {
	effHistsRatio[newName] = dynamic_cast<TH1D*>(effHists_new_cond[newName]->Clone(name));
	effHistsRatio[newName]->Divide(effHists_def[newName]);
      }
      //effHistsRatio[newName]->SetMinimum(0.6);    
      //effHistsRatio[newName]->SetMaximum(1.4);
      effHistsRatio[newName]->SetMinimum(0.01);
      effHistsRatio[newName]->SetMaximum(2.0);
      effHistsRatio[newName]->SetLineWidth(2);
    }
  }
  for(auto pair : effHists_new_cond) pair.second->SetLineWidth(2);
  //for(auto pair : effHists_hw) pair.second->SetLineStyle(kDashed);
  for(auto pair : effHists_def) pair.second->SetLineStyle(kDotted);
  if (includeHW) {
    for(auto pair : effHists_hw) {
      if ( ! compare_def_hw_only) {
	pair.second->SetLineStyle(kDashed);
      } else {
	pair.second->SetLineWidth(2);
      }
    }     
  }
  
  
  // Efficiencies
  std::map<std::string, TH1D*> jetHists_def;
  std::map<std::string, TH1D*> jetHists_new_cond;
  std::map<std::string, TH1D*> jetHists_hw;  
  std::map<std::string, TH1D*> jetrefHists_def;
  std::map<std::string, TH1D*> jetrefHists_new_cond;
  std::map<std::string, TH1D*> jetrefHists_hw;
  std::map<std::string, TH1D*> metHists_PF_def;
  std::map<std::string, TH1D*> metHists_PF_new_cond;
  std::map<std::string, TH1D*> metHists_PF_hw;
  std::map<std::string, TH1D*> metHists_Calo_def;
  std::map<std::string, TH1D*> metHists_Calo_new_cond;
  std::map<std::string, TH1D*> metHists_Calo_hw;
  
  std::map<std::string, TGraphAsymmErrors*> jeteffHists_def;
  std::map<std::string, TGraphAsymmErrors*> jeteffHists_new_cond;
  std::map<std::string, TGraphAsymmErrors*> jeteffHists_hw;
  std::map<std::string, TGraphAsymmErrors*> meteffHists_PF_def;
  std::map<std::string, TGraphAsymmErrors*> meteffHists_PF_new_cond;
  std::map<std::string, TGraphAsymmErrors*> meteffHists_PF_hw;
  std::map<std::string, TGraphAsymmErrors*> meteffHists_Calo_def;
  std::map<std::string, TGraphAsymmErrors*> meteffHists_Calo_new_cond;
  std::map<std::string, TGraphAsymmErrors*> meteffHists_Calo_hw;
  std::map<std::string, TGraphAsymmErrors*> jeteffHistsRatio;
  std::map<std::string, TGraphAsymmErrors*> meteffHistsRatio;
  
  //-----------------------------------------------------------------------
  // L1 Jet efficiencies
  //-----------------------------------------------------------------------

  int rebinF=10;
  
  std::vector<TCanvas*> tcanvases;
  std::vector<TPad*> pad0;
  std::vector<TPad*> pad1;
  std::vector<TPad*> pad2;
  
  auto mSize = 0.8; auto lWidth = 2.; auto lMargin = 0.20; auto xOff = 1.14; auto yOff = 1.40;
  auto yMinS = -0.9; auto yMaxS = 0.9;
  auto yMinR = -0.05; auto yMaxR = 1.05;
  auto yResTitle = "#sigma#left(#frac{online - offline}{offline}#right)"; auto ySclTitle = "#mu#left(#frac{online - offline}{offline}#right)";
  auto metXTitle = "pfMET [GeV]"; auto jetXTitle = "Offline Jet E_{T} [GeV]";

  gStyle->SetEndErrorSize(0.0);

  for (auto& [region, histos] : jetTypes) {

    std::string refName = "RefmJet_"; refName += region + std::string("_emu");
    std::string refNameHw = refName;
    boost::replace_all(refNameHw, "emu", "hw");
	
    TH2F* deftemp = dynamic_cast<TH2F*>(files.at(0)->Get(refName.c_str()));
    TH2F* newtemp = dynamic_cast<TH2F*>(files.at(1)->Get(refName.c_str()));
    TH2F* hwtemp;
    if (includeHW) hwtemp = dynamic_cast<TH2F*>(files.at(0)->Get(refNameHw.c_str()));
    for (unsigned int irange = 0; irange < puRanges.size(); irange++) {

      std::string custom = puRangeNames[irange]; std::vector<int> bounds = puRanges[irange];

      std::string newName = refName + custom;
      int lowBin = deftemp->GetYaxis()->FindBin(bounds[0]); int hiBin = deftemp->GetYaxis()->FindBin(bounds[1]);

      jetrefHists_def[newName] = deftemp->ProjectionX((newName+"_def").c_str(), lowBin, hiBin, ""); jetrefHists_def[newName]->Rebin(rebinF);
      jetrefHists_new_cond[newName] = newtemp->ProjectionX((newName+"_new").c_str(), lowBin, hiBin, ""); jetrefHists_new_cond[newName]->Rebin(rebinF);
      if (includeHW) { jetrefHists_hw[newName] = hwtemp->ProjectionX((newName+"_hw").c_str(), lowBin, hiBin, ""); jetrefHists_hw[newName]->Rebin(rebinF); }

      for (auto& jetType : histos) {
	TLegend* jetLegend = new TLegend(0.71, 0.3, 0.91, 0.45);

	std::string histName;
	if (region == "Incl") { histName = jetType; }
	else { histName = std::string(jetType) + std::string("_") + std::string(region); }
	histName += std::string("_emu");
	std::string histNameHw(histName);
	boost::replace_all(histNameHw, "emu", "hw");
	       
	std::string newName2 = histName + custom;

	tcanvases.push_back(new TCanvas);
	//tcanvases.back()->SetWindowSize(tcanvases.back()->GetWw(), 1.*tcanvases.back()->GetWh());
	//gPad->SetGridx(); gPad->SetGridy();
	tcanvases.back()->SetWindowSize(tcanvases.back()->GetWw(), 1.3*tcanvases.back()->GetWh());
	pad1.push_back(new TPad("pad1", "pad1", 0, 0.3, 1, 1));
	pad1.back()->SetGrid();
	pad1.back()->Draw();
	pad2.push_back(new TPad("pad2", "pad2", 0, 0, 1, 0.3));
	pad2.back()->SetGrid();
	pad2.back()->Draw();
	       
	pad1.back()->cd();
	pad1.back()->SetBottomMargin(0.02); 
	       
      
	TH2F* deftemp2 = dynamic_cast<TH2F*>(files.at(0)->Get(histName.c_str()));
	TH2F* newtemp2 = dynamic_cast<TH2F*>(files.at(1)->Get(histName.c_str()));
	TH2F* hwtemp2;
	if (includeHW) hwtemp2 = dynamic_cast<TH2F*>(files.at(0)->Get(histNameHw.c_str()));
      
	lowBin = deftemp->GetYaxis()->FindBin(bounds[0]); hiBin = deftemp->GetYaxis()->FindBin(bounds[1]);

	jetHists_def[newName2] = deftemp2->ProjectionX((newName2+"_def").c_str(), lowBin, hiBin, ""); jetHists_def[newName2]->Rebin(rebinF);
	jetHists_new_cond[newName2] = newtemp2->ProjectionX((newName2+"_new").c_str(), lowBin, hiBin, ""); jetHists_new_cond[newName2]->Rebin(rebinF);
	if (includeHW) { jetHists_hw[newName2] = hwtemp2->ProjectionX((newName2+"_hw").c_str(), lowBin, hiBin, ""); jetHists_hw[newName2]->Rebin(rebinF); }

	TGraphAsymmErrors *Eff_def      = new TGraphAsymmErrors();
	TGraphAsymmErrors *Eff_new_cond = new TGraphAsymmErrors();
	TGraphAsymmErrors *Eff_hw       = new TGraphAsymmErrors();
	       
	Eff_def->BayesDivide(jetHists_def[newName2],jetrefHists_def[newName]);
	Eff_new_cond->BayesDivide(jetHists_new_cond[newName2],jetrefHists_new_cond[newName]);
	if (includeHW) Eff_hw->BayesDivide(jetHists_hw[newName2],jetrefHists_hw[newName]);
		 
	jeteffHists_def[newName2]      = Eff_def;
	jeteffHists_new_cond[newName2] = Eff_new_cond;
	if (includeHW) jeteffHists_hw[newName2]       = Eff_hw;
	      
	jeteffHists_def[newName2]->GetYaxis()->SetRangeUser(yMinR, yMaxR);
	jeteffHists_new_cond[newName2]->GetYaxis()->SetRangeUser(yMinR, yMaxR);

	jeteffHists_def[newName2]->SetMarkerColor(kBlack);
	jeteffHists_new_cond[newName2]->SetMarkerColor(kRed);
	if (includeHW) jeteffHists_hw[newName2]->SetMarkerColor(kBlue);

	jeteffHists_def[newName2]->SetLineColor(kBlack);
	jeteffHists_new_cond[newName2]->SetLineColor(kRed);
	if (includeHW) jeteffHists_hw[newName2]->SetLineColor(kBlue);
	
	jeteffHists_def[newName2]->SetMarkerSize(mSize);
	jeteffHists_new_cond[newName2]->SetMarkerSize(mSize);
	if (includeHW) jeteffHists_hw[newName2]->SetMarkerSize(mSize);
	
	jeteffHists_def[newName2]->SetLineWidth(lWidth);
	jeteffHists_new_cond[newName2]->SetLineWidth(lWidth);
	if (includeHW) jeteffHists_hw[newName2]->SetLineWidth(lWidth);
	
	jetLegend->AddEntry(jeteffHists_def[newName2], "Default", "EP");
	if (includeHW) jetLegend->AddEntry(jeteffHists_hw[newName2], "Hardware", "EP");
	if ( ! includeHW || (includeHW && ! compare_def_hw_only))
	  jetLegend->AddEntry(jeteffHists_new_cond[newName2], "New", "EP");
	       
	jeteffHists_def[newName2]->GetXaxis()->SetLabelSize(0.0);

	TH1D *hRatioDummy = new TH1D(Form("%s_RatioDummy",jeteffHists_def[newName2]->GetName()), "",
				     jetHists_def[newName2]->GetNbinsX(),
				     jeteffHists_def[newName2]->GetXaxis()->GetXmin(),
				     jeteffHists_def[newName2]->GetXaxis()->GetXmax());
	hRatioDummy->GetYaxis()->SetRangeUser(yMinR, yMaxR);
	hRatioDummy->GetXaxis()->SetLabelSize(0.0);
	hRatioDummy->GetYaxis()->SetTitle("Efficiency");
	hRatioDummy->Draw();	       

	//jeteffHists_def[newName2]->Draw("AP");
	jeteffHists_def[newName2]->Draw("P");
	if (includeHW) jeteffHists_hw[newName2]->Draw("P");
	if ( ! includeHW || (includeHW && ! compare_def_hw_only)) jeteffHists_new_cond[newName2]->Draw("P");
	jeteffHists_def[newName2]->SetTitle("");
	jeteffHists_new_cond[newName2]->SetTitle("");
	jeteffHists_def[newName2]->GetXaxis()->SetTitle(jetXTitle);
	jeteffHists_def[newName2]->GetXaxis()->SetTitleOffset(1.17*jeteffHists_def[newName2]->GetXaxis()->GetTitleOffset());
	jetLegend->Draw("SAME");
	jeteffHists_def[newName2]->GetYaxis()->SetTitle("Efficiency");


	pad2.back()->cd();
	pad2.back()->SetTopMargin(0.05);
	pad2.back()->SetBottomMargin(0.3);

	if (includeHW) {
	  jeteffHistsRatio[newName2] = dynamic_cast<TGraphAsymmErrors*>(jeteffHists_def[newName2]->Clone(newName2.c_str()));
	  jeteffHistsRatio[newName2] = DivideGraphAsymmErr(Eff_def, Eff_hw);
	} else {
	  jeteffHistsRatio[newName2] = dynamic_cast<TGraphAsymmErrors*>(jeteffHists_new_cond[newName2]->Clone(newName2.c_str()));
	  jeteffHistsRatio[newName2] = DivideGraphAsymmErr(Eff_new_cond, Eff_def);
	}
	
	auto xLabSize = jeteffHistsRatio[newName2]->GetXaxis()->GetLabelSize();
	auto xTitleSize = jeteffHistsRatio[newName2]->GetXaxis()->GetTitleSize();
	auto yLabSize = jeteffHistsRatio[newName2]->GetYaxis()->GetLabelSize();
	auto yTitleSize = jeteffHistsRatio[newName2]->GetYaxis()->GetTitleSize();
	auto yOffSet = jeteffHistsRatio[newName2]->GetYaxis()->GetTitleOffset();
	/*
	  jeteffHistsRatio[newName2]->GetXaxis()->SetTitleSize(0.7*xTitleSize/0.3);
	  jeteffHistsRatio[newName2]->GetXaxis()->SetLabelSize(0.7*xLabSize/0.3);
	       
	  jeteffHistsRatio[newName2]->GetYaxis()->SetTitleSize(0.7*yTitleSize/0.3);
	  jeteffHistsRatio[newName2]->GetYaxis()->SetLabelSize(0.7*yLabSize/0.3);
	       
	  jeteffHistsRatio[newName2]->GetYaxis()->SetTitleOffset(0.3*yOffSet/0.7);
	       
	  jeteffHistsRatio[newName2]->GetYaxis()->SetNdivisions(505);

	  jeteffHistsRatio[newName2]->GetXaxis()->SetTitle(jetXTitle);
	  jeteffHistsRatio[newName2]->SetTitle("");
	       
	  //jeteffHistsRatio[newName2]->GetYaxis()->SetRangeUser(0.01, 2);
	  jeteffHistsRatio[newName2]->SetMinimum(0.4);   
	  jeteffHistsRatio[newName2]->SetMaximum(1.6); */	       

	TH1D *hRatioDummy1 = new TH1D(Form("%s_RatioDummy1",jeteffHists_def[newName2]->GetName()), "",
				      jetHists_def[newName2]->GetNbinsX(),
				      jeteffHists_def[newName2]->GetXaxis()->GetXmin(),
				      jeteffHists_def[newName2]->GetXaxis()->GetXmax());
	hRatioDummy1->GetYaxis()->SetRangeUser(0.4, 1.6);
	hRatioDummy1->GetXaxis()->SetTitle(jetXTitle);
	if (includeHW) hRatioDummy1->GetYaxis()->SetTitle("Default / Hardware");
	else           hRatioDummy1->GetYaxis()->SetTitle("New / Default");
	       	       
	hRatioDummy1->GetXaxis()->SetTitleSize(0.7*xTitleSize/0.3);
	hRatioDummy1->GetXaxis()->SetLabelSize(0.7*xLabSize/0.3);
	       
	hRatioDummy1->GetYaxis()->SetTitleSize(0.7*yTitleSize/0.3);
	hRatioDummy1->GetYaxis()->SetLabelSize(0.7*yLabSize/0.3);
	       
	hRatioDummy1->GetYaxis()->SetTitleOffset(0.3*yOffSet/0.7);
	       
	hRatioDummy1->GetYaxis()->SetNdivisions(505);
	       
	hRatioDummy1->Draw();

	jeteffHistsRatio[newName2]->Draw("P same");

	if(includeHW) {
	  tcanvases.back()->Print(Form("plots/%sjetEffs_hw_%s%s_lin.png", jetType.c_str(), region.c_str(), custom.c_str()));
	  tcanvases.back()->Print(Form("plots/%sjetEffs_hw_%s%s_lin.pdf", jetType.c_str(), region.c_str(), custom.c_str()));
	} else {
	  tcanvases.back()->Print(Form("plots/%sjetEffs_emu_%s%s_lin.png", jetType.c_str(), region.c_str(), custom.c_str()));
	  tcanvases.back()->Print(Form("plots/%sjetEffs_emu_%s%s_lin.pdf", jetType.c_str(), region.c_str(), custom.c_str()));
	}
	//tcanvases.back()->SetLogx();
	pad1.back()->SetLogx();
	pad2.back()->SetLogx();
	if(includeHW) {
	  tcanvases.back()->Print(Form("plots/%sjetEffs_hw_%s%s_log.png", jetType.c_str(), region.c_str(), custom.c_str()));
	  tcanvases.back()->Print(Form("plots/%sjetEffs_hw_%s%s_log.pdf", jetType.c_str(), region.c_str(), custom.c_str()));
	} else {
	  tcanvases.back()->Print(Form("plots/%sjetEffs_emu_%s%s_log.png", jetType.c_str(), region.c_str(), custom.c_str()));
	  tcanvases.back()->Print(Form("plots/%sjetEffs_emu_%s%s_log.pdf", jetType.c_str(), region.c_str(), custom.c_str()));
	}
	
	
      }
    }
  }
  std::cout << "Now L1 ETM efficiencies" << std::endl;

  //-----------------------------------------------------------------------
  // L1 ETM efficiencies
  //-----------------------------------------------------------------------

  rebinF=10;

  TH2F* deftemp_PF = dynamic_cast<TH2F*>(files.at(0)->Get("RefMET_PF_emu"));
  TH2F* newtemp_PF = dynamic_cast<TH2F*>(files.at(1)->Get("RefMET_PF_emu"));
  TH2F* hwtemp_PF;
  if(includeHW) hwtemp_PF = dynamic_cast<TH2F*>(files.at(0)->Get("RefMET_PF_hw"));
  
  for (unsigned int irange = 0; irange < puRanges.size(); irange++) {

    std::string custom = puRangeNames[irange]; std::vector<int> bounds = puRanges[irange];
    int lowBin = deftemp_PF->GetYaxis()->FindBin(bounds[0]); int hiBin = deftemp_PF->GetYaxis()->FindBin(bounds[1]);
    std::cout << "RefMET_PF_def " << custom << "  here1" << std::endl;
    TH1D* metrefHists_PF_def = deftemp_PF->ProjectionX("RefMET_PF_def", lowBin, hiBin, ""); metrefHists_PF_def->Rebin(rebinF);
    std::cout << "RefMET_PF_def " << custom << "  here2" << std::endl;
    TH1D* metrefHists_PF_new_cond = newtemp_PF->ProjectionX("RefMET_PF_new_cond", lowBin, hiBin, ""); metrefHists_PF_new_cond->Rebin(rebinF);
    std::cout << "RefMET_PF_def " << custom << "  here3" << std::endl;
    TH1D* metrefHists_PF_hw;
    if(includeHW) { metrefHists_PF_hw = (TH1D*)hwtemp_PF->ProjectionX("RefMET_PF_hw_tmp", lowBin, hiBin, "");
       std::cout << "RefMET_PF_def " << custom << "  here3_1" << std::endl;
      metrefHists_PF_hw->Rebin(rebinF); }
    std::cout << "RefMET_PF_def " << custom << "  here4" << std::endl;
    
    for (auto metType : sumTypesPF) {

      TLegend* metLegend = new TLegend(0.67, 0.17, 0.87, 0.33);

      tcanvases.push_back(new TCanvas);
      //tcanvases.back()->SetWindowSize(tcanvases.back()->GetWw(), 1.*tcanvases.back()->GetWh());
      //gPad->SetGridx(); gPad->SetGridy();
      tcanvases.back()->SetWindowSize(tcanvases.back()->GetWw(), 1.3*tcanvases.back()->GetWh());
      pad1.push_back(new TPad("pad1", "pad1", 0, 0.3, 1, 1));
      pad1.back()->SetGrid();
      pad1.back()->Draw();
      pad2.push_back(new TPad("pad2", "pad2", 0, 0, 1, 0.3));
      pad2.back()->SetGrid();
      pad2.back()->Draw();
	       
      pad1.back()->cd();
      pad1.back()->SetBottomMargin(0.02); 
	  
      std::string histName = metType + std::string("_emu");
      std::string histNameHw(histName);
      boost::replace_all(histNameHw, "emu", "hw");
      
      std::string newName = histName + custom;

      TH2F* deftemp2_PF = dynamic_cast<TH2F*>(files.at(0)->Get(histName.c_str()));
      TH2F* newtemp2_PF = dynamic_cast<TH2F*>(files.at(1)->Get(histName.c_str()));
      TH2F* hwtemp2_PF;
      if (includeHW) hwtemp2_PF = dynamic_cast<TH2F*>(files.at(0)->Get(histNameHw.c_str()));
	
      metHists_PF_def[newName] = deftemp2_PF->ProjectionX((newName+"_PF_def").c_str(), lowBin, hiBin, ""); metHists_PF_def[newName]->Rebin(rebinF);
      metHists_PF_new_cond[newName] = newtemp2_PF->ProjectionX((newName+"_PF_new").c_str(), lowBin, hiBin, ""); metHists_PF_new_cond[newName]->Rebin(rebinF);
      if (includeHW) { metHists_PF_hw[newName] = hwtemp2_PF->ProjectionX((newName+"_PF_hw").c_str(), lowBin, hiBin, ""); metHists_PF_hw[newName]->Rebin(rebinF); }
      
      TGraphAsymmErrors *Eff_def      = new TGraphAsymmErrors();
      TGraphAsymmErrors *Eff_new_cond = new TGraphAsymmErrors();
      TGraphAsymmErrors *Eff_hw       = new TGraphAsymmErrors();
      
      Eff_def->BayesDivide(metHists_PF_def[newName],metrefHists_PF_def);
      Eff_new_cond->BayesDivide(metHists_PF_new_cond[newName],metrefHists_PF_new_cond);
      if (includeHW) Eff_hw->BayesDivide(metHists_PF_hw[newName],metrefHists_PF_hw);
	
      meteffHists_PF_def[newName] = Eff_def;
      meteffHists_PF_new_cond[newName] = Eff_new_cond;
      if (includeHW) meteffHists_PF_hw[newName] = Eff_hw;
      
      meteffHists_PF_def[newName]->GetYaxis()->SetRangeUser(yMinR,yMaxR);
      meteffHists_PF_new_cond[newName]->GetYaxis()->SetRangeUser(yMinR,yMaxR);

      meteffHists_PF_def[newName]->SetMarkerColor(kBlack);
      meteffHists_PF_new_cond[newName]->SetMarkerColor(kRed);
      if (includeHW) meteffHists_PF_hw[newName]->SetMarkerColor(kBlue);
      
      meteffHists_PF_def[newName]->SetLineColor(kBlack);
      meteffHists_PF_new_cond[newName]->SetLineColor(kRed);
      if (includeHW) meteffHists_PF_hw[newName]->SetLineColor(kBlue);
      
      meteffHists_PF_def[newName]->SetLineWidth(lWidth);
      meteffHists_PF_new_cond[newName]->SetLineWidth(lWidth);
      if (includeHW) meteffHists_PF_hw[newName]->SetLineWidth(lWidth);
      
      meteffHists_PF_def[newName]->SetMarkerSize(mSize);
      meteffHists_PF_new_cond[newName]->SetMarkerSize(mSize);
      if (includeHW) meteffHists_PF_hw[newName]->SetMarkerSize(mSize);

      metLegend->AddEntry(meteffHists_PF_def[newName], "Default", "EP");
      if (includeHW) metLegend->AddEntry(meteffHists_PF_hw[newName], "Hardware", "EP");
      else           metLegend->AddEntry(meteffHists_PF_new_cond[newName], "New", "EP");
      

      meteffHists_PF_def[newName]->GetXaxis()->SetLabelSize(0.0);

      TH1D *hRatioDummy = new TH1D(Form("%s_RaioDummy",metHists_PF_def[newName]->GetName()), "",
				   metHists_PF_def[newName]->GetNbinsX(),
				   metHists_PF_def[newName]->GetXaxis()->GetXmin(),
				   metHists_PF_def[newName]->GetXaxis()->GetXmax());
      hRatioDummy->GetYaxis()->SetRangeUser(yMinR, yMaxR);
      hRatioDummy->GetXaxis()->SetLabelSize(0.0);
      hRatioDummy->GetYaxis()->SetTitle("Efficiency");
      hRatioDummy->Draw();

      meteffHists_PF_def[newName]->Draw("P");
      if (includeHW) meteffHists_PF_hw[newName]->Draw("P");
      if ( ! includeHW || (includeHW && ! compare_def_hw_only)) meteffHists_PF_new_cond[newName]->Draw("P");
      meteffHists_PF_def[newName]->SetTitle("");
      meteffHists_PF_new_cond[newName]->SetTitle("");
      meteffHists_PF_def[newName]->GetXaxis()->SetTitle(metXTitle);
      meteffHists_PF_def[newName]->GetXaxis()->SetTitleOffset(1.17*meteffHists_PF_def[newName]->GetXaxis()->GetTitleOffset());
      metLegend->Draw("SAME");

      meteffHists_PF_def[newName]->GetYaxis()->SetTitle("Efficiency");


      pad2.back()->cd();
      pad2.back()->SetTopMargin(0.05);
      pad2.back()->SetBottomMargin(0.3);
      if (includeHW) {
	meteffHistsRatio[newName] = dynamic_cast<TGraphAsymmErrors*>(meteffHists_PF_def[newName]->Clone(newName.c_str()));
	meteffHistsRatio[newName] = DivideGraphAsymmErr(Eff_def, Eff_hw);
      } else {
	meteffHistsRatio[newName] = dynamic_cast<TGraphAsymmErrors*>(meteffHists_PF_new_cond[newName]->Clone(newName.c_str()));
	meteffHistsRatio[newName] = DivideGraphAsymmErr(Eff_new_cond, Eff_def);
      }


      auto xLabSize = meteffHistsRatio[newName]->GetXaxis()->GetLabelSize();
      auto xTitleSize = meteffHistsRatio[newName]->GetXaxis()->GetTitleSize();
      auto yLabSize = meteffHistsRatio[newName]->GetYaxis()->GetLabelSize();
      auto yTitleSize = meteffHistsRatio[newName]->GetYaxis()->GetTitleSize();
      auto yOffSet = meteffHistsRatio[newName]->GetYaxis()->GetTitleOffset();
      /*
	meteffHistsRatio[newName]->GetXaxis()->SetTitleSize(0.7*xTitleSize/0.3);
	meteffHistsRatio[newName]->GetXaxis()->SetLabelSize(0.7*xLabSize/0.3);
	       
	meteffHistsRatio[newName]->GetYaxis()->SetTitleSize(0.7*yTitleSize/0.3);
	meteffHistsRatio[newName]->GetYaxis()->SetLabelSize(0.7*yLabSize/0.3);
	       
	meteffHistsRatio[newName]->GetYaxis()->SetTitleOffset(0.3*yOffSet/0.7);
	       
	meteffHistsRatio[newName]->GetYaxis()->SetNdivisions(505);
 
	meteffHistsRatio[newName]->GetXaxis()->SetTitle(jetXTitle);
	meteffHistsRatio[newName]->SetTitle("");
	  
	//jeteffHistsRatio[newName2]->GetYaxis()->SetRangeUser(0.01, 2);
	meteffHistsRatio[newName]->SetMinimum(0.4);   
	meteffHistsRatio[newName]->SetMaximum(1.6);*/

      TH1D *hRatioDummy1 = new TH1D(Form("%s_RaioDummy1",metHists_PF_def[newName]->GetName()), "",
				    metHists_PF_def[newName]->GetNbinsX(),
				    metHists_PF_def[newName]->GetXaxis()->GetXmin(),
				    metHists_PF_def[newName]->GetXaxis()->GetXmax());
      hRatioDummy1->GetYaxis()->SetRangeUser(0.4, 1.6);
      hRatioDummy1->GetXaxis()->SetTitle(metXTitle);
      if (includeHW) hRatioDummy1->GetYaxis()->SetTitle("Default / Hardware");
      else           hRatioDummy1->GetYaxis()->SetTitle("New / Default");
	       	       
      hRatioDummy1->GetXaxis()->SetTitleSize(0.7*xTitleSize/0.3);
      hRatioDummy1->GetXaxis()->SetLabelSize(0.7*xLabSize/0.3);
	       
      hRatioDummy1->GetYaxis()->SetTitleSize(0.7*yTitleSize/0.3);
      hRatioDummy1->GetYaxis()->SetLabelSize(0.7*yLabSize/0.3);
	       
      hRatioDummy1->GetYaxis()->SetTitleOffset(0.3*yOffSet/0.7);
	       
      hRatioDummy1->GetYaxis()->SetNdivisions(505);
	       
      hRatioDummy1->Draw();
	  
      meteffHistsRatio[newName]->Draw("P");
      meteffHistsRatio[newName]->GetYaxis()->SetTitle("New / Default");

	  
      if (includeHW) {
	tcanvases.back()->Print(Form("plots/%smetEffs%s_hw_lin.png", metType.c_str(), custom.c_str()));
	tcanvases.back()->Print(Form("plots/%smetEffs%s_hw_lin.pdf", metType.c_str(), custom.c_str()));
      } else {
	tcanvases.back()->Print(Form("plots/%smetEffs%s_emu_lin.png", metType.c_str(), custom.c_str()));
	tcanvases.back()->Print(Form("plots/%smetEffs%s_emu_lin.pdf", metType.c_str(), custom.c_str()));
      }
      //tcanvases.back()->SetLogx();
      pad1.back()->SetLogx();
      pad2.back()->SetLogx();
      if (includeHW) {
	tcanvases.back()->Print(Form("plots/%smetEffs%s_hw_log.png", metType.c_str(), custom.c_str()));
	tcanvases.back()->Print(Form("plots/%smetEffs%s_hw_log.pdf", metType.c_str(), custom.c_str()));
      } else {
	tcanvases.back()->Print(Form("plots/%smetEffs%s_emu_log.png", metType.c_str(), custom.c_str()));
	tcanvases.back()->Print(Form("plots/%smetEffs%s_emu_log.pdf", metType.c_str(), custom.c_str()));
      }
    }

  }
   
  TH2F* deftemp_Calo = dynamic_cast<TH2F*>(files.at(0)->Get("RefMET_Calo_emu"));
  TH2F* newtemp_Calo = dynamic_cast<TH2F*>(files.at(1)->Get("RefMET_Calo_emu"));
  TH2F* hwtemp_Calo;
  if (includeHW) hwtemp_Calo = dynamic_cast<TH2F*>(files.at(0)->Get("RefMET_Calo_hw"));
  
  for (unsigned int irange = 0; irange < puRanges.size(); irange++) {

    std::string custom = puRangeNames[irange]; std::vector<int> bounds = puRanges[irange];
    int lowBin = deftemp_Calo->GetYaxis()->FindBin(bounds[0]); int hiBin = deftemp_Calo->GetYaxis()->FindBin(bounds[1]);

    TH1D* metrefHists_Calo_def = deftemp_Calo->ProjectionX("RefMET_Calo_def", lowBin, hiBin, ""); metrefHists_Calo_def->Rebin(rebinF);
    TH1D* metrefHists_Calo_new_cond = newtemp_Calo->ProjectionX("RefMET_Calo_new_cond", lowBin, hiBin, ""); metrefHists_Calo_new_cond->Rebin(rebinF);
    TH1D* metrefHists_Calo_hw;
    if (includeHW) { metrefHists_Calo_hw = hwtemp_Calo->ProjectionX("RefMET_Calo_hw_tmp", lowBin, hiBin, ""); metrefHists_Calo_hw->Rebin(rebinF); }
      
    for (auto metType : sumTypesCalo) {

      TLegend* metLegend = new TLegend(0.67, 0.17, 0.87, 0.33);

      tcanvases.push_back(new TCanvas);
      tcanvases.back()->SetWindowSize(tcanvases.back()->GetWw(), 1.*tcanvases.back()->GetWh());
      gPad->SetGridx(); gPad->SetGridy();
      std::string histName = metType + std::string("_emu");
      std::string histNameHw(histName);
      boost::replace_all(histNameHw, "emu", "hw");
      
      std::string newName = histName + custom;

      TH2F* deftemp2 = dynamic_cast<TH2F*>(files.at(0)->Get(histName.c_str()));
      TH2F* newtemp2 = dynamic_cast<TH2F*>(files.at(1)->Get(histName.c_str()));
      TH2F* hwtemp2;
      if (includeHW) hwtemp2 = dynamic_cast<TH2F*>(files.at(0)->Get(histNameHw.c_str()));
      
      metHists_Calo_def[newName] = deftemp2->ProjectionX((newName+"_Calo_def").c_str(), lowBin, hiBin, ""); metHists_Calo_def[newName]->Rebin(rebinF);
      metHists_Calo_new_cond[newName] = newtemp2->ProjectionX((newName+"_Calo_new").c_str(), lowBin, hiBin, ""); metHists_Calo_new_cond[newName]->Rebin(rebinF);
      if (includeHW) { metHists_Calo_hw[newName] = hwtemp2->ProjectionX((newName+"_Calo_hw").c_str(), lowBin, hiBin, ""); metHists_Calo_hw[newName]->Rebin(rebinF); }
      
      TGraphAsymmErrors *Eff_def      = new TGraphAsymmErrors();
      TGraphAsymmErrors *Eff_new_cond = new TGraphAsymmErrors();
      TGraphAsymmErrors *Eff_hw       = new TGraphAsymmErrors();
      
      Eff_def->BayesDivide(metHists_Calo_def[newName],metrefHists_Calo_def);
      Eff_new_cond->BayesDivide(metHists_Calo_new_cond[newName],metrefHists_Calo_new_cond);
      if (includeHW) Eff_hw->BayesDivide(metHists_Calo_hw[newName],metrefHists_Calo_hw);
	
      meteffHists_Calo_def[newName] = Eff_def;
      meteffHists_Calo_new_cond[newName] = Eff_new_cond;
      if (includeHW) meteffHists_Calo_hw[newName] = Eff_hw;
      
      meteffHists_Calo_def[newName]->GetYaxis()->SetRangeUser(yMinR,yMaxR);
      meteffHists_Calo_new_cond[newName]->GetYaxis()->SetRangeUser(yMinR,yMaxR);

      meteffHists_Calo_def[newName]->SetMarkerColor(kBlack);
      meteffHists_Calo_new_cond[newName]->SetMarkerColor(kRed);
      if (includeHW) meteffHists_Calo_hw[newName]->SetMarkerColor(kBlue);
		       
      meteffHists_Calo_def[newName]->SetLineColor(kBlack);
      meteffHists_Calo_new_cond[newName]->SetLineColor(kRed);
      if (includeHW) meteffHists_Calo_hw[newName]->SetLineColor(kBlue);
		       
      meteffHists_Calo_def[newName]->SetLineWidth(lWidth);
      meteffHists_Calo_new_cond[newName]->SetLineWidth(lWidth);
      if (includeHW) meteffHists_Calo_hw[newName]->SetLineWidth(lWidth);
		       
      meteffHists_Calo_def[newName]->SetMarkerSize(mSize);
      meteffHists_Calo_new_cond[newName]->SetMarkerSize(mSize);
      if (includeHW) meteffHists_Calo_hw[newName]->SetMarkerSize(mSize);
		       
      metLegend->AddEntry(meteffHists_Calo_def[newName], "Default", "EP");
      if (includeHW) metLegend->AddEntry(meteffHists_Calo_hw[newName], "Hardware", "EP");
      else           metLegend->AddEntry(meteffHists_Calo_new_cond[newName], "New", "EP");
      
      meteffHists_Calo_def[newName]->Draw("AP");
      if (includeHW) meteffHists_Calo_hw[newName]->Draw("AP");
      if ( ! includeHW || (includeHW && ! compare_def_hw_only)) meteffHists_Calo_new_cond[newName]->Draw("P");
      meteffHists_Calo_def[newName]->SetTitle("");
      meteffHists_Calo_new_cond[newName]->SetTitle("");
      meteffHists_Calo_def[newName]->GetXaxis()->SetTitle(metXTitle);
      meteffHists_Calo_def[newName]->GetXaxis()->SetTitleOffset(1.17*meteffHists_Calo_def[newName]->GetXaxis()->GetTitleOffset());
      metLegend->Draw("SAME");

      meteffHists_Calo_def[newName]->GetYaxis()->SetTitle("Efficiency");

      if (includeHW) {
	tcanvases.back()->Print(Form("plots/%smetEffs%s_Calo_hw_lin.png", metType.c_str(), custom.c_str()));
	tcanvases.back()->Print(Form("plots/%smetEffs%s_Calo_hw_lin.pdf", metType.c_str(), custom.c_str()));
      } else {
	tcanvases.back()->Print(Form("plots/%smetEffs%s_Calo_emu_lin.png", metType.c_str(), custom.c_str()));
	tcanvases.back()->Print(Form("plots/%smetEffs%s_Calo_emu_lin.pdf", metType.c_str(), custom.c_str()));
      }
      tcanvases.back()->SetLogx();
      if (includeHW) {
	tcanvases.back()->Print(Form("plots/%smetEffs%s_Calo_hw_log.png", metType.c_str(), custom.c_str()));
	tcanvases.back()->Print(Form("plots/%smetEffs%s_Calo_hw_log.pdf", metType.c_str(), custom.c_str()));
      } else {
	tcanvases.back()->Print(Form("plots/%smetEffs%s_Calo_emu_log.png", metType.c_str(), custom.c_str()));
	tcanvases.back()->Print(Form("plots/%smetEffs%s_Calo_emu_log.pdf", metType.c_str(), custom.c_str()));
      }
    }

  }

  //-----------------------------------------------------------------------
  // L1 Jet resolution summary plots
  //-----------------------------------------------------------------------
  std::cout << "L1 Jet resolution summary plots " << std::endl;
  TF1 *fgaus = new TF1("g1","gaus");//,-2.,2.);
  fgaus->SetRange(-1.,1.);
  
  // // Jet resolution
  std::vector<TCanvas*> mycanvases_1;   std::vector<TCanvas*> mycanvases_2;
  std::vector<TH2D*> resHistos2D_def;   std::vector<TH2D*> resHistos2D_new_cond;
  std::vector<TH1D*> resHistos1D_1_def; std::vector<TH1D*> resHistos1D_1_new_cond;
  std::vector<TH1D*> resHistos1D_2_def; std::vector<TH1D*> resHistos1D_2_new_cond;
  std::vector<TH2D*> resHistos2D_hw;
  std::vector<TH1D*> resHistos1D_1_hw;
  std::vector<TH1D*> resHistos1D_2_hw;
  std::vector<std::string> regions = {"Incl", "HE", "HB", "HE2", "HE1"}; 

  for (auto& region : regions) {

    std::string histo2Dname = "hresJet_" + region + "_emu";
    std::string histo2DnameHw(histo2Dname);
    boost::replace_all(histo2DnameHw, "emu", "hw");
    std::string histo1Dname_1 = histo2Dname + "_1"; std::string histo1Dname_yx_1 = histo2Dname + "_yx_1";
    std::string histo1Dname_2 = histo2Dname + "_2"; std::string histo1Dname_yx_2 = histo2Dname + "_yx_2";

    std::string histo1Dname_1_new_cond = histo1Dname_1 + "_" + region + "_new_cond";
    std::string histo1Dname_2_new_cond = histo1Dname_2 + "_" + region + "_new_cond";

    std::string histo1Dname_1_def = histo1Dname_1 + "_" + region + "_def";
    std::string histo1Dname_2_def = histo1Dname_2 + "_" + region + "_def";
    //
    std::string histo1DnameHw_1 = histo2DnameHw + "_1"; std::string histo1DnameHw_yx_1 = histo2DnameHw + "_yx_1";
    std::string histo1DnameHw_2 = histo2DnameHw + "_2"; std::string histo1DnameHw_yx_2 = histo2DnameHw + "_yx_2";
    
    std::string histo1Dname_1_hw = histo1DnameHw_1 + "_" + region + "_hw";
    std::string histo1Dname_2_hw = histo1DnameHw_2 + "_" + region + "_hw";    

    std::string meanName = "resJet_mean_" + region;
    std::string sigmaName = "resJet_sigma_" + region;

    TH3F* deftemp = dynamic_cast<TH3F*>(files.at(0)->Get(histo2Dname.c_str()));
    TH3F* newtemp = dynamic_cast<TH3F*>(files.at(1)->Get(histo2Dname.c_str()));
    TH3F* hwtemp;
    if (includeHW) hwtemp = dynamic_cast<TH3F*>(files.at(0)->Get(histo2DnameHw.c_str()));

    std::cout << "histo2Dname: " << histo2Dname << ", histo2DnameHw: " << histo2DnameHw << std::endl;
    for (unsigned int irange = 0; irange < puRanges.size(); irange++) {

      std::string custom = puRangeNames[irange]; std::vector<int> bounds = puRanges[irange];
      int lowBin = deftemp->GetZaxis()->FindBin(bounds[0]); int hiBin = deftemp->GetZaxis()->FindBin(bounds[1]);
      std::cout << "histo2Dname: " << histo2Dname << ", custom: " << custom << "  here1" << std::endl;
      
      deftemp->GetZaxis()->SetRange(lowBin, hiBin); 
      newtemp->GetZaxis()->SetRange(lowBin, hiBin);
      if (includeHW) hwtemp->GetZaxis()->SetRange(lowBin, hiBin);
      std::cout << "histo2Dname: " << histo2Dname << ", custom: " << custom << "  here2" << std::endl;
      
      resHistos2D_def.push_back(dynamic_cast<TH2D*>(deftemp->Project3D("yx"))); resHistos2D_def.back()->RebinX(10);
      std::cout << "histo2Dname: " << histo2Dname << ", custom: " << custom << "  here3" << std::endl;
      std::cout << "entries: " << resHistos2D_def.back()->GetEntries() << std::endl;
      resHistos2D_def.back()->FitSlicesY(fgaus);
      std::cout << "histo2Dname: " << histo2Dname << ", custom: " << custom << "  here4" << std::endl;

      resHistos1D_1_def.push_back((TH1D*)gDirectory->Get((histo1Dname_yx_1).c_str())); resHistos1D_1_def.back()->SetName((histo1Dname_1_def+custom).c_str());
      std::cout << "histo2Dname: " << histo2Dname << ", custom: " << custom << "  here5" << std::endl;
      resHistos1D_2_def.push_back((TH1D*)gDirectory->Get((histo1Dname_yx_2).c_str())); resHistos1D_2_def.back()->SetName((histo1Dname_2_def+custom).c_str());
      std::cout << "histo2Dname: " << histo2Dname << ", custom: " << custom << "  here6" << std::endl;
      gROOT->cd();
      std::cout << "histo2Dname: " << histo2Dname << ", custom: " << custom << "  here7" << std::endl;
      resHistos2D_new_cond.push_back(dynamic_cast<TH2D*>(newtemp->Project3D("yx"))); resHistos2D_new_cond.back()->RebinX(10);
      std::cout << "entries: " << resHistos2D_new_cond.back()->GetEntries() << std::endl;
      std::cout << "histo2Dname: " << histo2Dname << ", custom: " << custom << "  here8" << std::endl;
      resHistos2D_new_cond.back()->FitSlicesY(fgaus);
      std::cout << "histo2Dname: " << histo2Dname << ", custom: " << custom << "  here9" << std::endl;
      resHistos1D_1_new_cond.push_back((TH1D*)gDirectory->Get((histo1Dname_yx_1).c_str())); resHistos1D_1_new_cond.back()->SetName((histo1Dname_1_new_cond+custom).c_str());
      std::cout << "histo2Dname: " << histo2Dname << ", custom: " << custom << "  here10" << std::endl;
      resHistos1D_2_new_cond.push_back((TH1D*)gDirectory->Get((histo1Dname_yx_2).c_str())); resHistos1D_2_new_cond.back()->SetName((histo1Dname_2_new_cond+custom).c_str());
      std::cout << "histo2Dname: " << histo2Dname << ", custom: " << custom << "  here11" << std::endl;
      gROOT->cd();
      std::cout << "histo2Dname: " << histo2Dname << ", custom: " << custom << "  here12" << std::endl;
      
      if (includeHW) {
	resHistos2D_hw.push_back(dynamic_cast<TH2D*>(hwtemp->Project3D("yx"))); resHistos2D_hw.back()->RebinX(10);
	std::cout << "histo2Dname: " << histo2Dname << ", custom: " << custom << "  here13" << std::endl;
	std::cout << "entries: " << resHistos2D_hw.back()->GetEntries() << std::endl;
	resHistos2D_hw.back()->FitSlicesY(fgaus);
	std::cout << "histo2Dname: " << histo2Dname << ", custom: " << custom << "  here14 " << histo1Dname_yx_1 << std::endl;
	
	resHistos1D_1_hw.push_back((TH1D*)gDirectory->Get((histo1DnameHw_yx_1).c_str()));
	std::cout << "histo2Dname: " << histo2Dname << ", custom: " << custom << "  here14_1" << std::endl;
	resHistos1D_1_hw.back()->SetName((histo1Dname_1_hw+custom).c_str());
	std::cout << "histo2Dname: " << histo2Dname << ", custom: " << custom << "  here15" << std::endl;
	resHistos1D_2_hw.push_back((TH1D*)gDirectory->Get((histo1DnameHw_yx_2).c_str())); resHistos1D_2_hw.back()->SetName((histo1Dname_2_hw+custom).c_str());
	std::cout << "histo2Dname: " << histo2Dname << ", custom: " << custom << "  here16" << std::endl;

      }
  
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

      if (includeHW) {
	resHistos1D_1_hw.back()->Draw("same");
	resHistos1D_1_hw.back()->SetLineColor(kBlue);
	resHistos1D_1_hw.back()->SetLineWidth(lWidth);
	resHistos1D_1_hw.back()->SetMarkerColor(kBlue);
	resHistos1D_1_hw.back()->SetMarkerSize(mSize);
	resHistos1D_1_hw.back()->SetMarkerStyle(20);
      }
      if ( ! includeHW || (includeHW && ! compare_def_hw_only)) {
	resHistos1D_1_new_cond.back()->Draw("same");
	resHistos1D_1_new_cond.back()->SetLineColor(2);
	resHistos1D_1_new_cond.back()->SetLineWidth(lWidth);
	resHistos1D_1_new_cond.back()->SetMarkerColor(2);
	resHistos1D_1_new_cond.back()->SetMarkerSize(mSize);
	resHistos1D_1_new_cond.back()->SetMarkerStyle(20);
      }
      
      TLegend* resLegend1 = new TLegend(0.23, 0.17, 0.43, 0.32);
      resLegend1->AddEntry(resHistos1D_1_def.back(), "Default", "EP");
      if (includeHW) resLegend1->AddEntry(resHistos1D_1_hw.back(), "Hardware", "EP");
      if ( ! includeHW || (includeHW && ! compare_def_hw_only)) resLegend1->AddEntry(resHistos1D_1_new_cond.back(), "New", "EP");
      resLegend1->Draw("SAME");

      if (includeHW) {
	mycanvases_1.back()->Print(Form("plots/%s_hw.png", (meanName+custom).c_str()));
	mycanvases_1.back()->Print(Form("plots/%s_hw.pdf", (meanName+custom).c_str()));
      } else {
	mycanvases_1.back()->Print(Form("plots/%s_emu.png", (meanName+custom).c_str()));
	mycanvases_1.back()->Print(Form("plots/%s_emu.pdf", (meanName+custom).c_str()));
      }
      
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

      if (includeHW) {
	resHistos1D_2_hw.back()->Draw("same");
	resHistos1D_2_hw.back()->SetLineColor(kBlue);
	resHistos1D_2_hw.back()->SetLineWidth(lWidth);
	resHistos1D_2_hw.back()->SetMarkerColor(kBlue);
	resHistos1D_2_hw.back()->SetMarkerSize(mSize);
	resHistos1D_2_hw.back()->SetMarkerStyle(20);
      }
      if ( ! includeHW || (includeHW && ! compare_def_hw_only)) {
	resHistos1D_2_new_cond.back()->Draw("same");
	resHistos1D_2_new_cond.back()->SetLineColor(2);
	resHistos1D_2_new_cond.back()->SetLineWidth(lWidth);
	resHistos1D_2_new_cond.back()->SetMarkerColor(2);
	resHistos1D_2_new_cond.back()->SetMarkerSize(mSize);
	resHistos1D_2_new_cond.back()->SetMarkerStyle(20);
      }
      
      TLegend* resLegend2 = new TLegend(0.33, 0.75, 0.53, 0.90);
      resLegend2->AddEntry(resHistos1D_2_def.back(), "Default", "EP");
      if (includeHW) resLegend2->AddEntry(resHistos1D_2_hw.back(), "Hardware", "EP");
      if ( ! includeHW || (includeHW && ! compare_def_hw_only)) resLegend2->AddEntry(resHistos1D_2_new_cond.back(), "New", "EP");
      resLegend2->Draw("SAME");

      if (includeHW) {
	mycanvases_2.back()->Print(Form("plots/%s_hw.png", (sigmaName+custom).c_str()));
	mycanvases_2.back()->Print(Form("plots/%s_hw.pdf", (sigmaName+custom).c_str()));
      } else {
	mycanvases_2.back()->Print(Form("plots/%s_emu.png", (sigmaName+custom).c_str()));
	mycanvases_2.back()->Print(Form("plots/%s_emu.pdf", (sigmaName+custom).c_str()));
      }
    }
  }

  //-----------------------------------------------------------------------
  // L1 ETM resolution summary plots
  //-----------------------------------------------------------------------
  std::cout << "L1 ETM resolution summary plots " << std::endl;
  TF1 *fgaus0 = new TF1("g0","gaus");//,-2.,2.);
  fgaus0->SetRange(-1.,3.);
  
  TH3F* dtemp_Calo = dynamic_cast<TH3F*>(files.at(0)->Get("hResMET_Calo_emu"));
  TH3F* ntemp_Calo = dynamic_cast<TH3F*>(files.at(1)->Get("hResMET_Calo_emu"));
  TH3F* hwtemp_Calo_3D;
  if (includeHW) hwtemp_Calo_3D = dynamic_cast<TH3F*>(files.at(0)->Get("hResMET_Calo_hw"));
  
  for (unsigned int irange = 0; irange < puRanges.size(); irange++) {

    std::string custom = puRangeNames[irange]; std::vector<int> bounds = puRanges[irange];
    int lowBin = dtemp_Calo->GetZaxis()->FindBin(bounds[0]); int hiBin = dtemp_Calo->GetZaxis()->FindBin(bounds[1]);
    std::cout << "hResMET_Calo_emu    custom: " << custom << "   here1" << std::endl;
    dtemp_Calo->GetZaxis()->SetRange(lowBin, hiBin); 
    ntemp_Calo->GetZaxis()->SetRange(lowBin, hiBin);
    if (includeHW) hwtemp_Calo->GetZaxis()->SetRange(lowBin, hiBin);
    std::cout << "hResMET_Calo_emu    custom: " << custom << "   here2" << std::endl;
    
    std::vector<TCanvas*> mycanvases2;
    TH2D *resMET_Calo_def = dynamic_cast<TH2D*>(dtemp_Calo->Project3D("yx"));resMET_Calo_def->RebinX(10);
    files.at(0)->cd();
    resMET_Calo_def->FitSlicesY(fgaus0);//,1,80);//,20);
    std::cout << "hResMET_Calo_emu    custom: " << custom << "   here3" << std::endl;
    
    TH2D *resMET_Calo_hw;
    if (includeHW) {
      resMET_Calo_hw = dynamic_cast<TH2D*>(hwtemp_Calo_3D->Project3D("yx")); resMET_Calo_hw->RebinX(10);
      files.at(0)->cd();
      resMET_Calo_hw->FitSlicesY(fgaus0);//,1,80);//,20);
    }
    std::cout << "hResMET_Calo_emu    custom: " << custom << "   here4" << std::endl;
    
    TH2D *resMET_Calo_new_cond = dynamic_cast<TH2D*>(ntemp_Calo->Project3D("yx"));resMET_Calo_new_cond->RebinX(10);
    files.at(1)->cd();
    resMET_Calo_new_cond->FitSlicesY(fgaus0);//,1,80);//,20);
    std::cout << "hResMET_Calo_emu    custom: " << custom << "   here5" << std::endl;

    mycanvases2.push_back(new TCanvas);
    mycanvases2.back()->SetWindowSize(mycanvases2.back()->GetWw(), 1.*mycanvases2.back()->GetWh());
    std::cout << "hResMET_Calo_emu    custom: " << custom << "   here6" << std::endl;
    
    gPad->SetGridx(); gPad->SetGridy();
    gPad->SetLeftMargin(lMargin);
    std::cout << "hResMET_Calo_emu    custom: " << custom << "   here7" << std::endl;
    
    TH1D *resMET_Calo_def_1 = (TH1D*)files.at(0)->Get("hResMET_Calo_emu_yx_1");
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
    std::cout << "hResMET_Calo_emu    custom: " << custom << "   here8" << std::endl;

    TH1D *resMET_Calo_hw_1;
    TH1D *resMET_Calo_new_cond_1;
    if (includeHW) {
      resMET_Calo_hw_1 = (TH1D*)files.at(0)->Get("hResMET_Calo_hw_yx_1");
      resMET_Calo_hw_1->Draw("same");
      resMET_Calo_hw_1->SetLineColor(kBlue);
      resMET_Calo_hw_1->SetLineWidth(lWidth);
      resMET_Calo_hw_1->SetMarkerColor(kBlue);
      resMET_Calo_hw_1->SetMarkerSize(mSize);
      resMET_Calo_hw_1->SetMarkerStyle(20);
    }
    std::cout << "hResMET_Calo_emu    custom: " << custom << "   here9" << std::endl;
    if ( ! includeHW || (includeHW && ! compare_def_hw_only)) {
      resMET_Calo_new_cond_1 = (TH1D*)files.at(1)->Get("hResMET_Calo_emu_yx_1");
      resMET_Calo_new_cond_1->Draw("same");
      resMET_Calo_new_cond_1->SetLineColor(2);
      resMET_Calo_new_cond_1->SetLineWidth(lWidth);
      resMET_Calo_new_cond_1->SetMarkerColor(2);
      resMET_Calo_new_cond_1->SetMarkerSize(mSize);
      resMET_Calo_new_cond_1->SetMarkerStyle(20);
    }
    std::cout << "hResMET_Calo_emu    custom: " << custom << "   here10" << std::endl;
      
    TLegend* resLegend1 = new TLegend(0.23, 0.17, 0.43, 0.32);
    resLegend1->AddEntry(resMET_Calo_def_1, "Default", "EP");
    if (includeHW) resLegend1->AddEntry(resMET_Calo_hw_1, "Hardware", "EP");
    if ( ! includeHW || (includeHW && ! compare_def_hw_only)) resLegend1->AddEntry(resMET_Calo_new_cond_1, "New", "EP");
    resLegend1->Draw("SAME");
    std::cout << "hResMET_Calo_emu    custom: " << custom << "   here11" << std::endl;
    
    if (includeHW) {
      mycanvases2.back()->Print(Form("plots/%s_hw.png", ("resMET_Calo_mean"+custom).c_str()));
      mycanvases2.back()->Print(Form("plots/%s_hw.pdf", ("resMET_Calo_mean"+custom).c_str()));
    } else {
      mycanvases2.back()->Print(Form("plots/%s_emu.png", ("resMET_Calo_mean"+custom).c_str()));
      mycanvases2.back()->Print(Form("plots/%s_emu.pdf", ("resMET_Calo_mean"+custom).c_str()));
    }
    std::cout << "hResMET_Calo_emu    custom: " << custom << "   here12" << std::endl;
    
    mycanvases2.push_back(new TCanvas);
    mycanvases2.back()->SetWindowSize(mycanvases2.back()->GetWw(), 1.*mycanvases2.back()->GetWh());

    gPad->SetGridx(); gPad->SetGridy();
    gPad->SetLeftMargin(lMargin);
    std::cout << "hResMET_Calo_emu    custom: " << custom << "   here13" << std::endl;
    
    TH1D *resMET_Calo_def_2 = (TH1D*)files.at(0)->Get("hResMET_Calo_emu_yx_2");
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
    std::cout << "hResMET_Calo_emu    custom: " << custom << "   here14" << std::endl;
    
    TH1D *resMET_Calo_hw_2;
    TH1D *resMET_Calo_new_cond_2;
    if (includeHW) {
      resMET_Calo_hw_2 = (TH1D*)files.at(0)->Get("hResMET_Calo_hw_yx_2");
      resMET_Calo_hw_2->Draw("same");
      resMET_Calo_hw_2->SetLineColor(kBlue);
      resMET_Calo_hw_2->SetMarkerColor(kBlue);
      resMET_Calo_hw_2->SetMarkerSize(mSize);
      resMET_Calo_hw_2->SetLineWidth(lWidth);
      resMET_Calo_hw_2->SetMarkerStyle(20);
    }
    if ( ! includeHW || (includeHW && ! compare_def_hw_only)) {
      resMET_Calo_new_cond_2 = (TH1D*)files.at(1)->Get("hResMET_Calo_emu_yx_2");
      resMET_Calo_new_cond_2->Draw("same");
      resMET_Calo_new_cond_2->SetLineColor(2);
      resMET_Calo_new_cond_2->SetMarkerColor(2);
      resMET_Calo_new_cond_2->SetMarkerSize(mSize);
      resMET_Calo_new_cond_2->SetLineWidth(lWidth);
      resMET_Calo_new_cond_2->SetMarkerStyle(20);
    }
    std::cout << "hResMET_Calo_emu    custom: " << custom << "   here15" << std::endl;
    
    TLegend* resLegend2 = new TLegend(0.23, 0.17, 0.43, 0.32);
    resLegend2->AddEntry(resMET_Calo_def_2, "Default", "EP");
    if (includeHW) resLegend2->AddEntry(resMET_Calo_hw_2, "Hardware", "EP");
    if ( ! includeHW || (includeHW && ! compare_def_hw_only)) resLegend2->AddEntry(resMET_Calo_new_cond_2, "New", "EP");
    resLegend2->Draw("SAME");
    std::cout << "hResMET_Calo_emu    custom: " << custom << "   here16" << std::endl;
    
    if (includeHW) {
      mycanvases2.back()->Print(Form("plots/%s_hw.png", ("resMET_Calo_sigma"+custom).c_str()));
      mycanvases2.back()->Print(Form("plots/%s_hw.pdf", ("resMET_Calo_sigma"+custom).c_str()));
    } else {
      mycanvases2.back()->Print(Form("plots/%s_emu.png", ("resMET_Calo_sigma"+custom).c_str()));
      mycanvases2.back()->Print(Form("plots/%s_emu.pdf", ("resMET_Calo_sigma"+custom).c_str()));
    }
    std::cout << "hResMET_Calo_emu    custom: " << custom << "   here17" << std::endl;
  }
  std::cout << "hResMET_PF_emu    " << std::endl;
  TH3F* dtemp_PF = dynamic_cast<TH3F*>(files.at(0)->Get("hResMET_PF_emu"));
  TH3F* ntemp_PF = dynamic_cast<TH3F*>(files.at(1)->Get("hResMET_PF_emu"));
  TH3F* hwtemp_PF_3D;
  if (includeHW) hwtemp_PF_3D = dynamic_cast<TH3F*>(files.at(0)->Get("hResMET_PF_hw"));
  
  for (unsigned int irange = 0; irange < puRanges.size(); irange++) {

    std::string custom = puRangeNames[irange]; std::vector<int> bounds = puRanges[irange];
    int lowBin = dtemp_PF->GetZaxis()->FindBin(bounds[0]); int hiBin = dtemp_PF->GetZaxis()->FindBin(bounds[1]);
    std::cout << "hResMET_PF_emu   custom " << custom << " here1" << std::endl;
    
    dtemp_PF->GetZaxis()->SetRange(lowBin, hiBin); 
    ntemp_PF->GetZaxis()->SetRange(lowBin, hiBin);
    if (includeHW) hwtemp_PF_3D->GetZaxis()->SetRange(lowBin, hiBin);
    std::cout << "hResMET_PF_emu   custom " << custom << " here2" << std::endl;
    
    std::vector<TCanvas*> mycanvases2;
    TH2D *resMET_PF_def = dynamic_cast<TH2D*>(dtemp_PF->Project3D("yx"));resMET_PF_def->RebinX(10);
    files.at(0)->cd();
    resMET_PF_def->FitSlicesY(fgaus0);//,1,80);//,20);
    std::cout << "hResMET_PF_emu   custom " << custom << " here3" << std::endl;
    
    TH2D *resMET_PF_new_cond = dynamic_cast<TH2D*>(ntemp_PF->Project3D("yx"));resMET_PF_new_cond->RebinX(10);
    files.at(1)->cd();
    resMET_PF_new_cond->FitSlicesY(fgaus0);//,1,80);//,20);
    std::cout << "hResMET_PF_emu   custom " << custom << " here4" << std::endl;
    
    TH2D *resMET_PF_hw;
    if (includeHW) {
      resMET_PF_hw = dynamic_cast<TH2D*>(hwtemp_PF_3D->Project3D("yx"));resMET_PF_hw->RebinX(10);
      files.at(0)->cd();
      resMET_PF_hw->FitSlicesY(fgaus0);//,1,80);//,20);
    }
    std::cout << "hResMET_PF_emu   custom " << custom << " here5" << std::endl;
    
    mycanvases2.push_back(new TCanvas);
    mycanvases2.back()->SetWindowSize(mycanvases2.back()->GetWw(), 1.*mycanvases2.back()->GetWh());

    gPad->SetGridx(); gPad->SetGridy();
    gPad->SetLeftMargin(lMargin);
    std::cout << "hResMET_PF_emu   custom " << custom << " here6" << std::endl;
    
    TH1D *resMET_PF_def_1 = (TH1D*)files.at(0)->Get("hResMET_PF_emu_yx_1");
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
    std::cout << "hResMET_PF_emu   custom " << custom << " here7" << std::endl;
    
    TH1D *resMET_PF_hw_1;
    TH1D *resMET_PF_new_cond_1;
    if (includeHW) {
      resMET_PF_hw_1 = (TH1D*)files.at(0)->Get("hResMET_PF_hw_yx_1");
      resMET_PF_hw_1->Draw("same");
      resMET_PF_hw_1->SetLineColor(kBlue);
      resMET_PF_hw_1->SetLineWidth(lWidth);
      resMET_PF_hw_1->SetMarkerColor(kBlue);
      resMET_PF_hw_1->SetMarkerSize(mSize);
      resMET_PF_hw_1->SetMarkerStyle(20);
    }
    std::cout << "hResMET_PF_emu   custom " << custom << " here8" << std::endl;
    if ( ! includeHW || (includeHW && ! compare_def_hw_only)) {
      resMET_PF_new_cond_1 = (TH1D*)files.at(1)->Get("hResMET_PF_emu_yx_1");
      resMET_PF_new_cond_1->Draw("same");
      resMET_PF_new_cond_1->SetLineColor(2);
      resMET_PF_new_cond_1->SetLineWidth(lWidth);
      resMET_PF_new_cond_1->SetMarkerColor(2);
      resMET_PF_new_cond_1->SetMarkerSize(mSize);
      resMET_PF_new_cond_1->SetMarkerStyle(20);
    }
    std::cout << "hResMET_PF_emu   custom " << custom << " here9" << std::endl;
    
    TLegend* resLegend1 = new TLegend(0.23, 0.17, 0.43, 0.32);
    resLegend1->AddEntry(resMET_PF_def_1, "Default", "EP");
    if (includeHW) resLegend1->AddEntry(resMET_PF_hw_1, "Hardware", "EP");
    if ( ! includeHW || (includeHW && ! compare_def_hw_only))  resLegend1->AddEntry(resMET_PF_new_cond_1, "New", "EP");
    resLegend1->Draw("SAME");
    std::cout << "hResMET_PF_emu   custom " << custom << " here10" << std::endl;
    
    if (includeHW) {
      mycanvases2.back()->Print(Form("plots/%s_hw.png", ("resMET_PF_mean"+custom).c_str()));
      mycanvases2.back()->Print(Form("plots/%s_hw.pdf", ("resMET_PF_mean"+custom).c_str()));
    } else {
      mycanvases2.back()->Print(Form("plots/%s_emu.png", ("resMET_PF_mean"+custom).c_str()));
      mycanvases2.back()->Print(Form("plots/%s_emu.pdf", ("resMET_PF_mean"+custom).c_str()));
    }
    
    mycanvases2.push_back(new TCanvas);
    mycanvases2.back()->SetWindowSize(mycanvases2.back()->GetWw(), 1.*mycanvases2.back()->GetWh());

    gPad->SetGridx(); gPad->SetGridy();
    gPad->SetLeftMargin(lMargin);
    std::cout << "hResMET_PF_emu   custom " << custom << " here11" << std::endl;
    
    TH1D *resMET_PF_def_2 = (TH1D*)files.at(0)->Get("hResMET_PF_emu_yx_2");
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
    std::cout << "hResMET_PF_emu   custom " << custom << " here12" << std::endl;
    
    TH1D *resMET_PF_hw_2;
    TH1D *resMET_PF_new_cond_2;
    if (includeHW) {
      resMET_PF_hw_2 = (TH1D*)files.at(0)->Get("hResMET_PF_hw_yx_2");
      resMET_PF_hw_2->Draw("same");
      resMET_PF_hw_2->SetLineColor(kBlue);
      resMET_PF_hw_2->SetMarkerColor(kBlue);
      resMET_PF_hw_2->SetMarkerSize(mSize);
      resMET_PF_hw_2->SetLineWidth(lWidth);
      resMET_PF_hw_2->SetMarkerStyle(20);
    }
    std::cout << "hResMET_PF_emu   custom " << custom << " here13" << std::endl;
    if ( ! includeHW || (includeHW && ! compare_def_hw_only)) {
      resMET_PF_new_cond_2 = (TH1D*)files.at(1)->Get("hResMET_PF_emu_yx_2");
      resMET_PF_new_cond_2->Draw("same");
      resMET_PF_new_cond_2->SetLineColor(2);
      resMET_PF_new_cond_2->SetMarkerColor(2);
      resMET_PF_new_cond_2->SetMarkerSize(mSize);
      resMET_PF_new_cond_2->SetLineWidth(lWidth);
      resMET_PF_new_cond_2->SetMarkerStyle(20);
    }
    
    TLegend* resLegend2 = new TLegend(0.23, 0.17, 0.43, 0.32);
    resLegend2->AddEntry(resMET_PF_def_2, "Default", "EP");
    if (includeHW) resLegend2->AddEntry(resMET_PF_hw_2, "Hardware", "EP");
    if ( ! includeHW || (includeHW && ! compare_def_hw_only)) resLegend2->AddEntry(resMET_PF_new_cond_2, "New", "EP");
    resLegend2->Draw("SAME");
    std::cout << "hResMET_PF_emu   custom " << custom << " here14" << std::endl;
    
    if (includeHW) {
      mycanvases2.back()->Print(Form("plots/%s_hw.png", ("resMET_PF_sigma"+custom).c_str()));
      mycanvases2.back()->Print(Form("plots/%s_hw.pdf", ("resMET_PF_sigma"+custom).c_str()));
    } else {
      mycanvases2.back()->Print(Form("plots/%s_emu.png", ("resMET_PF_sigma"+custom).c_str()));
      mycanvases2.back()->Print(Form("plots/%s_emu.pdf", ("resMET_PF_sigma"+custom).c_str()));
    }
    std::cout << "hResMET_PF_emu   custom " << custom << " here15" << std::endl;
  }
std::cout << "hResMET_PF_emu   d0ne "  << std::endl;
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

  //  rcanvases.back()->Print(Form("plots/%sbin_emu.png", rType.c_str()));
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

  //  //if(includeHW) canvases.back()->Print(Form("plots/%s_hw.png", iplot.first.c_str()));
  //  //else
  //  canvases.back()->SetLogx();
  //  canvases.back()->Print(Form("plots/%s_emu.png", iplot.first.c_str()));
  //} 

 

  return 0;
}
