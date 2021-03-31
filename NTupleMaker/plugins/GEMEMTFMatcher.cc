#include "TROOT.h"
#include <vector>

// FWCore
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h" // Necessary for DEFINE_FWK_MODULE

// Output branches
#include "EMTFAnalyzer/NTupleMaker/interface/Branches/EventInfo.h"
#include "EMTFAnalyzer/NTupleMaker/interface/Branches/GenMuonInfo.h"
#include "EMTFAnalyzer/NTupleMaker/interface/Branches/EMTFHitInfo.h"
#include "EMTFAnalyzer/NTupleMaker/interface/Branches/EMTFSimHitInfo.h"
#include "EMTFAnalyzer/NTupleMaker/interface/Branches/EMTFTrackInfo.h"
#include "EMTFAnalyzer/NTupleMaker/interface/Branches/EMTFUnpTrackInfo.h"
#include "EMTFAnalyzer/NTupleMaker/interface/Branches/CSCSegInfo.h"
#include "EMTFAnalyzer/NTupleMaker/interface/Branches/RecoMuonInfo.h"
#include "EMTFAnalyzer/NTupleMaker/interface/Branches/RecoPairInfo.h"

// Object matchers
#include "EMTFAnalyzer/NTupleMaker/interface/Matchers/RecoTrkDR.h"
#include "EMTFAnalyzer/NTupleMaker/interface/Matchers/RecoUnpTrkDR.h"
#include "EMTFAnalyzer/NTupleMaker/interface/Matchers/UnpEmuTrkDR.h"
#include "EMTFAnalyzer/NTupleMaker/interface/Matchers/SimUnpHit.h"
#include "EMTFAnalyzer/NTupleMaker/interface/Matchers/LCTSeg.h"

// CSC segment geometry
#include "DataFormats/MuonDetId/interface/CSCTriggerNumbering.h"
#include "DataFormats/L1TMuon/interface/CSCConstants.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "Geometry/GEMGeometry/interface/GEMGeometry.h"

// GEM Copads
#include "DataFormats/GEMDigi/interface/GEMCoPadDigiCollection.h"

class GEMEMTFMatcher : public edm::EDProducer {

 public:

  // Constructor/destructor
  explicit GEMEMTFMatcher(const edm::ParameterSet&);
  ~GEMEMTFMatcher();

 private:

  virtual void beginJob();
  virtual void beginRun(const edm::Run&,   const edm::EventSetup&);
  virtual void produce (edm::Event&, const edm::EventSetup&);
  virtual void endRun  (const edm::Run&,   const edm::EventSetup&);
  virtual void endJob  ();
  GlobalPoint getGlobalPosition(unsigned int rawId, const CSCCorrelatedLCTDigi& lct) const;

  // EDM Tokens
  edm::EDGetTokenT<std::vector<l1t::EMTFHit>>              EMTFHit_token;
  edm::EDGetTokenT<std::vector<l1t::EMTFTrack>>            EMTFTrack_token;
  edm::EDGetTokenT<GEMCoPadDigiCollection>                 GEMCoPad_token;

  const CSCGeometry* cscGeometry_;
  bool verbose_;
  int minChamber_;
  int maxChamber_;
}; // End class GEMEMTFMatcher public edm::EDProducer

// Constructor
GEMEMTFMatcher::GEMEMTFMatcher(const edm::ParameterSet& iConfig)
{
  EMTFHit_token      = consumes<std::vector<l1t::EMTFHit>>   (iConfig.getParameter<edm::InputTag>("emtfHitTag"));
  EMTFTrack_token    = consumes<std::vector<l1t::EMTFTrack>> (iConfig.getParameter<edm::InputTag>("emtfTrackTag"));
  GEMCoPad_token = consumes<GEMCoPadDigiCollection> (iConfig.getParameter<edm::InputTag>("gemCoPadTag"));
  verbose_ = iConfig.getParameter<bool>("verbose");
  minChamber_ = iConfig.getParameter<int>("minChamber");
  maxChamber_ = iConfig.getParameter<int>("maxChamber");

  produces<std::vector<l1t::EMTFTrack>>();
  produces<std::vector<l1t::EMTFHit>>();

} // End GEMEMTFMatcher::GEMEMTFMatcher

