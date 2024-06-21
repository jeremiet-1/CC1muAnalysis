// SBNAna includes.
#include "sbnanaobj/StandardRecord/Proxy/SRProxy.h"
#include "sbnanaobj/StandardRecord/SRVector3D.h"
#include "sbnanaobj/StandardRecord/SRTrack.h"
#include "sbnana/CAFAna/Core/Var.h"
#include "sbnana/CAFAna/Core/Cut.h"

// Root includes.
#include "TVector3.h"
#include "TMath.h"

// std includes.
#include <vector>
#include <algorithm>
#include <limits>
#include <tuple>

// Analysis includes.
#include "../../GeneratorAnalysis/Selections/TwoPTools.cxx"

namespace ana
{
    // Constants
    const float fFVXMax =  199.15f-10.f;
    const float fFVXMin = -199.15f+10.f;
    const float fFVYMax =  200.00f-10.f;
    const float fFVYMin = -200.00f+10.f;
    const float fFVZMax =  500.00f-50.f;
    const float fFVZMin =    0.00f+10.f;

    const float fMuCutMuScore = 30.0f;
    const float fMuCutPrScore = 60.0f;
    const float fMuCutLength  = 50.0f;
    const float fPrCutPrScore = 100.0f;

    const std::map<int, std::tuple<float, float>> PDGToThreshold = {
        {13, {0.1f, 1.2f}}, // Muon
        {2212, {0.3f, 1.0f}}, // Proton
        {211, {0.07f, std::numeric_limits<float>::max()}}, // Pi plus
        {-211, {0.07f, std::numeric_limits<float>::max()}}, // Pi minus
        {111, {0.0f, std::numeric_limits<float>::max()}} // Pi zero
    };

    //////////////
    // Functions
    //////////////

    // Check vector is in fiducial volume
    bool bIsInFV(const caf::Proxy<caf::SRVector3D>* data) {
        if (std::isnan(data->x) || std::isnan(data->y) || std::isnan(data->z)) return false;
        return (
            (data->x > fFVXMin) && (data->x < fFVXMax) &&
            (data->y > fFVYMin) && (data->y < fFVYMax) &&
            (data->z > fFVZMin) && (data->z < fFVZMax) 
        );
    }

    // Check multiplicity of particles in a given pdg
    int iCountMultParticle(const caf::SRTrueInteractionProxy* nu, int pdg, float lb, float up) {
        int count = 0;
        for (auto const& prim : nu->prim) {
            float totp = std::sqrt(std::pow(prim.startp.x, 2) + std::pow(prim.startp.y, 2) + std::pow(prim.startp.z, 2));
            if (prim.pdg == pdg && totp > lb && totp < up) {
                count++;
            }
        }
        return count;
    }

    // Looks for a muon track and gets its PID
    std::tuple<bool, int> bOneMuon(const caf::SRSliceProxy* slc) {
        // Momentum range for muons
        float lb = std::get<0>(PDGToThreshold.at(13));
        float ub = std::get<1>(PDGToThreshold.at(13));

        std::vector<int> CandidateMuons;
        std::vector<int> CandidateMuonsTrkLen;
        bool bSkipPFP = false;

        for (auto const& pfp : slc -> reco.pfp) {
            float fMuAverage = 0.0f;
            float fPrAverage = 0.0f;
            for (int i = 0; i < 3; i++) {
                // Skip events with Nan's or 0's in chi squared values
                if (
                    std::isnan(pfp.trk.chi2pid[i].chi2_muon) ||
                    pfp.trk.chi2pid[i].chi2_muon == 0. ||
                    std::isnan(pfp.trk.chi2pid[i].chi2_proton) ||
                    pfp.trk.chi2pid[i].chi2_proton == 0.
                ) {
                    bSkipPFP = true;
                    break;
                }
                fMuAverage += pfp.trk.chi2pid[i].chi2_muon / 3;
                fPrAverage += pfp.trk.chi2pid[i].chi2_proton / 3;
            }
            if (bSkipPFP) continue;
            
            // Check start point is in FV and assign momentum based on end point
            if (!bIsInFV(&pfp.trk.start)) continue;

            float fMomentum;
            if (!bIsInFV(&pfp.trk.end)) {
                if (std::isnan(pfp.trk.mcsP.fwdP_muon)) continue;
                fMomentum = pfp.trk.mcsP.fwdP_muon;
            } else {
                fMomentum = pfp.trk.rangeP.p_muon;
            }
            
            if (std::isnan(pfp.trk.len)) continue;
            if (
                fMuAverage < fMuCutMuScore &&
                fPrAverage > fMuCutPrScore &&
                pfp.trk.len > fMuCutLength &&
                fMomentum > lb &&
                fMomentum < ub
            ) {
                CandidateMuons.push_back(pfp.id);
                CandidateMuonsTrkLen.push_back(pfp.trk.len);
            }
        }

        // Choose candidate muon with longest track
        if (CandidateMuons.size() == 0) return {false, -1};

        float fLongestTrack = CandidateMuonsTrkLen.at(0);
        int iMuonIndex = CandidateMuons.at(0);
        for (std::size_t i = 1; i < CandidateMuons.size(); i++) {
            if (CandidateMuonsTrkLen.at(i) > fLongestTrack) {
                fLongestTrack = CandidateMuonsTrkLen.at(i);
                iMuonIndex = CandidateMuons.at(i);
            }
        }
        return {true, iMuonIndex};
    }

