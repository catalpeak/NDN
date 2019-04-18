

#include "Interface.h"
#include <cmath>

namespace ns3 {
namespace ndn {

void
Interface::SetToken (double _token) {
  token = _token;
}

double 
Interface::ReturnToken () const {
  return token;
}

void 
Interface::AddTentry (double tentry) {
  //double Eentry = 1.0 - (tentry / (3.0 * Tmax));
  double Eentry = 1.0 - tentry / Tmax;
  //double Eentry = pow ((1.0 - tentry / Tmax), 2.0);
  eentryList.push_back (Eentry);
}

void
Interface::AddTimeOutTentry () {
  //double Eentry = 1.0 - (2.0 / 3.0);
  double Eentry = 0;
  eentryList.push_back (Eentry);
}

void
Interface::SetEinf () {
  for (EentryList::iterator ientry = eentryList.begin ();
       ientry != eentryList.end (); ientry++) {
    avg_Eentry += *ientry;
    sum_Eentry += 1.0; // @brief I know that it can use .size (), but I wouldn't lol
  }

//cout << sum_Eentry << endl;

  // @brief this interface is not a neutral interface
  if (sum_Eentry > 1e-6) {

    avg_Eentry /= sum_Eentry;

    for (EentryList::iterator ientry = eentryList.begin ();
         ientry != eentryList.end (); ientry++) {
      var_Eentry += pow (*ientry - avg_Eentry, 2.0);
    }

    var_Eentry /= sum_Eentry;

    double Einf = Alpha * perPeriodEinf 
                + (1.0 - Alpha)
                * pow (avg_Eentry, var_Eentry);

    perPeriodEinf = Einf;
    // @brief I know I can use perPeriodEinf , BUT NO lol
    einf = Einf;

  }

  // But if an interface is a neutral interface we set a neutral number of tokens to it 
  else {
    einf = 0.5;
    perPeriodEinf = 0.5;
  }

  avg_Eentry = 0.0;
  var_Eentry = 0.0;
  sum_Eentry = 0.0;
  eentryList.clear ();

}

double
Interface::ReturnEinf () const {
  return einf;
}

void
Interface::AddNfd () {
  Nfd += 1.0;
  ECvi = einf / pow (2, Nfd + Nfv);
  Edp  = 1 / pow (2, Nfd + Nfv);
}

void
Interface::AddNfv () { 
  Nfv += 1.0;
  ECvi = einf / pow (2, Nfd + Nfv);
  Edp  = 1 / pow (2, Nfd + Nfv);
}

double
Interface::ReturnPverconj (double thresholdV) {
  if (ECvi <= thresholdV) {
    return 1.0;
  } else {
    return pow ((1.0 - ECvi) / (1.0 - thresholdV), 2);
  }
}

double
Interface::ReturnEdp () const {
  return Edp;
}

void
Interface::ResetEdp () {
  Edp = 1;
}

} // namespace ndn
} // namespace ns3
