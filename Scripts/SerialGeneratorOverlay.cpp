#include <TFile.h>
#include <TTree.h>
#include <TString.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <fstream>
#include <stdlib.h>

#include "Constants.h"
#include "../Utils/Tools.cxx"

using namespace std;
using namespace Constants;

void SerialGeneratorOverlay() {

    //------------------------------//

    TH1D::SetDefaultSumw2();
    TH2D::SetDefaultSumw2();
    gStyle->SetOptStat(0);

    int FontStyle = 132;
    double TextSize = 0.06;			

    TString OutFilePath = "/pnfs/sbnd/persistent/users/epelaez/HighSamples/FlatTree/";

    Tools tools;

    //------------------------------//

    // Event generators

    std::vector<TString> Names; std::vector<TString> Labels; std::vector<int> Colors;
    
    Names.push_back(OutFilePath+"FlatTreeAnalyzerOutput_GENIE_AR23.root"); 
    Labels.push_back("GENIE AR23");
    Colors.push_back(kBlue+8);

    // Names.push_back(OutFilePath+"FlatTreeAnalyzerOutput_GENIE_AR23_Emp2015.root"); 
    // Labels.push_back("GENIE AR23 Emp2015");
    // Colors.push_back(kBlue+16);

    // Names.push_back(OutFilePath+"FlatTreeAnalyzerOutput_GENIE_AR23_Nieves2016.root"); 
    // Labels.push_back("GENIE AR23 Nieves2016");
    // Colors.push_back(kBlue+24);

    Names.push_back(OutFilePath+"FlatTreeAnalyzerOutput_GENIE_G18.root"); 
    Labels.push_back("GENIE G18");
    Colors.push_back(kBlue+2);

    Names.push_back(OutFilePath+"FlatTreeAnalyzerOutput_NuWro.root"); 
    Labels.push_back("NuWro");
    Colors.push_back(kRed+1);

    Names.push_back(OutFilePath+"FlatTreeAnalyzerOutput_NEUT.root"); 
    Labels.push_back("NEUT");
    Colors.push_back(kOrange+7);

    Names.push_back(OutFilePath+"FlatTreeAnalyzerOutput_GiBUU.root"); 
    Labels.push_back("GiBUU");
    Colors.push_back(kGreen+1);

    // Names.push_back(OutFilePath+"FlatTreeAnalyzerOutput_GiBUU_NoFSI.root"); 
    // Labels.push_back("GiBUU NoFSI");
    // Colors.push_back(kGreen+1);

    const int NSamples = Names.size();
    std::vector<TFile*> Files; Files.resize(NSamples);

    //------------------------------//

    // Plots to overlay

    std::vector<TString> PlotNames;
    std::vector<TString> XAxisLabel;
    std::vector<TString> YAxisLabel;

    // We only look at double differential plots here
    PlotNames.push_back("TrueSerialTransverseMomentum_InMuonCosThetaPlot");
    XAxisLabel.push_back("#delta P_{T}");
    YAxisLabel.push_back("#frac{d#sigma}{d#delta P_{T}} #left[10^{-38} #frac{cm^{2}}{Ar}#right]");

    PlotNames.push_back("TrueSerialDeltaAlphaT_InMuonCosThetaPlot");
    XAxisLabel.push_back("#delta #alpha_{T}");
    YAxisLabel.push_back("#frac{d#sigma}{d#delta #alpha_{T}} #left[10^{-38} #frac{cm^{2}}{Ar}#right]");

    PlotNames.push_back("TrueSerialCosOpeningAngleProtons_InMuonCosThetaPlot");
    XAxisLabel.push_back("cos(#theta_{#vec{p}_{L},#vec{p}_{R}})");
    YAxisLabel.push_back("#frac{d#sigma}{dcos(#theta_{#vec{p}_{L},#vec{p}_{R}})} #left[10^{-38} #frac{cm^{2}}{Ar}#right]");

    PlotNames.push_back("TrueSerialCosOpeningAngleMuonTotalProton_InMuonCosThetaPlot");
    XAxisLabel.push_back("cos(#theta_{#vec{p}_{#mu},#vec{p}_{sum}})");
    YAxisLabel.push_back("#frac{d#sigma}{dcos(#theta_{#vec{p}_{#mu},#vec{p}_{sum}})} #left[10^{-38} #frac{cm^{2}}{Ar}#right]");

    const int NPlots = PlotNames.size();

    //------------------------------//	

    // Loop over the samples to open the files and the TTree

    for (int iSample = 0; iSample < NSamples; iSample++) {
        Files[iSample] = new TFile(Names[iSample],"readonly");
    } // End of the loop over the samples

    //------------------------------//

    // Loop over the plots to be compared

    for (int iPlot = 0; iPlot < NPlots; iPlot++) {
        TString PlotName = PlotNames[iPlot];
        std::vector<std::vector<TH1D*>> Histos;
        std::vector<TH1D*> TruePlots; TruePlots.resize(NSamples);

        //------------------------------//
        
        // Flatten out double differential bins
        auto [SliceDiscriminators, SliceBinning] = PlotNameToDiscriminator[PlotName];
        auto [NSlices, SerialVectorRanges, SerialVectorBins, SerialVectorLowBin, SerialVectorHighBin] = tools.FlattenNDBins(SliceDiscriminators, SliceBinning);
        
        int StartIndex = 0;

        //------------------------------//

        // Load true plots
        for (int iSample = 0; iSample < NSamples; iSample++) {
            TruePlots[iSample] = (TH1D*)(Files[iSample]->Get(PlotName));
        }

        // Resize histos
        Histos.resize(NSlices);
        for (int iSlice = 0; iSlice < NSlices; iSlice++) {
            Histos[iSlice].resize(NSamples);
        }

        //------------------------------//

        // Loop over the slices
        for (int iSlice = 0; iSlice < NSlices; iSlice++) {
            TString SlicePlotName = PlotName + "_" + TString(std::to_string(iSlice));
            double SliceWidth = SliceDiscriminators[iSlice + 1] - SliceDiscriminators[iSlice]; 

            // Get number of bins
            int SliceNBins = SerialVectorBins.at(iSlice);
            std::vector<double> SerialSliceBinning;

            for (int iBin = 0; iBin < SliceNBins + 1; iBin++) { 
                double value = SerialVectorRanges.at(StartIndex + iBin);
                SerialSliceBinning.push_back(value);
            } // End of the number of bins and the bin ranges declaration

            // Declare canvas and legend
            TString CanvasName = "Canvas_" + SlicePlotName;
            TCanvas* PlotCanvas = new TCanvas(CanvasName,CanvasName,205,34,1124,768);

            PlotCanvas->SetTopMargin(0.13);
            PlotCanvas->SetLeftMargin(0.17);
            PlotCanvas->SetRightMargin(0.05);
            PlotCanvas->SetBottomMargin(0.16);

            TLegend* leg = new TLegend(0.2,0.7,0.75,0.83);
            leg->SetBorderSize(0);
            leg->SetNColumns(2);
            leg->SetTextSize(TextSize*0.8);
            leg->SetTextFont(FontStyle);

            for (int iSample = 0; iSample < NSamples; iSample++) {
                Histos[iSlice][iSample] = tools.GetHistoBins(
                    TruePlots[iSample],
                    SerialVectorLowBin.at(iSlice),
                    SerialVectorHighBin.at(iSlice),
                    SliceWidth,
                    SerialSliceBinning,
                    Labels[iSample]
                );
                Histos[iSlice][iSample]->SetLineWidth(4);
                Histos[iSlice][iSample]->SetLineColor( Colors.at(iSample) );	

                Histos[iSlice][iSample]->GetXaxis()->SetTitleFont(FontStyle);
                Histos[iSlice][iSample]->GetXaxis()->SetLabelFont(FontStyle);
                Histos[iSlice][iSample]->GetXaxis()->SetNdivisions(8);
                Histos[iSlice][iSample]->GetXaxis()->SetLabelSize(TextSize);
                Histos[iSlice][iSample]->GetXaxis()->SetTitle(XAxisLabel.at(iPlot));
                Histos[iSlice][iSample]->GetXaxis()->SetTitleSize(TextSize);
                Histos[iSlice][iSample]->GetXaxis()->SetTitleOffset(1.1);
                Histos[iSlice][iSample]->GetXaxis()->CenterTitle();

                Histos[iSlice][iSample]->GetYaxis()->SetTitleFont(FontStyle);
                Histos[iSlice][iSample]->GetYaxis()->SetLabelFont(FontStyle);
                Histos[iSlice][iSample]->GetYaxis()->SetNdivisions(6);
                Histos[iSlice][iSample]->GetYaxis()->SetLabelSize(TextSize);
                Histos[iSlice][iSample]->GetYaxis()->SetTitle(YAxisLabel.at(iPlot));
                Histos[iSlice][iSample]->GetYaxis()->SetTitleSize(TextSize);
                Histos[iSlice][iSample]->GetYaxis()->SetTitleOffset(1.3);
                Histos[iSlice][iSample]->GetYaxis()->SetTickSize(0);
                Histos[iSlice][iSample]->GetYaxis()->CenterTitle();

                double imax = TMath::Max(Histos[iSlice][iSample]->GetMaximum(),Histos[iSlice][0]->GetMaximum());

                double YAxisRange = 1.15*imax;
                Histos[iSlice][iSample]->GetYaxis()->SetRangeUser(0.,YAxisRange);
                Histos[iSlice][0]->GetYaxis()->SetRangeUser(0.,YAxisRange);			

                PlotCanvas->cd();
                Histos[iSlice][iSample]->Draw("hist same");
                Histos[iSlice][0]->Draw("hist same");	

                leg->AddEntry(Histos[iSlice][iSample],Labels[iSample],"l");
            }
            PlotCanvas->cd();
            leg->Draw();

            TLatex *textSlice = new TLatex();
            textSlice->SetTextFont(FontStyle);
            textSlice->SetTextSize(TextSize);
            TString SliceLabel = tools.to_string_with_precision(SliceDiscriminators[iSlice + 1], 1) + " < " + PlotNameToSliceLabel[PlotName] + " < " + tools.to_string_with_precision(SliceDiscriminators[iSlice], 1);
            textSlice->DrawLatexNDC(0.4,0.92,SliceLabel);

            PlotCanvas->SaveAs("./Figs/Overlay/Serial/"+SlicePlotName+".png");
            delete PlotCanvas;

            // Update starting index to move to next slice
            StartIndex += (SliceNBins + 1);
        } // End loop over slices
    } // End of the loop over the plots

    //------------------------------//

} // End of the program
