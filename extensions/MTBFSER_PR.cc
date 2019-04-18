

#include "MTBFSER_PR.h"
#include "algorithm.hpp"
#include "core/logger.hpp"

#include <iostream>

using namespace std;

namespace nfd {
namespace fw  {

NFD_REGISTER_STRATEGY (MTBFSER_PR);

NFD_LOG_INIT ("MTBFSER_PR");

const time::milliseconds MTBFSER_PR::RETX_SUPPRESSION_INITIAL(10);
const time::milliseconds MTBFSER_PR::RETX_SUPPRESSION_MAX(250);

MTBFSER_PR::MTBFSER_PR (Forwarder& forwarder, const Name& name)
  : Strategy (forwarder)
  , ProcessNackTraits (this)
  , m_retxSuppression ( RETX_SUPPRESSION_INITIAL
                      , RetxSuppressionExponential::DEFAULT_MULTIPLIER
                      , RETX_SUPPRESSION_MAX)
  , NumberOfTokens (200.0)
  , perPeriodTime  (0.0)
  , onePeriodTime  (1000.0)
  , numberOfPit    (0.0)
  , LOWPIT         (250.0)
  , UPPIT          (900.0)
  , NOWTIME        (0.0)
  , THRESHOLDV     (0.2)
  , thresholdB     (0.5)
{
  ParsedInstanceName parsed = parseInstanceName(name);
  if (!parsed.parameters.empty()) {
    BOOST_THROW_EXCEPTION(std::invalid_argument("MulticastStrategy does not accept parameters"));
  }
  if (parsed.version && *parsed.version != getStrategyName()[-1].toVersion()) {
    BOOST_THROW_EXCEPTION(std::invalid_argument(
      "MTBFSER does not support version " + to_string(*parsed.version)));
  }
  this->setInstanceName(makeInstanceName(name, getStrategyName()));
}

const Name&
MTBFSER_PR::getStrategyName () {
  static Name strategyName ("/localhost/nfd/strategy/MTBFSER_PR/%FD%03");
  return strategyName;
}

void 
MTBFSER_PR::afterReceiveInterest (const Face& inFace, const Interest& interest,
                                  const shared_ptr<pit::Entry>& pitEntry)
{
  const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
  const fib::NextHopList& nexthops = fibEntry.getNextHops();

  int nEligibleNextHops = 0;

  bool isSuppressed = false;

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////											//////
//////											//////
//////											//////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

  //@brief count the number of its interface
  interfaceList.insert (make_pair (&inFace, ns3::ndn::Interface (NumberOfTokens)));

  // Test Consumer's feedback
  if (interest.getName ().toUri () == "/Feedback") {
    // Receive a feedback 
    for (InterfaceList::iterator ientry = interfaceList.begin ();
         ientry != interfaceList.end (); ientry++) {
      if (ientry->first == &inFace) {
        ientry->second.AddNfd ();
        break;
      }
    }
  }

  if (interest.getExclude ().size () != 0) {
    // Receive a Exclude
    for (InterfaceList::iterator ientry = interfaceList.begin ();
         ientry != interfaceList.end (); ientry++) {
      if (ientry->first == &inFace) {
        ientry->second.AddNfv ();
        break;
      }
    }
  }

  // @brief Period Checking strategy lueluelue :)
  if (IfAPeriod ()) {
    NOWTIME++;
    for (InterfaceList::iterator ientry = interfaceList.begin ();
         ientry != interfaceList.end (); ientry++) {
      ientry->second.SetEinf ();
      ientry->second.ResetEdp ();
    }
    DistributeTokenPeriodly ();
  }

  // @brief Real-time update pitQueue delete timeout Interest
  for (PitQueue::iterator ientry = pitQueue.begin ();
       ientry != pitQueue.end ();) {
    // Time out
    if ((Time.GetNowTimeMS () - ientry->second.GetReceiveTime ()) >= 2000) {
//cout << "this interest is timeout" << endl;
      pitQueue.erase (ientry++);
    } else {
      ++ientry;
    }
  }


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////                                                                                  //////
//////                                                                                  //////
//////                                                                                  //////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////                                                                                  //////
//////                              Send Interest Packets                               //////
//////                                                                                  //////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

    // Add the number of "numberOfPit"
//    numberOfPit += 1.0;

    // Insert this pit::Entry into pitQueue
//    pitQueue.insert (make_pair (&interest, ns3::ndn::PitQueue (Time.GetNowTimeMS (), &inFace)));

    // Consume the tokens of inFace
//    TokenConsume (&inFace);

    // Limit sending interest packets
    for (InterfaceList::iterator ientry = interfaceList.begin ();
         ientry != interfaceList.end (); ientry++) {
      if (&inFace == ientry->first) {
//cout << "ER's TIME : " << NOWTIME << " ";
//cout << "PITs : " << numberOfPit <<" The token of this Interface is : " << ientry->second.ReturnToken () << " ";
//cout << "Interface Number : " << interfaceList.size ();
//cout << "Interface Einf is : " << ientry->second.ReturnEinf () <<endl;
        if (ientry->second.ReturnToken () > 1e-6) {
          // Insert this pit::Entry into pitQueue
          numberOfPit += 1.0;
          pitQueue.insert (make_pair (&interest, ns3::ndn::PitQueue (Time.GetNowTimeMS (), &inFace)));
          this->sendInterest (pitEntry, outFace, interest);
          TokenConsume (&inFace);
        } else {
        }
        break;
      } else {
        continue;
      }
    }


    NFD_LOG_DEBUG (interest << " from=" << inFace.getId()
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
MTBFSER_PR::afterReceiveNack(const Face& inFace, const lp::Nack& nack,
                                    const shared_ptr<pit::Entry>& pitEntry)
{
  this->processNack(inFace, nack, pitEntry);
}

void
MTBFSER_PR::beforeSatisfyInterest ( const shared_ptr< pit::Entry > &pitEntry
                                  , const Face &inFace, const Data &data)
{
  TokenReward (&data);
  CalculateWhenInterestHit (pitEntry);
  Strategy::beforeSatisfyInterest (pitEntry, inFace, data);
}

void 
MTBFSER_PR::afterContentStoreHit ( const shared_ptr< pit::Entry > &pitEntry
                                 , const Face &inFace, const Data &data)
{
  TokenReward (&data);
  CalculateWhenInterestHit (pitEntry);
  Strategy::afterContentStoreHit (pitEntry, inFace, data);
}

void
MTBFSER_PR::onDroppedInterest (const Face &outFace, const Interest &interest) 
{
  PitQueue::iterator icount;
  for (PitQueue::iterator ientry = pitQueue.begin ();
       ientry != pitQueue.end (); ientry++) {
    if (ientry->first == &interest) {
      ientry->second.SetTimeOutDifference ();
      // Find corresponding interface
      for (InterfaceList::iterator icount = interfaceList.begin ();
           icount != interfaceList.end (); icount++) {
        if (ientry->second.GetFace () == icount->first) {
          icount->second.AddTimeOutTentry ();
          break;
        }
      }
      // Add the Eentry into Interface
cout << "This interest is timeout" << endl;
      icount = ientry;
      break;
    }
  }
  pitQueue.erase (icount);

  Strategy::onDroppedInterest (outFace, interest);
}

void
MTBFSER_PR::afterReceiveData (const shared_ptr <pit::Entry> &pitEntry,
                              const Face &inFace, const Data &data) {
  for (InterfaceList::iterator ientry = interfaceList.begin ();
       ientry != interfaceList.end (); ientry++) {
    if (ientry->first == &inFace) {
      ientry->second.SetToken (ientry->second.ReturnToken () - 1.0);
      if (ientry->second.ReturnToken () - 1 <= 1e-6) {
        // When this interface's token is 0 we can send Data
        return;
      } else {
        break;
      }
    }
  }

  Strategy::afterReceiveData (pitEntry, inFace, data);
}

void
MTBFSER_PR::CalculateWhenInterestHit (const shared_ptr<pit::Entry> & pitEntry) {
  for (PitQueue::iterator ientry = pitQueue.begin ();
       ientry != pitQueue.end (); ) {
    if (ientry->first->getName () == pitEntry->getInterest ().getName ()) {
      ientry->second.SetHitTime (Time.GetNowTimeMS ());
      for (InterfaceList::iterator icount = interfaceList.begin ();
           icount != interfaceList.end (); icount++) {
        if (icount->first == ientry->second.GetFace ()) {
          icount->second.AddTentry (ientry->second.GetTimeDifference ());

//cout << "ER TimeDifference is :" << ientry->second.GetTimeDifference () << endl;

          break;
        }
      }
      // Have got the timeDifference in class PitQueue set it into class Interface
//cout << "The timeDifference is " << ientry->second.GetTimeDifference () << endl;
      pitQueue.erase (ientry++);
    } else {
      ientry++;
    }
  }
  //pitQueue.erase (icount);
}

bool
MTBFSER_PR::IfAPeriod () {
  if (Time.GetNowTimeMS () - perPeriodTime >= onePeriodTime) {
    perPeriodTime = Time.GetNowTimeMS ();
    return true;
  } else {
    return false;
  }
}

void
MTBFSER_PR::DistributeTokenPeriodly () {

  double sumEinf = 0.0;

  for (InterfaceList::iterator ientry = interfaceList.begin ();
       ientry != interfaceList.end (); ientry++) {
    sumEinf += ientry->second.ReturnEdp ();
  }

//cout << "TIME : " << NOWTIME << " " << "The sumEinf of ER is " << sumEinf << endl;

  for (InterfaceList::iterator ientry = interfaceList.begin ();
       ientry != interfaceList.end (); ientry++) {
//cout << "The PITs of this node is " << numberOfPit <<" The token of this Interface is " << ientry->second.ReturnToken () << endl;
    if (numberOfPit <= LOWPIT) {
      // @brief this is the situation in first time
      if (sumEinf <= 1e-6) {
        ientry->second.SetToken (NumberOfTokens / interfaceList.size ());
      } else {
        ientry->second.SetToken ((ientry->second.ReturnEdp () / sumEinf) * NumberOfTokens * thresholdB
                                + (1.0 - thresholdB) * ientry->second.ReturnToken ());
      }
    } else if (numberOfPit < UPPIT) {
      if (sumEinf <= 1e-6) {
        ientry->second.SetToken (NumberOfTokens / interfaceList.size ());
      } else {
        ientry->second.SetToken (
                                  (ientry->second.ReturnEdp () / sumEinf) 
                                * NumberOfTokens 
                                * ((UPPIT - numberOfPit) / (UPPIT - LOWPIT))
                                * thresholdB
                                + (1.0 - thresholdB)
                                * ientry->second.ReturnToken ());
      }
    } else {
      ientry->second.SetToken (ientry->second.ReturnToken () * (1.0 - thresholdB));
    }
  }
  numberOfPit = 0.0;
}

// It can plus anyway but can't reduce less than 0
void 
MTBFSER_PR::TokenConsume (const Face* face) {
  for (InterfaceList::iterator ientry = interfaceList.begin ();
       ientry != interfaceList.end (); ientry++)
  {
    if (face == ientry->first && ientry->second.ReturnToken () > 1e-6) {
      ientry->second.SetToken (ientry->second.ReturnToken () - 1.0);
      break;
    }
  }
  return;
}

void
MTBFSER_PR::TokenReward (const Data * data) {

  for (PitQueue::iterator ientry = pitQueue.begin ();
       ientry != pitQueue.end (); ientry++) {
    // This interest can be hitted by data
    if (ientry->first->getName () == data->getName ()) {
      // @brief here can use map.find () function to replace , but I still wouldn't lol ...
      for (InterfaceList::iterator icount = interfaceList.begin ();
           icount != interfaceList.end (); icount++) {
        if (ientry->second.GetFace () == icount->first) {
          // @brief if type is 23 which is means this Data have varified
          {
            ns3::ndn::Srand srand = ns3::ndn::Srand ();
            if (srand.IfEventHappen (icount->second.ReturnPverconj (THRESHOLDV))) {
//cout << "have verified" << endl;
              // @brief This Data should be verified
              if (data->getSignature ().getKeyLocator ().getName ().toUri () == "/100") {
                // @brief This Data has been verified so reword a token
                if (icount->second.ReturnToken () <= 900) {
                  icount->second.SetToken (icount->second.ReturnToken () + 2.0);
                } else {}
              } else {
                icount->second.SetToken (icount->second.ReturnToken () / 2.0);
                // Corresponding Interface's NFV plus one
                icount->second.AddNfv ();
              }
            } else {
              // @brief This Data should not be verified 
              // And we hypothesis it's a good one so reword corresponding interface a token
//cout << "verified down" << endl;
              if (icount->second.ReturnToken () <= 900) {
                icount->second.SetToken (icount->second.ReturnToken () + 2.0);
              } else {}
            }
          }
          break;
        }
      }
    }
  }
}

} // namespace fw
} // namespace nfd
