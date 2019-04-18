// MineTime.h
#ifndef MINETIME_H
#define MINETIME_H

#include "ns3/ndnSIM/model/ndn-common.hpp"

namespace ns3 {
namespace ndn {

class MineTime {

public :

  double
  GetNowTimeMS ();

  double
  GetNowTimeS  ();

};

} // namespace ndn
} // namespace ns3

#endif // MINETIME_H