// Destructor
GEMEMTFMatcher::~GEMEMTFMatcher() {}

// Called once per run
void GEMEMTFMatcher::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {} // End GEMEMTFMatcher::beginRun()


// Called once per run
void GEMEMTFMatcher::endRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {
  if (verbose_) std::cout << "\nInside GEMEMTFMatcher::endRun()\n" << std::endl;
}


// Called once per event
void GEMEMTFMatcher::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  std::unique_ptr<std::vector<l1t::EMTFTrack>> oc(new std::vector<l1t::EMTFTrack>);
  std::unique_ptr<std::vector<l1t::EMTFHit>> oc2(new std::vector<l1t::EMTFHit>);

  edm::Handle<std::vector<l1t::EMTFHit>> emtfHits;
  iEvent.getByToken(EMTFHit_token, emtfHits);

  edm::Handle<std::vector<l1t::EMTFTrack>> emtfTracks;
  iEvent.getByToken(EMTFTrack_token, emtfTracks);

  edm::Handle<GEMCoPadDigiCollection> gemCoPadsH;
  iEvent.getByToken(GEMCoPad_token, gemCoPadsH);
  const GEMCoPadDigiCollection& gemCoPads = *gemCoPadsH.product();

  edm::ESHandle<CSCGeometry> cscGeom;
  iSetup.get<MuonGeometryRecord>().get(cscGeom);
  cscGeometry_ = &(*cscGeom);

  edm::ESHandle<GEMGeometry> gemGeom;
  iSetup.get<MuonGeometryRecord>().get(gemGeom);

  // first step: copy over the original EMTF Hit collection
  if ( emtfHits.isValid() ) {
    // loop over the tracks
    for (const auto& hit : *emtfHits) {
      oc2->push_back(hit);
    }

    if (verbose_ and false) {
      std::cout << "Printing CSC TP info:" << std::endl;
      for (const auto& emtfHit: *emtfHits) {
        if (emtfHit.Is_CSC() == 1 and emtfHit.Station() == 1 and emtfHit.Ring() == 1) {
          std::cout << emtfHit.CSC_DetId() << " " << emtfHit.CreateCSCCorrelatedLCTDigi()
            //                    << ", Phi: " << emtfHit.Phi_glob() << ", Eta: " << emtfHit.Eta() << ", Theta: " << emtfHit.Theta()
                    << std::endl;
        }
      }
    }
  }

  if (verbose_)
    std::cout << "Printing ME1/1 muon info in GEMEMTFMatcher (top):" << std::endl;

  if ( emtfTracks.isValid() ) {

    // loop over the tracks
    for (const auto& ttrack : *emtfTracks) {

      auto track = ttrack;
      if (verbose_)
        std::cout << std::endl << "Analyzing track: pT " << track.Pt() << " eta " << track.Eta() << " phi " << track.Phi_glob()
                  << " Winner: " << track.Winner() << " Rank: " << track.Rank()
                  << " Charge: " << track.Charge() << " BX: " << track.BX()
                  << " Mode: " << track.Mode() << " Zone: " << track.Zone()
                  << " Mode_neighbor: " << track.Mode_neighbor() << " Track_num: " << track.Track_num()
                  <<  std::endl;

      const auto& trackHits = track.Hits();

      double glob_phi;
      double glob_theta;
      double glob_eta;
      double glob_rho;

      // here, need to do the association with GEM hits
      for (const l1t::EMTFHit& emtfHit: trackHits) {

        // require ME1/1 stubs!
        if (emtfHit.Is_CSC() == 1 and
            emtfHit.Station() == 1 and
            emtfHit.Ring() == 1) {

          // ME1/1 detid
          const auto& cscId = emtfHit.CSC_DetId();
          CSCDetId key_id(cscId.endcap(), cscId.station(), cscId.ring(), cscId.chamber(), 3);

          // sector and subsector
          const int sector(CSCTriggerNumbering::triggerSectorFromLabels(key_id));
          const int subsector(CSCTriggerNumbering::triggerSubSectorFromLabels(key_id));

          // ME1/1 chamber
          const auto& cscChamber = cscGeom->chamber(cscId);

          // CSC GP
          const auto& lct = emtfHit.CreateCSCCorrelatedLCTDigi();

          if (verbose_)
            std::cout << "Analyzing " << cscId << " " << lct << std::endl;

          const GlobalPoint& csc_gp = getGlobalPosition(cscId, lct);

          // best copad
          GEMCoPadDigi best;
          GEMDetId bestId;

          // 90% inclusive cut
          float maxDPhi = 0.024;

          // have to consider +1/0/-1 GEM chambers
          for (int deltaChamber = minChamber_; deltaChamber <= maxChamber_; deltaChamber++){

            // corresponding GE1/1 detid
            const GEMDetId gemId(cscId.zendcap(), 1, 1, 0,
                                 (cscId.chamber() + deltaChamber) % 36,
                                 0);

            // copad collection
            const auto& co_pads_in_det = gemCoPads.get(gemId);

            // loop on the GEM coincidence pads
            // find the closest matching one
            for (auto it = co_pads_in_det.first; it != co_pads_in_det.second; ++it) {
              // pick the first layer in the copad!
              const auto& copad = (*it);

              // select in-time coincidence pads
              if ( std::abs(copad.bx(1)) != 0) continue;

              const GEMDetId gemCoId(cscId.zendcap(), 1, 1, 1,
                                     (cscId.chamber() + deltaChamber) % 36,
                                     copad.roll());

              const LocalPoint& gem_lp = gemGeom->etaPartition(gemCoId)->centreOfPad(copad.pad(1));
              const GlobalPoint& gem_gp = gemGeom->idToDet(gemCoId)->surface().toGlobal(gem_lp);

              // Code by Denis 2021-02-23
              int cscIO = cscId.chamber()%2==0?0:1; //determines the CSC inner or outermost table
              int gemIO = (cscIO + abs(deltaChamber))%2; //determines whether the innermost GEM copad is parallel or off-side
              float Slopes[2][2]={{6.523, 5.968},{16.11, 13.90}}; //linear slope correction fit values for innermost GEM to any CSC combinations
              // slope value. The extra -1 is to account for a sign convention
              float cscSlope = pow(-1, lct.getBend()) * emtfHit.Slope();

              // if (verbose_) {
              //   std::cout << "emtfHit.Slope: " << emtfHit.Slope() << std::endl;
              //   if (cscIO==0){std::cout << "corrected EvenToEven slope: " << cscSlope*6.523 << std::endl;}
              //   else { std::cout << "corrected OddToOdd slope: " << cscSlope*13.90 << std::endl;}
              // }

              // need to extract the sign bit of the CSC z coordinate
              float signCSCz = pow(-1, 1-signbit(csc_gp.z()));
              float currentDPhi = reco::deltaPhi(float(csc_gp.phi()) , float(gem_gp.phi())) * signCSCz - Slopes[cscIO][gemIO] * cscSlope * 0.00296/8.;
              std::cout << "Candidate GEM " << gemCoId << " " << copad << " " << currentDPhi << std::endl;

              // if (verbose_) {
              //   std::cout << "Current dPhi: " << currentDPhi << std::endl;
              //   std::cout << "CSC gp phi: " << float(csc_gp.phi()) << ", GEM gp phi: " << float(gem_gp.phi()) << std::endl;
              // }

              if (std::abs(currentDPhi) < std::abs(maxDPhi)) {
                best = copad;
                bestId = gemCoId;
                maxDPhi = currentDPhi;

                glob_phi = emtf::rad_to_deg(gem_gp.phi().value());
                glob_theta = emtf::rad_to_deg(gem_gp.theta().value());
                glob_eta = gem_gp.eta();
                glob_rho = gem_gp.perp();

              }
            }
          }


          if (best.isValid()) {
            if (verbose_) {
              std::cout << "Best matching GEM: " << bestId << " " << best << " maxDPhi: " << maxDPhi << std::endl;
            }

            l1t::EMTFHit bestEMTFHit;

            // create a new EMTFHit with the
            // best matching coincidence pad
            bestEMTFHit.set_subsystem(3);
            bestEMTFHit.SetGEMDetId(bestId);
            bestEMTFHit.set_endcap(bestId.region());
            bestEMTFHit.set_station(1);
            bestEMTFHit.set_ring(1);
            bestEMTFHit.set_chamber(bestId.chamber());
            bestEMTFHit.set_sector(sector);
            bestEMTFHit.set_subsector(subsector);
            bestEMTFHit.set_roll(best.roll());
            bestEMTFHit.set_strip(best.pad(1));
            bestEMTFHit.set_strip_hi(best.pad(1));
            bestEMTFHit.set_strip_low(best.pad(2));
            bestEMTFHit.set_bx(best.bx(1));
            bestEMTFHit.set_valid(1);

            int fph = emtf::calc_phi_loc_int(glob_phi, sector);
            int th = emtf::calc_theta_int(glob_theta, bestEMTFHit.Endcap());

            if (0 > fph || fph > 4920) {break;}
            if (0 > th || th > 32) {break;}
            if (th == 0b11111) {break;}  // RPC hit valid when data is not all ones
            fph <<= 2;                   // upgrade to full CSC precision by adding 2 zeros
            th <<= 2;                    // upgrade to full CSC precision by adding 2 zeros
            th = (th == 0) ? 1 : th;     // protect against invalid value

            bestEMTFHit.set_phi_sim(glob_phi);
            bestEMTFHit.set_theta_sim(glob_theta);
            bestEMTFHit.set_eta_sim(glob_eta);
            bestEMTFHit.set_rho_sim(glob_rho);

            bestEMTFHit.set_phi_loc(emtf::calc_phi_loc_deg_from_glob(glob_phi, sector));
            bestEMTFHit.set_phi_glob(glob_phi);
            bestEMTFHit.set_eta(emtf::calc_eta_from_theta_deg(glob_theta, bestEMTFHit.Endcap() ));
            bestEMTFHit.set_theta(glob_theta);

            bestEMTFHit.set_phi_fp(fph);   // Full-precision integer phi
            bestEMTFHit.set_theta_fp(th);  // Full-precision integer theta

            // push the new hit to the track and to the hit collection
            track.push_Hit(bestEMTFHit);
            oc2->push_back(bestEMTFHit);

            // An EMTF track can only be matched to 1 copad!
            break;
          }
        }
      }

      oc->push_back(track);

    }
  }

  // put the new collections in the event
  iEvent.put(std::move(oc));
  iEvent.put(std::move(oc2));
} // End GEMEMTFMatcher::produce

