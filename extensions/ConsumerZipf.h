

#ifndef CONSUMERZIPF_H_
#define CONSUMERZIPF_H_

#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "ns3/ndnSIM/apps/ndn-consumer.hpp"
#include "ns3/ndnSIM/apps/ndn-consumer-cbr.hpp"

#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {
namespace ndn {

class ConsumerZipf : public ConsumerCbr {

public :

  static TypeId
  GetTypeId ();

  ConsumerZipf ();
  virtual ~ConsumerZipf ();

  virtual void
  SendPacket ();

  uint32_t
  GetNextSeq ();

  virtual void
  OnData (shared_ptr<const Data> data) override;

protected :

  virtual void
  ScheduleNextPacket ();

private :

  void
  SetNumberOfContents (uint32_t numOfContents);

  uint32_t
  GetNumberOfContents () const;

  void
  SetQ (double q);

  double
  GetQ () const;

  void
  SetS (double s);

  double
  GetS () const;

private :

  uint32_t m_N;
  double   m_q;
  double   m_s;
  std::vector<double> m_Pcum;

  Ptr<UniformRandomVariable> m_seqRng;

  double ProbabilityOfExclude;

  bool IfWrongData;
  shared_ptr<const Data> data_;

};

} // namespace ndn
} // namespace ns3

#endif // CONSUMERZIPF_H_
