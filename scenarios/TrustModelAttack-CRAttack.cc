
// TrustModel.cc

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/ndnSIM-module.h>

#include <iostream>
#include <string>

using namespace std;

using namespace ns3;
using namespace ns3::ndn;

int main (int argc, char * argv []) 
{

  CommandLine cmd;
  cmd.Parse (argc, argv);

  system ("touch config.txt");
  system ("touch TrustModel.txt");
  fstream config;

  string topology   = "test";
  string prefixName = "/";

  AnnotatedTopologyReader topologyReader ("", 3.0);
  topologyReader.SetFileName ("topology/" + topology + ".txt");
  topologyReader.Read ();

  NodeContainer lf;
  NodeContainer pr;
  NodeContainer cr;
  NodeContainer erl;
  NodeContainer erp;

  NodeContainer testlf;

  config.open ("config.txt");

int _count_ = 0;

  for_each (NodeList::Begin (), NodeList::End (), [&] (Ptr <Node> node) {
_count_++;
    if (Names::FindName (node).compare (0, 5, "lf-21") == 0 ||
        Names::FindName (node).compare (0, 4, "lf-6") == 0) {
      testlf.Add (node);
    } else
    if (Names::FindName (node).compare (0, 2, "pr") == 0) {
      pr.Add (node);
    } else if (Names::FindName (node).compare (0, 2, "cr") == 0) {
      if (Names::FindName (node).compare (0, 4, "cr-1") != 0 &&
          Names::FindName (node).compare (0, 4, "cr-5") != 0 ) {
      cr.Add (node);
      AppHelper TrustModelHelper ("ns3::ndn::TrustModel");
      TrustModelHelper.SetPrefix (prefixName + "TrustModel");
      TrustModelHelper.SetAttribute ("Frequency", StringValue ("1"));
      TrustModelHelper.SetAttribute ("AppName", StringValue (Names::FindName (node)));
      TrustModelHelper.Install (node);
      config << Names::FindName (node) << "\n";
      }
    } else if (Names::FindName (node).compare (0, 3, "erl") == 0) {
      erl.Add (node);
    } else if (Names::FindName (node).compare (0, 3, "erp") == 0) {
      erp.Add (node);
    } else if (Names::FindName (node).compare (0, 2, "lf") == 0) {
      if (Names::FindName (node).compare (0, 4, "lf-6") != 0 &&
          Names::FindName (node).compare (0, 5, "lf-21") != 0) {
        lf.Add (node);
      } else {

      }
    }
  });  

cr.Add (Names::Find <Node> ("cr-10"));
cr.Add (Names::Find <Node> ("cr-11"));
cr.Add (Names::Find <Node> ("cr-12"));
cr.Add (Names::Find <Node> ("cr-13"));

cout << "The number of nodes is : " << _count_ << endl;

  config.close ();

  // This Er is a evil edge router
  NodeContainer evilCR;
  evilCR.Add (Names::Find <Node> ("cr-1"));
  evilCR.Add (Names::Find <Node> ("cr-5"));

//  NodeContainer leaf1;
//  leaf1.Add (Names::Find <Node> ("lf-1"));

  StackHelper ndnStackHelper1;
  ndnStackHelper1.SetOldContentStore ( "ns3::ndn::cs::slru", "MaxSize", "50");
  ndnStackHelper1.Install (erl);
  ndnStackHelper1.Install (erp);
  ndnStackHelper1.Install (cr);
  ndnStackHelper1.Install (evilCR);

cout << "FUCK3"  << endl;

cout << "TestLF size is : " << testlf.size () << endl;
cout << "lf size is : " << lf.size () << endl;
cout << "cr size is : " << cr.size () << endl;
cout << "evilcr size is : " << evilCR.size () << endl;
cout << "pr size is : " << pr.size () << endl;
cout << "erl size is : " << erl.size () << endl;
cout << "erp size is : " << erp.size () << endl;

  StackHelper ndnStackHelper2;
  ndnStackHelper2.SetOldContentStore ("ns3::ndn::cs::Nocache");
  ndnStackHelper2.Install (lf);
  ndnStackHelper2.Install (pr);
//  ndnStackHelper2.Install (leaf1);
  ndnStackHelper2.Install (testlf);

cout << "MUSTN't SHOE !!!" << endl;

  GlobalRoutingHelper ndnGlobalRoutingHelper;
cout <<"one" << endl;
  ndnGlobalRoutingHelper.InstallAll ();
cout <<"two" << endl;
  ndnGlobalRoutingHelper.AddOrigins (prefixName, pr);
cout <<"three" << endl;
  GlobalRoutingHelper::CalculateRoutes ();

cout << "FUCK2"  << endl;

  StrategyChoiceHelper::Install (pr, prefixName, "localhost/nfd/strategy/multicast");
  StrategyChoiceHelper::Install (cr, prefixName, "localhost/nfd/strategy/MTBFSCR");
  StrategyChoiceHelper::Install (erl, prefixName, "localhost/nfd/strategy/MTBFSER");
  StrategyChoiceHelper::Install (erp, prefixName, "localhost/nfd/strategy/MTBFSER_PR");
  StrategyChoiceHelper::Install (lf, prefixName, "localhost/nfd/strategy/multicast");

  StrategyChoiceHelper::Install (evilCR, prefixName, "localhost/nfd/strategy/MTBFSCRHJ");
  StrategyChoiceHelper::Install (testlf, prefixName, "localhost/nfd/strategy/MTBFS");

  AppHelper consumerHelper ("ns3::ndn::ConsumerZipf");
  consumerHelper.SetPrefix (prefixName);
  consumerHelper.SetAttribute ("Frequency", StringValue ("50"));

  AppHelper producerHelper ("ns3::ndn::MProducer");
  producerHelper.SetPrefix (prefixName);
  producerHelper.SetAttribute ("PayloadSize", StringValue ("1100"));

  AppHelper badconsumerHelper ("ns3::ndn::ConsumerCbr");
  badconsumerHelper.SetPrefix (prefixName + "bad");
  badconsumerHelper.SetAttribute ("Frequency", StringValue ("500"));// 204

cout << "FUCK1"  << endl;

  ApplicationContainer consumer = consumerHelper.Install (lf);
  consumer.Start (Seconds (0.0));
  consumer.Stop  (Seconds (100.0));

  ApplicationContainer consumerTest = consumerHelper.Install (testlf);
  consumerTest.Start (Seconds (0.0));
  consumerTest.Stop  (Seconds (100.0));

  ApplicationContainer badconsumer = badconsumerHelper.Install (evilCR);
  badconsumer.Start (Seconds (30.0));
  badconsumer.Stop  (Seconds (70.0));

  ApplicationContainer producer = producerHelper.Install (pr);
  //producer.Start (Seconds (0.0));
  //producer.Stop  (Seconds (100.0));

  L3RateTracer::InstallAll ("results/" + topology + ".txt", Seconds (1.0));
  CsTracer::InstallAll ("results/ContentStore.txt", Seconds (1.0));

  Simulator::Stop (Seconds (100.0));
  Simulator::Run  ();
  Simulator::Destroy ();

  L3RateTracer::Destroy ();

  system ("rm config.txt");
  system ("rm TrustModel.txt");


  return 0;

}
