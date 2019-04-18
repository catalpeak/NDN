
// ConsensusAdvertisement.h

#ifndef CONSENSUSADVERTISEMENT_H
#define CONSENSUSADVERTISEMENT_H

#define TEST_FOR_NDS 1
#define TEST_FOR_NDR 2
#define TEST_FOR_NIS 3
#define TEST_FOR_NIR 4

#include <iostream>
#include <cmath>

using namespace std;

namespace ns3 {
namespace ndn {

class CA {

private :

  double NDs;

  double NDr;

  double NIs;

  double NIr;

  double CSHit;

  double ERROR_THRESHOLD;

public :

  explicit
  CA ()
    : NDs   (0.0)
    , NDr   (0.0)
    , NIs   (0.0)
    , NIr   (0.0)
    , CSHit (0.0)
    , ERROR_THRESHOLD (200.0)
  {

  }

  void
  NDsAddOne () {
    NDs += 1.0;
  }

  void
  NDrAddOne () {
    NDr += 1.0;
  }

  void
  NIsAddOne () {
    NIs += 1.0;
  }

  void
  NIrAddOne () {
    NIr += 1.0;
  }

  void
  CSHitAddOne () {
    CSHit += 1.0;
  }

  void
  SetNIs (double _NIs) {
    if (_NIs <= 1e-6) {
      NIs = 0.0;
    } else {
      NIs = _NIs;
    }
  }

  void
  SetNDs (double _NDs) {
    NDs = _NDs;
  }

  void
  SetNDr (double _NDr) {
    NDr = _NDr;
  }

  void
  SetNIr (double _NIr) {
    NIr = _NIr;
  }

  double
  GetNIr () const {
    return NIr;
  }

  double 
  GetNIs () const {
    return NIs;
  }

  double
  GetNDr () const {
    return NDr;
  }

  double 
  GetNDs () const {
    return NDs;
  }

  double
  GetCSHit () const {
    return CSHit;
  }

  string
  ReturnValue () {
    char str [256];
    sprintf (str, "%lf %lf %lf %lf", NDs, NDr, NIs, NIr);
    NDs   = 0.0;
    NDr   = 0.0;
    NIs   = 0.0;
    NIr   = 0.0;
    CSHit = 0.0;
    return str;
  }

  void
  SetCA (CA _ca) {
    NDr = _ca.GetNDr ();
    NDs = _ca.GetNDs ();
    NIr = _ca.GetNIr ();
    NIs = _ca.GetNIs ();
  }

  bool
  TestForCA (CA _ca, int TYPE) {
    if (TYPE == TEST_FOR_NDS) {
      if (abs (_ca.GetNDs () - NDs) > ERROR_THRESHOLD) {
        return false;
      } else {
        return true;
      }
    } else if (TYPE == TEST_FOR_NDR) {
      if (abs (_ca.GetNDr () - NDr) > ERROR_THRESHOLD) {
        return false;
      } else {
        return true;
      }
    } else if (TYPE == TEST_FOR_NIS) {
      if (abs (_ca.GetNIs () - NIs) > ERROR_THRESHOLD) {
        return false;
      } else {
        return true;
      }
    } else if (TYPE == TEST_FOR_NIR) {
      if (abs (_ca.GetNIr () - NIr) > ERROR_THRESHOLD) {
        return false;
      } else {
        return true;
      }
    }
    cout << "WHAT THE F**K WHY YOU INPUT IN \"ns3::ndn::CA::TestForCA\" FUNCTION A ERROR VALUE !!!" << endl;
    return true;
  }

};

} // namespace ndn
} // namespace ns3

#endif // CONSENSUSADVERTISEMENT_H
