
#ifndef BADPRODUCER_H
#define BADPRODUCER_H

#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "/root/ndnSIM/ns-3/src/ndnSIM/apps/ndn-app.hpp"

#include "ns3/nstime.h"
#include "ns3/ptr.h"

#include <iostream>

using namespace std;

namespace ns3 {
namespace ndn {

class BadProducer : public App {

public :

  static TypeId
  GetTypeId (void);

  BadProducer ();

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

#endif // BadPRODUCER_H
