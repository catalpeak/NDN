// PitQueue.h

#ifndef PITQUEUE_H
#define PITQUEUE_H

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/ndnSIM/NFD/daemon/fw/strategy.hpp"
#include "ns3/ndnSIM/NFD/daemon/fw/process-nack-traits.hpp"
#include "ns3/ndnSIM/NFD/daemon/fw/retx-suppression-exponential.hpp"

#include <iostream>

using namespace std;

using namespace nfd;

namespace ns3 {
namespace ndn {

class PitQueue {

private :


  const Face *
  interface;

  double
  receiveTime;

  double 
  hitTime;

  double 
  timeDifference;

public :

  PitQueue (double nowTime, const Face* face) 
    : interface   (face)
    , receiveTime (nowTime)
  {

  }

  PitQueue (double nowTime)
    : receiveTime (nowTime)
  {

  }

  void
  SetReceiveTime (double _receiveTime);

  double 
  GetReceiveTime () const;

  void
  SetHitTime (double _hitTime);

  double
  GetHitTime () const;

  double
  GetTimeDifference () const;

  void
  SetTimeOutDifference ();

  const Face * 
  GetFace () const;

};

} // namespace ndn
} // namespace ns3

#endif // PITQUEUE_H
