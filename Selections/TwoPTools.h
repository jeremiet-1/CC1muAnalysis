#ifndef TwoPTools_h
#define TwoPTools_h

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <vector>

#include "TMath.h"
#include <TVector3.h>
#include <TLorentzVector.h>

class TwoPTools {
    private: 
        double fMuonMomentum;
        double fLeadingProtonMomentum;
        double fLeadingProtonCosTheta;
        double fRecoilProtonMomentum;
        double fRecoilProtonCosTheta;
        double fCosOpeningAngleProtons;
        double fCosOpeningAngleMuonTotalProton;
        double fTransverseMomentum;
        double fDeltaAlphaT;

    public:
        // Default constructor
        TwoPTools(TVector3 MuonVector, TVector3 LeadingProtonVector, TVector3 RecoilProtonVector);

        // Default destructor
        ~TwoPTools() = default;

        // Getter functions
        double ReturnMuonMomentum();
        double ReturnLeadingProtonMomentum();
        double ReturnLeadingProtonCosTheta();
        double ReturnRecoilProtonMomentum();
        double ReturnRecoilProtonCosTheta();
        double ReturnCosOpeningAngleProtons();
        double ReturnCosOpeningAngleMuonTotalProton();
        double ReturnTransverseMomentum();
        double ReturnDeltaAlphaT();
};

#endif