    // Look for protons and get their PIDs
    std::tuple<bool, std::vector<int>> bTwoProtons(const caf::SRSliceProxy* slc, int MuonID) {
        // Momentum range for protons
        float lb = std::get<0>(PDGToThreshold.at(2212)); 
        float ub = std::get<1>(PDGToThreshold.at(2212));

        std::vector<int> ProtonIDs;
        for (auto const& pfp : slc -> reco.pfp) {
            if (pfp.id == MuonID) continue; // skip pfp tagged as muon

            float fPrAverage  = 0.0f;
            bool bSkipPFP = false;

            for (int i = 0; i < 3; i++) {
                // Skip events with Nan's or 0's in chi squared values
                if (
                    std::isnan(pfp.trk.chi2pid[i].chi2_muon) ||
                    pfp.trk.chi2pid[i].chi2_muon == 0.
                ) {
                    bSkipPFP = true;
                    break;
                }
                fPrAverage += pfp.trk.chi2pid[i].chi2_proton / 3;
            }
            if (bSkipPFP) continue;

            // Check full track is in FV
            if (!(bIsInFV(&pfp.trk.start) && bIsInFV(&pfp.trk.end))) continue;
            float fMomentum = pfp.trk.rangeP.p_proton;

            if (
                fPrAverage < fPrCutPrScore && 
                fMomentum > lb &&
                fMomentum < ub
            ) ProtonIDs.push_back(pfp.id);
        }
        return {ProtonIDs.size() == 2, ProtonIDs};
    }

    bool bNoChargedPions(const caf::SRSliceProxy* slc, std::vector<int> TaggedIDs) {
        // Momentum range for charged pions
        float lb = std::get<0>(PDGToThreshold.at(211)); 
        float ub = std::get<1>(PDGToThreshold.at(211));

        for (auto const& pfp : slc -> reco.pfp) {
            if (std::find(TaggedIDs.begin(), TaggedIDs.end(), pfp.id) != TaggedIDs.end()) continue;
            if (!(bIsInFV(&pfp.trk.start) && bIsInFV(&pfp.trk.end))) continue;

            float fMomentum = pfp.trk.rangeP.p_pion;
            if (fMomentum > lb && fMomentum < ub) return false; // tag pion
        }
        return true;
    }

    bool bNoShowers(const caf::SRSliceProxy* slc, std::vector<int> TaggedIDs) {
        for (auto const& pfp : slc -> reco.pfp) {
            if (std::find(TaggedIDs.begin(), TaggedIDs.end(), pfp.id) != TaggedIDs.end()) continue;
            if (pfp.trackScore == -5.f) continue; // clear cosmic
            if (pfp.trackScore < 0.5) return false;
        }
        return true;
    }

    ////////////
    // MultiVars
    ////////////

    // Gets position vectors given the particle IDs
    std::tuple<TVector3, TVector3, TVector3> GetVectors(const caf::SRSliceProxy* slc, int MuonID, int ProtonID1, int ProtonID2) {
        TVector3 Muon(1, 1, 1); 
        TVector3 LeadingProton(1, 1, 1); 
        TVector3 RecoilProton(1, 1, 1);
        
        for (auto const& pfp : slc -> reco.pfp) {
            if (pfp.id == MuonID) {
                Muon.SetTheta(TMath::ACos(pfp.trk.costh));
                Muon.SetPhi(pfp.trk.phi);
                if (bIsInFV(&pfp.trk.end)) {
                    Muon.SetMag(pfp.trk.rangeP.p_muon);
                } else {
                    Muon.SetMag(pfp.trk.mcsP.fwdP_muon);
                }
            } else if (pfp.id == ProtonID1) {
                LeadingProton.SetTheta(TMath::ACos(pfp.trk.costh));
                LeadingProton.SetPhi(pfp.trk.phi);
                LeadingProton.SetMag(pfp.trk.rangeP.p_proton);
            } else if (pfp.id == ProtonID2) {
                RecoilProton.SetTheta(TMath::ACos(pfp.trk.costh));
                RecoilProton.SetPhi(pfp.trk.phi);
                RecoilProton.SetMag(pfp.trk.rangeP.p_proton);
            }
        }
        return {Muon, LeadingProton, RecoilProton};
    }

