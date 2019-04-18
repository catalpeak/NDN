

#include "ConsumerZipf.h"

#include <math.h>

#include "utils/Srand.h"

NS_LOG_COMPONENT_DEFINE ("ndn.ConsumerZipf");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED (ConsumerZipf);

TypeId
ConsumerZipf::GetTypeId (void) {
  static TypeId tid = 
    TypeId ("ns3::ndn::ConsumerZipf")
      .SetGroupName ("Ndn")
      .SetParent<ConsumerCbr> ()
      .AddConstructor<ConsumerZipf> ()

      .AddAttribute ( "NumberOfContents"
                    , "Number of the Contents in total"
                    , StringValue ("100")
                    , MakeUintegerAccessor ( &ConsumerZipf::SetNumberOfContents
                                           , &ConsumerZipf::GetNumberOfContents)
                    , MakeUintegerChecker<uint32_t> ())

      .AddAttribute ( "q", "parameter of improve rank"
                    , StringValue ("0.7")
                    , MakeDoubleAccessor ( &ConsumerZipf::SetQ
                                         , &ConsumerZipf::GetQ)
                    , MakeDoubleChecker<double> ())

      .AddAttribute ( "s", "parameter of power"
                    , StringValue ("0.7")
                    , MakeDoubleAccessor ( &ConsumerZipf::SetS
                                         , &ConsumerZipf::GetS)
                    , MakeDoubleChecker<double> ())
    ;

  return tid;

}

ConsumerZipf::ConsumerZipf ()
  : m_N (100)
  , m_q (0.7)
  , m_s (0.7)
  , m_seqRng (CreateObject<UniformRandomVariable> ())
    // @brief The Probability for IF excluding this data
  , ProbabilityOfExclude (0.1)
  , IfWrongData (false)
{
}

ConsumerZipf::~ConsumerZipf ()
{
}

void
ConsumerZipf::SetNumberOfContents (uint32_t numOfContents) {

  m_N = numOfContents;

  NS_LOG_DEBUG (m_q << " and " << m_s << " and " << m_N);

  m_Pcum = std::vector<double> (m_N + 1);

  m_Pcum[0] = 0.0;
  for (uint32_t i = 1; i <= m_N; i++) {
    m_Pcum[i] = m_Pcum[i - 1] + 1.0 / std::pow (i + m_q, m_s);
  }

  for (uint32_t i = 1; i <= m_N; i++) {
    m_Pcum[i] = m_Pcum[i] / m_Pcum[m_N];
    NS_LOG_LOGIC("Cumulative probability [" << i << "]=" << m_Pcum[i]);
  }
}

uint32_t
ConsumerZipf::GetNumberOfContents () const
{
  return m_N;
}

void
ConsumerZipf::SetQ (double q)
{
  m_q = q;
  SetNumberOfContents (m_N);
}

double
ConsumerZipf::GetQ () const
{
  return m_q;
}

void
ConsumerZipf::SetS (double s)
{
  m_s = s;
  SetNumberOfContents (m_N);
}

double
ConsumerZipf::GetS () const
{
  return m_s;
}

