

#include "ns3/ptr.h"
#include "ns3/ndnSIM/model/ndn-common.hpp"

#include <iostream>
#include <vector>
#include <math.h>

using namespace std;

namespace ns3 {
namespace ndn {

class Interface {

private :

  double token;

  double perPeriodEinf;

  double avg_Eentry;

  double var_Eentry;

  double sum_Eentry;

  double Alpha;

  double Tmax;

  double einf;

  // @brief no feedback
  double Nfd;

  // @brief failure verification
  double Nfv;

  // @brief the credibility of content source of this interface
  double ECvi;

  // @brief Evluation of data producer
  double Edp;

  typedef std::vector <double> EentryList;
  EentryList eentryList;

public :

  Interface (double _token)
    : token (_token)
    , perPeriodEinf (0.0)
    , avg_Eentry    (0.0)
    , var_Eentry    (0.0)
    , sum_Eentry    (0.0)
    , Alpha         (0.5)
    , Tmax          (2000.0)
    , einf          (0.0)
    , Nfd           (0.0)
    , Nfv           (0.0)
      // @brief 1.0 means this interface's credibility is highest at start
    , ECvi          (1.0)
    , Edp           (1.0)
  {

  }

  void 
  SetToken (double _token);

  double 
  ReturnToken () const;

  // First Add
  void 
  AddTentry (double tentry);

  void 
  AddTimeOutTentry ();

  // Then Get
  void
  SetEinf ();

  double 
  ReturnEinf () const;

  void 
  AddNfd ();

  void
  AddNfv ();

  double 
  ReturnPverconj (double thresholdV);

  double
  ReturnEdp () const;

  void
  ResetEdp ();

};

} // namespace ndn
} // namespace ns3
