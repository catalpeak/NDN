
// IfHaveBadInFace.h

#ifndef IFHAVEBADINFACE_H
#define IFHAVEBADINFACE_H

#include "ns3/ndnSIM/model/ndn-common.hpp"

namespace ns3 {
namespace ndn {

class IfHaveBadInFace {

private :

  bool
  HaveBadInFace;

  const Face *
  InFace;

  double
  KilledTime;

public :

  IfHaveBadInFace () 
    : HaveBadInFace (false)
    , KilledTime    (0.0)
  {

  }

  bool
  JudgeBad () const {
    return HaveBadInFace;
  }

  void
  GoodOrBad (bool _BOOL) {
    HaveBadInFace = _BOOL;
  }

  void 
  SetInFace (const Face & face) {
    InFace = &face;
  }

  const Face *
  ReturnFace () const {
    return InFace;
  }

  void
  SetTime (double NowTime) {
    KilledTime = NowTime;
  }

  double
  ReturnTime () const {
    return KilledTime;
  }

};

} // namespace ndn
} // namespace ns3

#endif // IFHAVEBADINFACE_H