void
ConsumerZipf::SendPacket () {
  if (!m_active) {
    return;
  }

  NS_LOG_FUNCTION_NOARGS();

  // @brief The last Data is a wrong one , so send a fail-verify Interest , AND set IfWrongData = true
  if (IfWrongData) {
    Name _name = Name ("Feedback");
    shared_ptr <Name> name = make_shared <Name> (_name);

    shared_ptr <Interest> _interest = make_shared <Interest> ();
    _interest->setNonce (m_rand->GetValue (0, std::numeric_limits<uint32_t>::max()));
    _interest->setName (*name);

    // Send this interest
    m_transmittedInterests(_interest, this, m_face);
    m_appLink->onReceiveInterest(*_interest);
    ConsumerZipf::ScheduleNextPacket ();

    IfWrongData = false;
    return ;
  }

  uint32_t seq = std::numeric_limits<uint32_t>::max();

  while (m_retxSeqs.size ()) {
    seq = *m_retxSeqs.begin();
    m_retxSeqs.erase(m_retxSeqs.begin());
    NS_LOG_DEBUG("=interest seq " << seq << " from m_retxSeqs");
    break;
  }

  if (seq == std::numeric_limits<uint32_t>::max ())
  {
    if (m_seqMax != std::numeric_limits<uint32_t>::max ()) {
      if (m_seq >= m_seqMax) {
        return;
      }
    }

    seq = ConsumerZipf::GetNextSeq ();
    m_seq++;
  }

  // Create one Interest & send it
  shared_ptr<Name> nameWithSequence = make_shared<Name> (m_interestName);
  nameWithSequence->appendSequenceNumber (seq);

  shared_ptr<Interest> interest = make_shared<Interest> ();
  interest->setNonce (m_rand->GetValue (0, std::numeric_limits<uint32_t>::max()));
  interest->setName(*nameWithSequence);

  // The Exclude of Interest is judged by application layer so I use a probability to simulate it
  Srand srand = Srand ();
  if (srand.IfEventHappen (ProbabilityOfExclude)) {
    // @brief This Interest have an Exclude for excluding a corresponding Data
    // But what the Data is which is not important , so nothing need be saved
    name::Component component  = name::Component ("Exclude");
    Exclude exclude = Exclude();
    exclude.excludeOne (component);
    interest->setExclude (exclude);
  } else  {
    // do nothing
  }

  NS_LOG_INFO("> Interest for " << seq << ", Total: " << m_seq << ", face: " << m_face->getId());
  NS_LOG_DEBUG("Trying to add " << seq << " with " << Simulator::Now() << ". already "
                                << m_seqTimeouts.size() << " items");

  m_seqTimeouts.insert(SeqTimeout(seq, Simulator::Now()));
  m_seqFullDelay.insert(SeqTimeout(seq, Simulator::Now()));

  m_seqLastDelay.erase(seq);
  m_seqLastDelay.insert(SeqTimeout(seq, Simulator::Now()));

  m_seqRetxCounts[seq]++;

  m_rtt->SentSeq(SequenceNumber32(seq), 1);

  m_transmittedInterests(interest, this, m_face);
  m_appLink->onReceiveInterest(*interest);

  ConsumerZipf::ScheduleNextPacket ();

}

uint32_t
ConsumerZipf::GetNextSeq () {

  uint32_t content_index = 1;
  double p_sum = 0;

  double p_random = m_seqRng->GetValue ();
  while (p_random == 0) {
    p_random = m_seqRng->GetValue ();
  }
  NS_LOG_LOGIC ("p_random=" << p_random);
  for (uint32_t i = 1; i <= m_N; i++) {
    p_sum = m_Pcum[i];
    if (p_random <= p_sum) {
      content_index = i;
      break;
    }
  }
  NS_LOG_DEBUG ("RandomNumber=" << content_index);
  return content_index;
}

void
ConsumerZipf::ScheduleNextPacket () {
  if (m_firstTime) {
    m_sendEvent = Simulator::Schedule ( Seconds (0.0)
                                      , &ConsumerZipf::SendPacket
                                      , this);
    m_firstTime = false;
  } else if (!m_sendEvent.IsRunning ()) {
    m_sendEvent = Simulator::Schedule ( (m_random == 0)
                                      ? Seconds (1.0 / m_frequency)
                                      : Seconds (m_random->GetValue ())
                                      , &ConsumerZipf::SendPacket
                                      , this);
  }
}

void 
ConsumerZipf::OnData (shared_ptr<const Data> data) {
  // When receive a Data as the terminal , it should verify it 
  if (data->getSignature ().hasKeyLocator ()) {
    // @brief This Data is a good one
    // Do nothing
//cout << "Have received a good Data" << endl;
  } else {
    // @brief This Data is fail-verify and send a fail message back
    IfWrongData = true;
    data_ = data;
//cout << "FUCK this is a bad one" << endl;
  }

//  cout << "On Data" << endl;

}

} // namespace ndn
} // namespace ns3
