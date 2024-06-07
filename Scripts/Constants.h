#ifndef Constants_h
#define Constants_h

#include "TString.h"
#include "TMath.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <map>

using namespace std;

namespace Constants {
    // Variables for double differential analysis
    static const int TwoDNBinsMuonCosTheta = 2; 
    std::vector<double> TwoDArrayNBinsMuonCosTheta{0.0,0.5,1.0};

    static const int TwoDNBinsTransverseMomentum = 11;
    std::vector<double> TwoDArrayTransverseMomentum{0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.,1.1,1.2};

    static const int TwoDNBinsDeltaAlphaT = 9;
    std::vector<double> TwoDArrayDeltaAlphaT{0.,20.,40.,60.,80.,100.,120.,140.,160.,180.};

    std::vector<std::vector<double>> TwoDArrayNBinsTransverseMomentumInMuonCosThetaSlices{
        {0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.1,1.2},
        {0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.1,1.2},
    };
    static const TString LabelXAxisTwoDTransverseMomentumInMuonCosTheta = ";#deltap_{T} [bin #]";

    std::vector<std::vector<double>> TwoDArrayNBinsDeltaAlphaTInMuonCosThetaSlices{
        {0.,20.,40.,60.,80.,100.,120.,140.,160.,180.},
        {0.,20.,40.,60.,80.,100.,120.,140.,160.,180.},
    };
    static const TString LabelXAxisTwoDDeltaAlphaTInMuonCosTheta = ";#delta #alpha_{T} [bin #]";

    static std::map<TString,TString> LatexLabel = {
        { "MuonCosThetaPlot",  "All events" },
        { "LeadingProtonCosThetaPlot",  "All events" },	
        { "RecoilProtonCosThetaPlot",  "All events" },	
        { "LeadingProtonMomentumPlot",  "All events" },	
        { "RecoilProtonMomentumPlot",  "All events" },	
        { "MuonMomentumPlot",  "All events" },	
        { "CosOpeningAngleProtonsPlot",  "All events" },	
        { "CosOpeningAngleMuonTotalProtonPlot",  "All events" },	
        { "TransverseMomentumPlot",  "All events" },	
    };
}

#endif