// Called once per job before starting event loop
void GEMEMTFMatcher::beginJob() {
} // End GEMEMTFMatcher::beginJob

  // Called once per job after ending event loop
void GEMEMTFMatcher::endJob() {
}

GlobalPoint GEMEMTFMatcher::getGlobalPosition(unsigned int rawId, const CSCCorrelatedLCTDigi& lct) const {
  CSCDetId cscId(rawId);
  CSCDetId keyId(cscId.endcap(), cscId.station(), cscId.ring(), cscId.chamber(), CSCConstants::KEY_CLCT_LAYER);
  float fractional_strip = lct.getFractionalStrip();
  // case ME1/a
  if (cscId.station() == 1 and cscId.ring() == 4 and lct.getStrip() > CSCConstants::MAX_HALF_STRIP_ME1B) {
    fractional_strip -= CSCConstants::MAX_NUM_STRIPS_ME1B;
  }
  // regular cases
  const auto& chamber = cscGeometry_->chamber(cscId);
  const auto& layer_geo = chamber->layer(CSCConstants::KEY_CLCT_LAYER)->geometry();
  // LCT::getKeyWG() also starts from 0
  float wire = layer_geo->middleWireOfGroup(lct.getKeyWG() + 1);
  const LocalPoint& csc_intersect = layer_geo->intersectionOfStripAndWire(fractional_strip, wire);
  const GlobalPoint& csc_gp = cscGeometry_->idToDet(keyId)->surface().toGlobal(csc_intersect);
  return csc_gp;
}

// Define as a plugin
DEFINE_FWK_MODULE(GEMEMTFMatcher);
