
// TrustModel.h

#ifndef TRUSTMODEL_H
#define TRUSTMODEL_H

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/ndnSIM/apps/ndn-consumer.hpp"

#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace std;

namespace ns3 {
namespace ndn {

class TrustModel : public Consumer {

public :

  static TypeId 
  GetTypeId ();

  TrustModel ();
  virtual ~TrustModel ();

  void
  SendPacket ();

protected :

  virtual void
  ScheduleNextPacket () override;

protected :

  double m_frequency;

  bool m_firstTime;

  string AppName;

  bool IfFirstTime;

};

} // namespace ndn
} // namespace ns3

#endif // TRUSTMODEL_H
