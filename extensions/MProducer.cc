
#include "MProducer.h"

#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

#include "model/ndn-l3-protocol.hpp"
#include "helper/ndn-fib-helper.hpp"

#include "utils/Srand.h"

#include <memory>
#include <iostream>

using namespace std;

NS_LOG_COMPONENT_DEFINE("ndn.MProducer");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(MProducer);

TypeId 
MProducer::GetTypeId (void) {
  static TypeId tid = 
    TypeId ("ns3::ndn::MProducer")
	.SetGroupName ("Ndn")
	.SetParent<App> ()
	.AddConstructor<MProducer> ()
	.AddAttribute ("Prefix", "Prefix, for which producer has the data", StringValue ("/"),
			MakeNameAccessor (&MProducer::m_prefix), MakeNameChecker ())
	.AddAttribute ("Postfix", "Postfix that is added to the output data (e.g., for adding producer-uniqueness)",
			StringValue ("/"), MakeNameAccessor (&MProducer::m_postfix), MakeNameChecker ())
	.AddAttribute ("PayloadSize", "Virtual payload size for Content packets", 
			UintegerValue (1024), MakeUintegerAccessor (&MProducer::m_virtualPayloadSize),
			MakeUintegerChecker<uint32_t> ())
	.AddAttribute ("Freshness", "Freshness of data packets, if 0, then unlimited freshness", 
			TimeValue (Seconds (0.0)), MakeTimeAccessor (&MProducer::m_freshness),
			MakeTimeChecker ())
	.AddAttribute ("Signature", "Fake signature, 0 valid signature (default), other values application-specific", 
			UintegerValue (0.0), MakeUintegerAccessor (&MProducer::m_signature),
			MakeUintegerChecker<uint32_t> ())
	.AddAttribute ("KeyLocator", "Name to be used for key locator.  If root, then key locator is not used", 
			NameValue (), MakeNameAccessor (&MProducer::m_keyLocator), MakeNameChecker ())
	;
  return tid;
}

MProducer::MProducer () 
  : m_signature      (100)
  , m_congestionMark (100)
// @brief good data-producer's signature type is  23 and m_congestionMark is 100
// I use m_congestionMark as Exclude field
{
  NS_LOG_FUNCTION_NOARGS();
}

void
MProducer::StartApplication () {
  NS_LOG_FUNCTION_NOARGS();
  App::StartApplication ();

  FibHelper::AddRoute (GetNode (), m_prefix, m_face, 0);
}

void
MProducer::StopApplication () {
  NS_LOG_FUNCTION_NOARGS();

  App::StopApplication ();
}

void
MProducer::OnInterest (shared_ptr<const Interest> interest) {
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

  m_keyLocator = Name ("100");

  if (m_keyLocator.size () > 0) {
    signatureInfo.setKeyLocator (m_keyLocator);
//cout << "This KeyLocator has been onload" << endl;
  }

  signature.setInfo (signatureInfo);
  signature.setValue ( ::ndn::makeNonNegativeIntegerBlock ( ::ndn::tlv::SignatureValue, m_signature ) );

  data->setSignature (signature);

  data->setCongestionMark (m_congestionMark);

  NS_LOG_INFO ("node(" << GetNode ()->GetId () << ") responding with Data: " << data->getName ());

  data->wireEncode ();

  m_transmittedDatas (data, this, m_face);
  m_appLink->onReceiveData (*data);

//cout << "FUCK" << endl;

}

} // namespace ndn
} // namespace ns3

