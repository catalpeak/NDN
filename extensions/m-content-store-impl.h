
// m-content-store-impl.h

#ifndef M_CONTENT_STORE_IMPL_H
#define M_CONTENT_STORE_IMPL_H

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/ndnSIM/model/cs/ndn-content-store.hpp"

#include "ns3/packet.h"
#include <boost/foreach.hpp>

#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"

#include "ns3/ndnSIM/utils/trie/trie-with-policy.hpp"

#include "utils/MineTime.h"
#include "utils/Srand.h"

#include <iostream>
#include <map>
#include <math.h>

using namespace std;

namespace ns3 {
namespace ndn {
namespace cs  {

template <class CS>
class MEntryImpl : public Entry {

public :

  typedef Entry base_type;

public :

  MEntryImpl (Ptr<ContentStore> cs, shared_ptr<const Data> data)
    : Entry (cs, data)
    , item_  (0)
  {
  }

  void 
  SetTrie (typename CS::super::iterator item) {
    item_ = item;
  }

  typename CS::super::iterator
  to_iterator () {
    return item_;
  }

  typename CS::super::const_iterator
  to_iterator () const {
    return item_;
  }

private :

  typename CS::super::iterator item_;

};

template <class Policy>
class MContentStoreImpl
  : public ContentStore
  , protected ndnSIM::
      trie_with_policy <Name, ndnSIM::smart_pointer_payload_traits <MEntryImpl<MContentStoreImpl<Policy> >, Entry>, Policy>
{

public :

  typedef ndnSIM::trie_with_policy <Name, ndnSIM::smart_pointer_payload_traits <MEntryImpl<MContentStoreImpl<Policy> >, Entry>, Policy>
          super;

  typedef MEntryImpl <MContentStoreImpl <Policy> > entry;

  static TypeId
  GetTypeId ();

  MContentStoreImpl () 
    : TIMEOUT (2000.0)
    , avg_Eentry (0.0)
    , var_Eentry (0.0)
    , str_Eentry (0.0)
    , sumEentry  (0.0)
    , perTime (0.0)
    , nowPeriodNumber (0.0)
    , nowPeriodTotalNumber (0.0)
  {
  };

  virtual ~MContentStoreImpl () {};

  virtual inline shared_ptr <Data>
  Lookup (shared_ptr <const Interest> interest);

  virtual inline bool
  Add (shared_ptr <const Data> data);

  virtual inline void
  Print (std::ostream & os) const;

  virtual uint32_t
  GetSize () const;

  virtual Ptr <Entry>
  Begin ();

  virtual Ptr <Entry>
  End ();

  virtual Ptr <Entry> 
  Next (Ptr <Entry>);


  const typename super::policy_container&
  GetPolicy () const {
    return super::getPolicy ();
  }


  typename super::policy_container&
  GetPolicy()
  {
    return super::getPolicy();
  }


public :

  typedef void (*CsEntryCallback) (Ptr <const Entry>);

private :

  void
  SetMaxSize (uint32_t maxSize) ;

  uint32_t
  GetMaxSize () const;

private :

  static LogComponent g_log;

  TracedCallback <Ptr <const Entry> > m_didAddEntry;

  typedef multimap < string , double> PITQUEUE;
  PITQUEUE pitQueue;

  MineTime time;

  double TIMEOUT;

  double avg_Eentry;
  double var_Eentry;
  double str_Eentry;

  double sumEentry;

  double perTime;

  double nowPeriodNumber;
  double nowPeriodTotalNumber;
};

template <class Policy>
LogComponent 
MContentStoreImpl <Policy>::g_log = LogComponent (("ndn.cs." + Policy::GetName ()).c_str (), __FILE__);

template <class Policy>
TypeId
MContentStoreImpl <Policy>::GetTypeId () {
  static TypeId tid = 
    TypeId (("ns3::ndn::cs::" + Policy::GetName ()).c_str ())
      .SetGroupName ("Ndn")
      .SetParent <ContentStore> ()
      .AddConstructor <MContentStoreImpl <Policy> > ()
      .AddAttribute ( "MaxSize"
                    , "Set maximum number of entries in ContentStore. If 0, limit is not enforced"
                    , StringValue ("100")
                    , MakeUintegerAccessor ( &MContentStoreImpl <Policy>::GetMaxSize
                                           , &MContentStoreImpl <Policy>::SetMaxSize)
                    , MakeUintegerChecker <uint32_t> ())
      .AddTraceSource ( "DidAddEntry"
                      , "Trace fired every time entry is successfully added to the cache"
                      , MakeTraceSourceAccessor (&MContentStoreImpl <Policy>::m_didAddEntry)
                      , "ns3::ndn::cs::ContentStoreImpl::CsEntryCallback")
  ;

  return tid;

}

struct isNotExcluded {
  inline isNotExcluded (const Exclude & exclude)
    : m_exclude (exclude)
  {
  }

