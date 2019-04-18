
#ifndef MPRODUCER_H
#define MPRODUCER_H

#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "ns3/ndnSIM/apps/ndn-app.hpp"

#include "ns3/nstime.h"
#include "ns3/ptr.h"

#include <iostream>

using namespace std;

namespace ns3 {
namespace ndn {

class MProducer : public App {

public :

  static TypeId
  GetTypeId (void);

  MProducer ();

  virtual void
  OnInterest (shared_ptr<const Interest> interest);

protected :

  virtual void
  StartApplication ();
  
  virtual void
  StopApplication ();

private :
  
  Name m_prefix;
  Name m_postfix;
  uint32_t m_virtualPayloadSize;
  Time m_freshness;

  uint32_t m_signature;
  Name m_keyLocator;

  uint64_t m_congestionMark;

};

} // namespace ndn
} // namespace ns3

#endif // MPRODUCER_H
