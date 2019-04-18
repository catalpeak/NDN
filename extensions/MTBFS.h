

#ifndef MTBFS_H
#define MTBFS_H

#include "ns3/ndnSIM/NFD/daemon/fw/strategy.hpp"
#include "ns3/ndnSIM/NFD/daemon/fw/process-nack-traits.hpp"
#include "ns3/ndnSIM/NFD/daemon/fw/retx-suppression-exponential.hpp"

#include "utils/MineTime.h"

#include <map>
#include <iostream>

using namespace std;

namespace nfd {
namespace fw  {

class MTBFS : public Strategy
            , public ProcessNackTraits<MTBFS>
{
public :
  explicit
  MTBFS (Forwarder& forwarder, const Name& name = getStrategyName());

  static const Name&
  getStrategyName();

  void
  afterReceiveInterest(const Face& inFace, const Interest& interest,
                       const shared_ptr<pit::Entry>& pitEntry) override;

  void
  afterReceiveNack(const Face& inFace, const lp::Nack& nack,
                   const shared_ptr<pit::Entry>& pitEntry) override;

  virtual void
  beforeSatisfyInterest ( const shared_ptr <pit::Entry> & pitEntry
                        , const Face & inFace, const Data & data) override;

  virtual void
  afterReceiveData (const shared_ptr<pit::Entry>& pitEntry,
                    const Face& inFace, const Data& data) override;

  bool
  IfAPeriod ();

private :
  friend ProcessNackTraits<MTBFS>;
  RetxSuppressionExponential m_retxSuppression;

  typedef multimap <const Interest *, double> PitQueue;
  PitQueue pitQueue;

  ns3::ndn::MineTime time;

  double NOWTIME;

  double perPeriodTime;

  double onePeriodTime;

  double SUMENTRY;

  double AVG;

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  static const time::milliseconds RETX_SUPPRESSION_INITIAL;
  static const time::milliseconds RETX_SUPPRESSION_MAX;

}; // class MTBFS

} // namespace fw
} // namespace nfd

#endif // MTBFS_H
