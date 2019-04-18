
#include "BadProducer.h"

#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

#include "model/ndn-l3-protocol.hpp"
#include "helper/ndn-fib-helper.hpp"

#include <memory>

#include <iostream>

using namespace std;

NS_LOG_COMPONENT_DEFINE("ndn.BadProducer");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(BadProducer);

TypeId 
BadProducer::GetTypeId (void) {
  static TypeId tid = 
    TypeId ("ns3::ndn::BadProducer")
	.SetGroupName ("Ndn")
	.SetParent<App> ()
	.AddConstructor<BadProducer> ()
	.AddAttribute ("Prefix", "Prefix, for which producer has the data", StringValue ("/"),
			MakeNameAccessor (&BadProducer::m_prefix), MakeNameChecker ())
	.AddAttribute ("Postfix", "Postfix that is added to the output data (e.g., for adding producer-uniqueness)",
			StringValue ("/"), MakeNameAccessor (&BadProducer::m_postfix), MakeNameChecker ())
	.AddAttribute ("PayloadSize", "Virtual payload size for Content packets", 
			UintegerValue (1024), MakeUintegerAccessor (&BadProducer::m_virtualPayloadSize),
			MakeUintegerChecker<uint32_t> ())
	.AddAttribute ("Freshness", "Freshness of data packets, if 0, then unlimited freshness", 
			TimeValue (Seconds (0.0)), MakeTimeAccessor (&BadProducer::m_freshness),
			MakeTimeChecker ())
	.AddAttribute ("Signature", "Fake signature, 0 valid signature (default), other values application-specific", 
			UintegerValue (0.0), MakeUintegerAccessor (&BadProducer::m_signature),
			MakeUintegerChecker<uint32_t> ())
	.AddAttribute ("KeyLocator", "Name to be used for key locator.  If root, then key locator is not used", 
			NameValue (), MakeNameAccessor (&BadProducer::m_keyLocator), MakeNameChecker ())
	;
  return tid;
}

BadProducer::BadProducer () 
  : m_congestionMark (100)
// @brief good data-producer's signature type is  23 and m_congestionMark is 100
// I use m_congestionMark as Exclude field
{
  m_keyLocator = Name ("100");
  NS_LOG_FUNCTION_NOARGS();
}

void
BadProducer::StartApplication () {
  NS_LOG_FUNCTION_NOARGS();
  App::StartApplication ();

  FibHelper::AddRoute (GetNode (), m_prefix, m_face, 0);
}

void
BadProducer::StopApplication () {
  NS_LOG_FUNCTION_NOARGS();

  App::StopApplication ();
}

void
BadProducer::OnInterest (shared_ptr<const Interest> interest) {
  App::OnInterest (interest);

  NS_LOG_FUNCTION (this << interest);

  if (!m_active)
   return;

  Name dataName (interest->getName ());

  auto data = make_shared<Data> ();
  data->setName (dataName);
  data->setFreshnessPeriod (::ndn::time::milliseconds (m_freshness.GetMilliSeconds ()));

  data->setContent (make_shared< ::ndn::Buffer > (m_virtualPayloadSize));

  Signature signature;
  SignatureInfo signatureInfo (static_cast< ::ndn::tlv::SignatureTypeValue > (255));

// @brief As a BadProducer which cause cache poison , the Datas which is sent by it will never own a KeyLocator

  m_keyLocator = Name ("50");
  if (m_keyLocator.size () > 0) {
    signatureInfo.setKeyLocator (m_keyLocator);
  }

  signature.setInfo (signatureInfo);
  // @brief bad Producer is the case of cache poison , thus it will send unsignature data packets .
  // I use ::ndn::tlv::Nonce (10) as bad signature and actually if a number is not 23 it can be used .
  signature.setValue ( ::ndn::makeNonNegativeIntegerBlock ( ::ndn::tlv::SignatureValue, m_signature ) );

  data->setSignature (signature);

//  data->setCongestionMark (m_congestionMark);

  NS_LOG_INFO ("node(" << GetNode ()->GetId () << ") responding with Data: " << data->getName ());

  data->wireEncode ();

  m_transmittedDatas (data, this, m_face);
  m_appLink->onReceiveData (*data);
//cout << "Bad FUCK" << endl;
}

} // namespace ndn
} // namespace ns3

