

#include "MTBFS.h"
#include "algorithm.hpp"
#include "core/logger.hpp"

namespace nfd {
namespace fw  {

NFD_REGISTER_STRATEGY (MTBFS);

NFD_LOG_INIT ("MTBFS");

const time::milliseconds MTBFS::RETX_SUPPRESSION_INITIAL(10);
const time::milliseconds MTBFS::RETX_SUPPRESSION_MAX(250);

MTBFS::MTBFS (Forwarder& forwarder, const Name& name)
  : Strategy (forwarder)
  , ProcessNackTraits(this)
  , m_retxSuppression( RETX_SUPPRESSION_INITIAL
                     , RetxSuppressionExponential::DEFAULT_MULTIPLIER
                     , RETX_SUPPRESSION_MAX)
  , NOWTIME       (0.0)
  , perPeriodTime (0.0)
  , onePeriodTime (1000.0)
  , SUMENTRY      (0.0)
  , AVG           (0.0)
{
  ParsedInstanceName parsed = parseInstanceName(name);
  if (!parsed.parameters.empty()) {
    BOOST_THROW_EXCEPTION(std::invalid_argument("MulticastStrategy does not accept parameters"));
  }
  if (parsed.version && *parsed.version != getStrategyName()[-1].toVersion()) {
    BOOST_THROW_EXCEPTION(std::invalid_argument(
      "MTBFS does not support version " + to_string(*parsed.version)));
  }
  this->setInstanceName(makeInstanceName(name, getStrategyName()));
}

const Name&
MTBFS::getStrategyName () {
  static Name strategyName ("/localhost/nfd/strategy/MTBFS/%FD%03");
  return strategyName;
}

void 
MTBFS::afterReceiveInterest (const Face& inFace, const Interest& interest,
                             const shared_ptr<pit::Entry>& pitEntry)
{
  const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
  const fib::NextHopList& nexthops = fibEntry.getNextHops();

  int nEligibleNextHops = 0;

  bool isSuppressed = false;

  if (IfAPeriod ()) {
    NOWTIME += 1.0;

    cout << NOWTIME << "\t" << AVG / SUMENTRY << endl;

    SUMENTRY = 0.0;
    AVG      = 0.0;
  }

  for (const auto& nexthop : nexthops) {
    Face& outFace = nexthop.getFace();

    RetxSuppressionResult suppressResult = m_retxSuppression.decidePerUpstream(*pitEntry, outFace);

    if (suppressResult == RetxSuppressionResult::SUPPRESS) {
      NFD_LOG_DEBUG(interest << " from=" << inFace.getId()
                    << "to=" << outFace.getId() << " suppressed");
      isSuppressed = true;
      continue;
    }

    if ((outFace.getId() == inFace.getId() && outFace.getLinkType() != ndn::nfd::LINK_TYPE_AD_HOC) ||
        wouldViolateScope(inFace, interest, outFace)) {
      continue;
    }


    pitQueue.insert (make_pair (&interest, time.GetNowTimeMS ()));

    this->sendInterest(pitEntry, outFace, interest);
    NFD_LOG_DEBUG(interest << " from=" << inFace.getId()
                           << " pitEntry-to=" << outFace.getId());
    if (suppressResult == RetxSuppressionResult::FORWARD) {
      m_retxSuppression.incrementIntervalForOutRecord(*pitEntry->getOutRecord(outFace));
    }
    ++nEligibleNextHops;
  }

  if (nEligibleNextHops == 0 && !isSuppressed) {
    NFD_LOG_DEBUG(interest << " from=" << inFace.getId() << " noNextHop");

    lp::NackHeader nackHeader;
    nackHeader.setReason(lp::NackReason::NO_ROUTE);
    this->sendNack(pitEntry, inFace, nackHeader);

    this->rejectPendingInterest(pitEntry);
  }
}

void
MTBFS::afterReceiveNack(const Face& inFace, const lp::Nack& nack,
                                    const shared_ptr<pit::Entry>& pitEntry)
{
  this->processNack(inFace, nack, pitEntry);
}

void
MTBFS::beforeSatisfyInterest ( const shared_ptr <pit::Entry> & pitEntry
                             , const Face & inFace, const Data & data)
{
  for (PitQueue::iterator ientry = pitQueue.begin ();
       ientry != pitQueue.end ();) {
    if ((&data)->getName () == ientry->first->getName ()) {
      if  (time.GetNowTimeMS () - ientry->second <= 2000) {
        SUMENTRY += 1.0;
        AVG      += time.GetNowTimeMS () - ientry->second;
      }
      pitQueue.erase (ientry++);
    } else {
      ++ientry;
    }
  }
  Strategy::beforeSatisfyInterest (pitEntry, inFace, data);
}

void
MTBFS::afterReceiveData (const shared_ptr<pit::Entry>& pitEntry,
                         const Face& inFace, const Data& data) {
//cout << data.getSignature ().getKeyLocator ().getName ().toUri () << endl;
// result is "/100"
/*
if (time.GetNowTimeMS () >= 30000) {
  cout << data.getSignature ().getKeyLocator ().getName ().toUri () << endl;
}
*/

  if (data.getSignature ().getKeyLocator ().getName ().toUri () != "/100") {
    //SUMENTRY += 1.0;
//cout << "FUCK" << endl;
    return;
  }
//cout << "FUCK 1" << endl;
  Strategy::afterReceiveData (pitEntry, inFace, data);
}

bool
MTBFS::IfAPeriod () {
  if (time.GetNowTimeMS () - perPeriodTime >= onePeriodTime) {
    perPeriodTime = time.GetNowTimeMS ();
    return true;
  } else {
    return false;
  }
}

} // namespace fw
} // namespace nfd
