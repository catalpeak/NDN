

#include "MTBFSCR_HJ.h"
#include "algorithm.hpp"
#include "core/logger.hpp"

#include <iostream>

using namespace std;

namespace nfd {
namespace fw  {

NFD_REGISTER_STRATEGY (MTBFSCRHJ);

NFD_LOG_INIT ("MTBFSCRHJ");

const time::milliseconds MTBFSCRHJ::RETX_SUPPRESSION_INITIAL(10);
const time::milliseconds MTBFSCRHJ::RETX_SUPPRESSION_MAX(250);

MTBFSCRHJ::MTBFSCRHJ (Forwarder& forwarder, const Name& name)
  : Strategy (forwarder)
  , ProcessNackTraits (this)
  , m_retxSuppression ( RETX_SUPPRESSION_INITIAL
                      , RetxSuppressionExponential::DEFAULT_MULTIPLIER
                      , RETX_SUPPRESSION_MAX)
  , NumberOfTokens (600.0)
  , perPeriodTime  (0.0)
  , onePeriodTime  (1000.0)
  , numberOfPit    (0.0)
  , LOWPIT         (250.0)
  , UPPIT          (900.0)
  , NOWTIME        (0.0)
  , thresholdB     (0.5)
  , ca             ()
{
  ParsedInstanceName parsed = parseInstanceName(name);
  if (!parsed.parameters.empty()) {
    BOOST_THROW_EXCEPTION(std::invalid_argument("MulticastStrategy does not accept parameters"));
  }
  if (parsed.version && *parsed.version != getStrategyName()[-1].toVersion()) {
    BOOST_THROW_EXCEPTION(std::invalid_argument(
      "MTBFSCR does not support version " + to_string(*parsed.version)));
  }
  this->setInstanceName(makeInstanceName(name, getStrategyName()));

  fstream config ("config.txt");
  string buffer;
  char BUFFER [256];
  config.getline (BUFFER, 100);
  NodeName = BUFFER;
  config.close ();
  config.open  ("config.txt");

  // Delete the fisrt line of "config.txt" 
  system ("touch temp.txt");
  fstream temp ("temp.txt");
  bool FirstLine = true;
  while (!config.eof ()) {
    getline (config, buffer);
    if  (FirstLine) {
      FirstLine = false;
      continue;
    } else {
      temp << buffer << endl;
    }
  }

  temp.close ();
  config.close ();
  system ("rm config.txt");
  system ("mv temp.txt config.txt");

//  cout << NodeName << endl;

}

const Name&
MTBFSCRHJ::getStrategyName () {
  static Name strategyName ("/localhost/nfd/strategy/MTBFSCRHJ/%FD%03");
  return strategyName;
}

void 
MTBFSCRHJ::afterReceiveInterest (const Face& inFace, const Interest& interest,
                             const shared_ptr<pit::Entry>& pitEntry)
{

//Record neighbor Core Router's Interface

if (interest.getName ().toUri ().compare (0, 11 ,"/TrustModel") == 0) {
  // This inFace sending a TrustModel interest, so add This interface 
  // into neighborCr map , to make sure Bucket Token won't react to it
//  neighborCr.insert (make_pair (&inFace, 0.0));
}
//cout << "The Length of neighborCr is " << neighborCr.size () << endl;

//////////////////////////////////////////
////////     Trust Model Field    ////////
//////////////////////////////////////////

  if (ifHaveBadInFace.JudgeBad ()) {
    // There is a bad Face we should drop it 
    if (ifHaveBadInFace.ReturnFace () == &inFace) {
      // And This Face is The bad one FUCK IT !!!
      // Test If has FUCK it last for a period
      if (Time.GetNowTimeMS () - ifHaveBadInFace.ReturnTime () > onePeriodTime) {
        // Turn This inFace into a good one
        ifHaveBadInFace.GoodOrBad (false);
      }
      ns3::ndn::MineTime time;
      if (time.GetNowTimeS () >= 30 && time.GetNowTimeS () <= 70) {
        
      } else {
//        return;
      }
    }
  }

//////////////////////////////////////////
////////   Trust Model Finished   ////////
//////////////////////////////////////////

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

  ca.NIrAddOne ();

  // @brief Period Checking strategy lueluelue :)
  if (IfAPeriod ()) {

//cout << "This CR's Interest is " << numberOfPit << endl;

    NOWTIME++;
//cout << "TIME : " << NOWTIME << "The number of PIT is " << numberOfPit << endl;
    DistributeTokenPeriodly ();

    for (InterfaceList::iterator ientry = interfaceList.begin ();
         ientry != interfaceList.end (); ientry++) {
      ientry->second.SetEinf ();
    }

    // @brief CA's data saves
    // Now have got all four parameters in one period
    ca.SetNIs (ca.GetNIr () - ca.GetCSHit ());

    // Write All Datas into "TrustModel.txt" document
    // First delete the corressponding line in document
    // Then insert datas
    system ("touch temp.txt");
    fstream temp ("temp.txt");
    fstream TrustModel ("TrustModel.txt");
    char buffer [256];
    while (!TrustModel.eof ()) {
      TrustModel.getline (buffer, 100);
      string BUFFER = buffer;
      if (BUFFER.compare (0, NodeName.size (), NodeName) == 0) {
        continue;
        // This line should be deleted
      } else {
        // This line should be insert into temp document
        temp << BUFFER << endl;
      }
    }
    temp.close ();
    TrustModel.close ();

    system ("rm TrustModel.txt");
    system ("mv temp.txt TrustModel.txt");

    // Record CA's value this period into preCA for comparing
    preCA = ca;

    TrustModel.open ("TrustModel.txt", ios::app);
    TrustModel << NodeName << " " << ca.ReturnValue () << endl;
    TrustModel.close ();

  }

  // @brief When receive a /TrustModel interest do something
  if  (interest.getName ().toUri ().compare (0, 11 ,"/TrustModel") == 0) {
    // This interest is a TrustModel verify interest
    ns3::ndn::CA trustmodel;
    trustmodel.SetNDs (interest.getCongestionMark () / 1000000000);
    trustmodel.SetNDr (interest.getCongestionMark () % 1000000000 / 1000000);
    trustmodel.SetNIs (interest.getCongestionMark () % 1000000 / 1000);
    trustmodel.SetNIr (interest.getCongestionMark () % 1000);
     // Use This router's CA to compare with interest's CA
    
    if (trustmodel.TestForCA (preCA, TEST_FOR_NIR) == false) {
//      cout << NodeName << " FUCK" << endl;
      ifHaveBadInFace.GoodOrBad (true);
      ifHaveBadInFace.SetInFace (inFace);
      ifHaveBadInFace.SetTime   (Time.GetNowTimeMS ());
    } 

    return ;
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

    // Check If This inFace is from neighbor core router
    bool IfInFaceIsNeighborCr = false;
    for (NeighborCr::iterator ientry = neighborCr.begin (); ientry != neighborCr.end (); ientry++) {
      if (ientry->first == &inFace) {
        IfInFaceIsNeighborCr = true;
      } else {
        continue;
      }
    }

    // Limit sending interest packets
    for (InterfaceList::iterator ientry = interfaceList.begin ();
         ientry != interfaceList.end (); ientry++) {
      if (&inFace == ientry->first) {
//cout << "The PITs of CR is " << numberOfPit <<"The token of this Interface is " << ientry->second.ReturnToken () << endl;
        if (ientry->second.ReturnToken () > 1e-6 || IfInFaceIsNeighborCr) {
          // Insert this pit::Entry into pitQueue
          numberOfPit += 1.0;
          pitQueue.insert (make_pair (&interest, ns3::ndn::PitQueue (Time.GetNowTimeMS (), &inFace)));
          ns3::ndn::MineTime time;
          if (interest.getName ().toUri ().compare (0, 4, "/bad") == 0 && 
              time.GetNowTimeS () >= 30 && time.GetNowTimeS () <= 70 ||
              time.GetNowTimeS () < 30 || time.GetNowTimeS () > 70) {
            this->sendInterest (pitEntry, outFace, interest);
          }
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
MTBFSCRHJ::afterReceiveNack(const Face& inFace, const lp::Nack& nack,
                                    const shared_ptr<pit::Entry>& pitEntry)
{
  this->processNack(inFace, nack, pitEntry);
}

void
MTBFSCRHJ::beforeSatisfyInterest ( const shared_ptr< pit::Entry > &pitEntry
                             , const Face &inFace, const Data &data)
{
  TokenReward (&data);
  CalculateWhenInterestHit (pitEntry);
  Strategy::beforeSatisfyInterest (pitEntry, inFace, data);
}

void 
MTBFSCRHJ::afterContentStoreHit ( const shared_ptr< pit::Entry > &pitEntry
                            , const Face &inFace, const Data &data)
{
  TokenReward (&data);
  CalculateWhenInterestHit (pitEntry);
  ca.CSHitAddOne ();
  Strategy::afterContentStoreHit (pitEntry, inFace, data);
}

void
MTBFSCRHJ::onDroppedInterest (const Face &outFace, const Interest &interest) 
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
//cout << "This interest is timeout" << endl;
      icount = ientry;
      break;
    }
  }
  pitQueue.erase (icount);

  Strategy::onDroppedInterest (outFace, interest);
}

void
MTBFSCRHJ::CalculateWhenInterestHit (const shared_ptr<pit::Entry> & pitEntry) {
  for (PitQueue::iterator ientry = pitQueue.begin ();
       ientry != pitQueue.end (); ) {
    if (ientry->first->getName () == pitEntry->getInterest ().getName ()) {
      ientry->second.SetHitTime (Time.GetNowTimeMS ());
      for (InterfaceList::iterator icount = interfaceList.begin ();
           icount != interfaceList.end (); icount++) {
        if (icount->first == ientry->second.GetFace ()) {
          icount->second.AddTentry (ientry->second.GetTimeDifference ());
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
MTBFSCRHJ::IfAPeriod () {
  if (Time.GetNowTimeMS () - perPeriodTime >= onePeriodTime) {
    perPeriodTime = Time.GetNowTimeMS ();
    return true;
  } else {
    return false;
  }
}

void
MTBFSCRHJ::DistributeTokenPeriodly () {

  double sumEinf = 0.0;

  for (InterfaceList::iterator ientry = interfaceList.begin ();
       ientry != interfaceList.end (); ientry++) {
    sumEinf += ientry->second.ReturnEinf ();
  }

//cout << "The sumEinf of CR is " << sumEinf << endl;

  for (InterfaceList::iterator ientry = interfaceList.begin ();
       ientry != interfaceList.end (); ientry++) {
//cout << "The PITs of this node is " << numberOfPit <<" The token of this Interface is " << ientry->second.ReturnToken () << endl;
    if (numberOfPit <= LOWPIT) {
      // @brief this is the situation in first time
      if (sumEinf <= 1e-6) {
        ientry->second.SetToken (NumberOfTokens / interfaceList.size ());
      } else {
        ientry->second.SetToken (((ientry->second.ReturnEinf () / sumEinf) * NumberOfTokens) * thresholdB 
                                + ((1.0 - thresholdB) * ientry->second.ReturnToken ()));
      }
    } else if (numberOfPit < UPPIT) {
      if (sumEinf <= 1e-6) {
        ientry->second.SetToken (NumberOfTokens / interfaceList.size ());
      } else {
        ientry->second.SetToken (
                                  (ientry->second.ReturnEinf () / sumEinf) 
                                * NumberOfTokens 
                                * ((UPPIT - numberOfPit) / (UPPIT - LOWPIT))
                                * thresholdB
                                + (1.0 - thresholdB)
                                * ientry->second.ReturnToken ());
      }
    } else {
      ientry->second.SetToken ((1.0 - thresholdB) * ientry->second.ReturnToken ());
    }
  }
  numberOfPit = 0.0;
}

// It can plus anyway but can't reduce less than 0
void 
MTBFSCRHJ::TokenConsume (const Face* face) {
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
MTBFSCRHJ::TokenReward (const Data * data) {

  for (PitQueue::iterator ientry = pitQueue.begin ();
       ientry != pitQueue.end (); ientry++) {
    // This interest can be hitted by data
    if (ientry->first->getName () == data->getName ()) {
      // @brief here can use map.find () function to replace , but I still wouldn't lol ...
      for (InterfaceList::iterator icount = interfaceList.begin ();
           icount != interfaceList.end (); icount++) {
        if (ientry->second.GetFace () == icount->first) {
          icount->second.SetToken (icount->second.ReturnToken () + 1.0);
          break;
        }
      }
    }
  }
}

void
MTBFSCRHJ::afterReceiveData ( const shared_ptr <pit::Entry> &pitEntry
                          , const Face & inFace, const Data & data) {


// before TrustModel we should use token
  for (InterfaceList::iterator ientry = interfaceList.begin ();
       ientry != interfaceList.end (); ientry++) {
    if (ientry->first == &inFace) {
      ientry->second.SetToken (ientry->second.ReturnToken () - 1.0);
      if (ientry->second.ReturnToken () <= 1e-6) {
        return;
      } else {
        break;
      }
    }
  }


// Data's TrustModel is same as interest
  if (ifHaveBadInFace.JudgeBad ()) {
    // There is a bad Face we should drop it 
    if (ifHaveBadInFace.ReturnFace () == &inFace) {
      // And This Face is The bad one FUCK IT !!!
      // Test If has FUCK it last for a period
      if (Time.GetNowTimeMS () - ifHaveBadInFace.ReturnTime () > onePeriodTime) {
        // Turn This inFace into a good one
        ifHaveBadInFace.GoodOrBad (false);
      }
      return ;
    }
  }

  ca.NDsAddOne ();
  ca.NDrAddOne ();
  Strategy::afterReceiveData (pitEntry, inFace, data);

}

} // namespace fw
} // namespace nfd
