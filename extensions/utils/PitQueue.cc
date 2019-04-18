// PitQueue.cc

#include "PitQueue.h"

namespace ns3 {
namespace ndn {

void
PitQueue::SetReceiveTime (double _receiveTime) {
  receiveTime = _receiveTime;
}

double
PitQueue::GetReceiveTime () const {
  return receiveTime;
}

void
PitQueue::SetHitTime (double _hitTime) {
  hitTime = _hitTime;
  timeDifference = hitTime - receiveTime;
}

double
PitQueue::GetHitTime () const {
  return hitTime;
}

double
PitQueue::GetTimeDifference () const {
  return timeDifference;
}

void
PitQueue::SetTimeOutDifference () {
  timeDifference = 4000;
  // Tmax is the liftTime of Interest which is 2000ms, when a interest is timeout will set 
  // timeDifference with 2 * Tmax = 4000ms.
}

const Face *
PitQueue::GetFace () const {
  return interface;
}

} // namespace
} // namespace