  bool
  operator () (const name::Component & comp) const {
    return !m_exclude.isExcluded (comp);
  }

  private :

    const Exclude & m_exclude;

};

// OnInterest
template <class Policy>
shared_ptr <Data>
MContentStoreImpl <Policy>::Lookup (shared_ptr <const Interest> interest)
{
  NS_LOG_FUNCTION(this << interest->getName());

  typename super::const_iterator node;

////////////////////////////////////////////////////////////////////////////////////////
////////			     On Interest				////////
////////////////////////////////////////////////////////////////////////////////////////

  pitQueue.insert (make_pair (interest->getName ().toUri (), time.GetNowTimeMS ()));

  for (PITQUEUE::iterator ientry = pitQueue.begin ();
       ientry != pitQueue.end ();) {
    if (time.GetNowTimeMS () - ientry->second >= TIMEOUT) {
      pitQueue.erase (ientry++);
    } else {
      ++ientry;
    }
  }

  if (interest->getExclude ().empty ()) {
    node = this->deepest_prefix_match(interest->getName());
  } else {
    node = this->deepest_prefix_match_if_next_level(interest->getName(),
                                                    isNotExcluded(interest->getExclude()));
  }

  if (node != this->end ()) {

    if (false == node->payload ()->GetData ()->getSignature ().hasKeyLocator ()) {
      // This Data is not through verify
      this->m_cacheMissesTrace (interest);
      return 0;
    }

    if (interest->getName ().toUri ().compare (0, 4, "/bad") != 0)
    {
      this->m_cacheHitsTrace (interest, node->payload ()->GetData ());
    }

    shared_ptr<Data> copy = make_shared <Data> (*node->payload ()->GetData ());
    return copy;
  } else {
    this->m_cacheMissesTrace (interest);
    return 0;
  }
}

// OnData
template <class Policy>
bool
MContentStoreImpl <Policy>::Add (shared_ptr <const Data> data)
{
  NS_LOG_FUNCTION(this << data->getName());

  nowPeriodTotalNumber += 1.0;
  // Have been a period
  if (time.GetNowTimeMS () - perTime >= 1000.0) {
    sumEentry = 0;
    avg_Eentry = 0;
    var_Eentry = 0;
    perTime = time.GetNowTimeMS ();
//    cout << time.GetNowTimeS () << "\t" << nowPeriodNumber / nowPeriodTotalNumber << endl;
//    cout << time.GetNowTimeS () << "\t" << nowPeriodNumber << endl;
    nowPeriodNumber = 0.0;
    nowPeriodTotalNumber = 0.0;
  }

  double ThisDataEentry = 0.0;
  double Eentry = 0.0;
  for (PITQUEUE::iterator ientry = pitQueue.begin ();
       ientry != pitQueue.end ();) {
    if (data->getName ().toUri () == ientry->first) {
      ThisDataEentry = time.GetNowTimeMS () - ientry->second;
      sumEentry += 1.0;
      //avg_Eentry = (avg_Eentry * (sumEentry - 1) + ThisDataEentry) / sumEentry;
      //var_Eentry = (var_Eentry * (sumEentry - 1) + pow ((ThisDataEentry - avg_Eentry), 2.0)) / sumEentry;
      // Here is better to put outside but I wouldn't
      //str_Eentry  = pow (var_Eentry, 0.5);
      pitQueue.erase (ientry++);
    } else {
      ++ientry;
    }
  }

/*
  if (ThisDataEentry <= avg_Eentry) {
    Eentry = pow ((1.0 - ThisDataEentry / (avg_Eentry * 3)), 0.5);
  } else if (3 * avg_Eentry <= TIMEOUT && ThisDataEentry <= 3 * avg_Eentry) {
    Eentry = 1.0 - ThisDataEentry / (avg_Eentry * 3);
  } else if (TIMEOUT < 3 * avg_Eentry && ThisDataEentry <= TIMEOUT) {
    Eentry = 1.0 - ThisDataEentry / TIMEOUT;
  } else {
    Eentry = 0;
  }
*/

  double HALFTIMEOUT = TIMEOUT / 2;
  if (avg_Eentry < HALFTIMEOUT) {
    if (ThisDataEentry <= avg_Eentry && ThisDataEentry > 0) {
      Eentry = 1;
    } else if (ThisDataEentry <= HALFTIMEOUT) {
      Eentry = 1 - ThisDataEentry / TIMEOUT;
    } else if (ThisDataEentry < TIMEOUT) { 
      Eentry = pow ((1 - ThisDataEentry / TIMEOUT), 2.0);
    } else { // ThisDataEentry >= TIMEOUT
      Eentry = 0;
    }
  } else {
    if (ThisDataEentry < HALFTIMEOUT && ThisDataEentry > 0) {
      Eentry = 1;
    } else if (ThisDataEentry <= avg_Eentry) {
      Eentry = 1 - ThisDataEentry / TIMEOUT;
    } else if (ThisDataEentry < TIMEOUT) {
      Eentry = pow ((1 - ThisDataEentry / TIMEOUT), 2.0);
    } else {
      Eentry = 0;
    }
  }

  /*
  double threshlodC = avg_Eentry + str_Eentry;
  double dataProbability;
  if (Eentry < threshlodC) {
    dataProbability = Eentry;
  } else {
    dataProbability = 1.0;
  }
  */

//cout << "Eentry = " << Eentry << endl;
  ns3::ndn::Srand srand = ns3::ndn::Srand ();
  if  (srand.IfEventHappen (Eentry) == false) {
//cout << "This Data is dropped & Eentry = " << Eentry << endl;
    // This Data will not be saved
    return false;
  }

  Ptr <entry> newEntry = Create <entry> (this, data);
  std::pair<typename super::iterator, bool> result = super::insert (data->getName (), newEntry);

  if (result.first != super::end ()) {
    if (result.second) {
      newEntry->SetTrie (result.first);
      m_didAddEntry (newEntry);
      nowPeriodNumber += 1.0;
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

template <class Policy>
void
MContentStoreImpl <Policy>::Print (std::ostream & os) const {
  for (typename super::policy_container::const_iterator item = this->getPolicy ().begin ();
       item != this->getPolicy ().end (); item++) {
    os << item->payload ()->GetName () << std::endl;
  }
}

template <class Policy>
void
MContentStoreImpl <Policy>::SetMaxSize (uint32_t maxSize) {
  this->getPolicy ().set_max_size (maxSize);
}

template <class Policy>
uint32_t
MContentStoreImpl <Policy>::GetMaxSize () const {
  return this->getPolicy ().get_max_size ();
}

template <class Policy>
uint32_t
MContentStoreImpl <Policy>::GetSize () const {
  return this->getPolicy ().size ();
}

template <class Policy>
Ptr <Entry>
MContentStoreImpl <Policy>::Begin () {
  typename super::parent_trie::recursive_iterator item (super::getTrie ()), end (0); // such a amazing programmer
  for (; item != end; item++) {
    if (item->payload () == 0) {
      continue;
    }
    break;
  }

  if (item == end) {
    return End ();
  } else {
    return item->payload ();
  }
}

template <class Policy>
Ptr <Entry>
MContentStoreImpl <Policy>::End () {
  return 0;
}

template <class Policy>
Ptr <Entry>
MContentStoreImpl <Policy>::Next (Ptr <Entry> from) {
  if (from == 0) {
    return 0;
  }

  typename super::parent_trie::recursive_iterator item (*StaticCast <entry> (from)->to_iterator ()), end (0);

  for (item++; item != end; item++) {
    if (item->payload () == 0) {
      continue;
    }
    break;
  }

  if (item == end) {
    return End ();
  } else {
    return item->payload ();
  }

}

} // namespace cs
} // namespace ndn
} // namespace ns3

#endif // M_CONTENT_STORE_IMPL_H
