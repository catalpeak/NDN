//MineTime.cc

#include "MineTime.h"

namespace ns3 {
namespace ndn {

double
MineTime::GetNowTimeMS () {
  return ns3::Now ().GetMilliSeconds ();
}

double
MineTime::GetNowTimeS  () {
  return ns3::Now ().GetSeconds ();
}

} // namespace ndn
} // namespace ns3
