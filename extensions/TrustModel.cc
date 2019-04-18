
// TrustModel.cc

#include "TrustModel.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/double.h"

NS_LOG_COMPONENT_DEFINE ("ndn.TrustModel");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED (TrustModel);

TypeId
TrustModel::GetTypeId (void) {
  static TypeId tid =
    TypeId ("ns3::ndn::TrustModel")
      .SetGroupName ("Ndn")
      .SetParent <Consumer> ()
      .AddConstructor <TrustModel> () 
      .AddAttribute ( "Frequency", "Frequency of interest packets"
                    , StringValue ("1.0")
                    , MakeDoubleAccessor (&TrustModel::m_frequency)
                    , MakeDoubleChecker <double> ())
      .AddAttribute ( "AppName" , "The Name of this App"
                    , StringValue ("/")
                    , MakeStringAccessor (&TrustModel::AppName)
                    , MakeStringChecker ())
  ;
  return tid;
}

TrustModel::TrustModel ()
  : m_frequency (1.0)
  , m_firstTime (true)
  , IfFirstTime (true)
{
  NS_LOG_FUNCTION_NOARGS();
}

TrustModel::~TrustModel () {

}

void
TrustModel::ScheduleNextPacket () {

  if (m_firstTime) {
    m_sendEvent = Simulator::Schedule (Seconds (0.0), &TrustModel::SendPacket, this);
    m_firstTime = false;
  } else

  if (!m_sendEvent.IsRunning ()) {
    m_sendEvent = Simulator::Schedule (Seconds (1.0 / m_frequency), &TrustModel::SendPacket, this);
  }

}

void
TrustModel::SendPacket () {

  if (!m_active) {
    return ;
  }

  // Send CA to each neighbor
  NS_LOG_FUNCTION_NOARGS();

  uint32_t seq = 1.0;

  // Read data from TrustModel 
  fstream trustModel ("TrustModel.txt");
  char buffer [256];
  while (!trustModel.eof ()) {
    trustModel.getline (buffer, 100);
    string BUFFER = buffer;
    if (BUFFER.compare (0, AppName.size (), AppName) == 0) {
      // This line the line which I need
      break;
    } else {
      // This line is not the right line
      continue;
    }
  }

  trustModel.close ();

  // Insert the CA into interet

  char save [100];
  int ia = 0;
  for (int ib = 5; buffer [ib] != '\0'; ia++, ib++) {
    save [ia] = buffer [ib];
  }
  save [ia] = '\0';
  char CA [4][12];
  int icount = 0;
  for (int iout = 0; iout < 4; iout++) {
    for (int iin = 0; iin < 12; iin++) {
      if (save [icount] != ' ') {
        CA [iout][iin] = save [icount++];
      } else {
        CA [iout][iin] = '\0';
        ++icount;
        break;
      }
    }
  }

  // extract string to int
  uint64_t value [4];
  for (int i = 0; i < 4; i++)
    value [i] = 0;
  for (int ivalue = 0; ivalue < 4; ivalue++) {
    for (int iin = 0; CA [ivalue][iin] != '\0' && CA [ivalue][iin] != '.'; iin++) {
      value [ivalue] = value [ivalue] * 10 + CA [ivalue][iin] - '0';
    }
  }

  uint64_t mark = value [0] * 1000000000
                + value [1] * 1000000
                + value [2] * 1000
                + value [3];

  // Get the value of CA and save it into CongestionMark

  shared_ptr <Name> _name = make_shared <Name> (m_interestName);
  _name->appendSequenceNumber(seq);

  shared_ptr <Interest> interest = make_shared <Interest> ();
  interest->setNonce (m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setName (*_name);
  time::milliseconds interestLifeTime (m_interestLifeTime.GetMilliSeconds ());
  interest->setInterestLifetime (interestLifeTime);
  interest->setCongestionMark (mark);

  NS_LOG_INFO ("FUCK");

  WillSendOutInterest(seq);

  m_transmittedInterests(interest, this, m_face);
  m_appLink->onReceiveInterest(*interest);

  ScheduleNextPacket();

}

} // namespace ndn
} // namespace ns3