    // Contains all variables we are interested in
    const MultiVar kVars([](const caf::SRSliceProxy* slc) -> std::vector<double> {
        std::vector<double> vars; 

        // Get IDs for tagged particles
        std::vector<int> TaggedIDs;
        auto [OneMuon, MuonID] = bOneMuon(slc);
        auto [TwoProtons, ProtonIDs] = bTwoProtons(slc, MuonID);
        auto [Muon, LeadingProton, RecoilProton] = GetVectors(slc, MuonID, ProtonIDs.at(0), ProtonIDs.at(1));

        // Use helper class
        TwoPTools Helper(Muon, LeadingProton, RecoilProton);

        double MuonCosTheta = Helper.ReturnMuonCosTheta();
        vars.push_back(MuonCosTheta);

        double LeadingProtonCosTheta = Helper.ReturnLeadingProtonCosTheta();
        vars.push_back(LeadingProtonCosTheta);

        double RecoilProtonCosTheta = Helper.ReturnRecoilProtonCosTheta();
        vars.push_back(RecoilProtonCosTheta);

        double CosOpeningAngleProtons = Helper.ReturnCosOpeningAngleProtons();
        vars.push_back(CosOpeningAngleProtons);

        double CosOpeningAngleMuonTotalProton = Helper.ReturnCosOpeningAngleMuonTotalProton();
        vars.push_back(CosOpeningAngleMuonTotalProton);

        double DeltaAlphaT = Helper.ReturnDeltaAlphaT();
        vars.push_back(DeltaAlphaT);

        double TransverseMomentum = Helper.ReturnTransverseMomentum();
        vars.push_back(TransverseMomentum);

        double MuonMomentum = Helper.ReturnMuonMomentum();
        vars.push_back(MuonMomentum);

        double LeadingProtonMomentum = Helper.ReturnLeadingProtonMomentum();
        vars.push_back(LeadingProtonMomentum);

        double RecoilProtonMomentum = Helper.ReturnRecoilProtonMomentum();
        vars.push_back(RecoilProtonMomentum);

        return vars;
    });

    ////////////// 
    // Vars
    //////////////

    // Primary energy
    const Var kPrimaryEnergy = SIMPLEVAR(truth.E);

    // True energy
    const TruthVar kTrueEnergy = SIMPLETRUTHVAR(E);

    // Muon angle
    const Var kMuonCosTheta([](const caf::SRSliceProxy* slc) -> double {
        return kVars(slc).at(0);
    });

    // Leading proton angle
    const Var kLeadingProtonCosTheta([](const caf::SRSliceProxy* slc) -> double {
        return kVars(slc).at(1);
    });

    // Recoil proton angle
    const Var kRecoilProtonCosTheta([](const caf::SRSliceProxy* slc) -> double {
        return kVars(slc).at(2);
    });

    // Opening angle between protons
    const Var kCosOpeningAngleProtons([](const caf::SRSliceProxy* slc) -> double {
        return kVars(slc).at(3);
    });

    // Opening angle between muon and total proton
    const Var kCosOpeningAngleMuonTotalProton([](const caf::SRSliceProxy* slc) -> double {
        return kVars(slc).at(4);
    });

    // Delta alpha transverse
    const Var kDeltaAlphaT([](const caf::SRSliceProxy* slc) -> double {
        return kVars(slc).at(5);
    });

    // Transverse momentum
    const Var kTransverseMomentum([](const caf::SRSliceProxy* slc) -> double {
        return kVars(slc).at(6);
    });

    // Muon momentum
    const Var kMuonMomentum([](const caf::SRSliceProxy* slc) -> double {
        return kVars(slc).at(7);
    });

    // Leading proton momentum
    const Var kLeadingProtonMomentum([](const caf::SRSliceProxy* slc) -> double {
        return kVars(slc).at(8);
    });

    // Muon momentum
    const Var kRecoilProtonMomentum([](const caf::SRSliceProxy* slc) -> double {
        return kVars(slc).at(9);
    });

    //////////////
    // Truth Cuts
    //////////////

    const TruthCut kTruthIsSignal([](const caf::SRTrueInteractionProxy* nu) {
        return (
            bIsInFV(&nu->position) && // check position is in fiducial volume
            nu->iscc &&               // check it is charged current interaction
            nu->pdg == 14 &&          // check neutrino is muon neutrino
            iCountMultParticle(nu, 13, std::get<0>(PDGToThreshold.at(13)), std::get<1>(PDGToThreshold.at(13))) == 1 &&       // check for one muon
            iCountMultParticle(nu, 2212, std::get<0>(PDGToThreshold.at(2212)), std::get<1>(PDGToThreshold.at(2212))) == 2 && // check for two protons
            iCountMultParticle(nu, 211, std::get<0>(PDGToThreshold.at(211)), std::get<1>(PDGToThreshold.at(211))) == 0 &&    // no positively charged pions
            iCountMultParticle(nu, -211, std::get<0>(PDGToThreshold.at(-211)), std::get<1>(PDGToThreshold.at(-211))) == 0 && // no negatively charged pions
            iCountMultParticle(nu, 111, std::get<0>(PDGToThreshold.at(111)), std::get<1>(PDGToThreshold.at(111))) == 0       // no neutral pions
        );
    });

    const TruthCut kTruthNoSignal([](const caf::SRTrueInteractionProxy* nu) {
        return !kTruthIsSignal(nu);
    });

    //////////////
    // Cuts
    //////////////

    // Check fmatch is in beam
    const Cut kIsInBeam([](const caf::SRSliceProxy* slc) {
        return ((slc->fmatch.time > 0.) && (slc->fmatch.time < 1.800));
    });

    // Check reconstructed event is signal
    const Cut kRecoIsSignal([](const caf::SRSliceProxy* slc) {
        std::vector<int> TaggedIDs;

        // Check neutrino vertex is in fiducial volume
        if (!bIsInFV(&slc->vertex)) return false;

        // Reject cosmic events
        if (!(
            slc->nu_score > 0.4 &&     // check how neutrino like slice is
            slc->fmatch.score < 7.0 && // check flash match score
            slc->fmatch.time > 0. &&   // check flash is in beam
            slc->fmatch.time < 1.8
        )) return false; 

        // Check there is one muon in signal
        auto [OneMuon, MuonID] = bOneMuon(slc);
        if (!OneMuon) return false;
        TaggedIDs.push_back(MuonID);

        // Check there are two protons in signal
        auto [TwoProtons, ProtonIDs] = bTwoProtons(slc, MuonID);
        if (!TwoProtons) return false;
        TaggedIDs.insert(TaggedIDs.end(), ProtonIDs.begin(), ProtonIDs.end());

        // Check there are no charged pions
        if (!bNoChargedPions(slc, TaggedIDs)) return false;

        // // Check there are no shower-like objects (neutral pions)
        if (!bNoShowers(slc, TaggedIDs)) return false;

        // Signal definition satisifed
        return true;
    });

    const Cut kRecoIsTrueReco([](const caf::SRSliceProxy* slc) {
        return (kRecoIsSignal(slc) && kTruthIsSignal(&slc->truth));
    });

    const Cut kRecoIsBackground([](const caf::SRSliceProxy* slc) {
        return (kRecoIsSignal(slc) && kTruthNoSignal(&slc->truth));
    });

    const Cut kNoInvalidVariables([](const caf::SRSliceProxy* slc) {
        if (std::isnan(slc->vertex.x) || std::isnan(slc->vertex.y) || std::isnan(slc->vertex.z)) return false;
        for (auto const& pfp : slc -> reco.pfp) {
            if (std::isnan(pfp.trk.start.x) || std::isnan(pfp.trk.start.y) || std::isnan(pfp.trk.start.z)) return false;
            if (std::isnan(pfp.trk.end.x)   || std::isnan(pfp.trk.end.y)   || std::isnan(pfp.trk.end.z)) return false;
            if (std::isnan(pfp.trk.len)) return false;
            if (std::isnan(pfp.trk.mcsP.fwdP_muon) || std::isnan(pfp.trk.rangeP.p_muon)) return false;

            for (int i = 0; i < 3; i++) {
                if (
                    std::isnan(pfp.trk.chi2pid[i].chi2_muon) ||
                    pfp.trk.chi2pid[i].chi2_muon == 0. ||
                    std::isnan(pfp.trk.chi2pid[i].chi2_proton) ||
                    pfp.trk.chi2pid[i].chi2_proton == 0.
                ) return false;
            }
        }
        return true;
    });

    const Cut kTrueSignal([](const caf::SRSliceProxy* slc) {
        return (kNoInvalidVariables(slc) && kTruthIsSignal(&slc->truth));
    });
}