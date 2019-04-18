


#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/ndnSIM-module.h>

#include <iostream>
#include <string>

// These head I include is useless but it just for telling you what heads I used in programmer
#include "ConsumerZipf.h"
#include "MProducer.h"
#include "BadProducer.h"
#include "MTBFS.h"

using namespace std;

using namespace ns3;
using namespace ns3::ndn;

int main (int argc, char* argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

//  string topology = "big-tree";
  string topology = "test";
  string prefixName = "/";

  system ("touch config.txt");
  system ("touch TrustModel.txt");
  fstream config;

  // Read topology 
  AnnotatedTopologyReader topologyReader ("", 3.0);
  topologyReader.SetFileName ("topology/" + topology + ".txt");
  topologyReader.Read ();

///////////////////////////////////////////////////////////////////////
///////		Separate all different kinds of nodes		///////
///////////////////////////////////////////////////////////////////////
  NodeContainer lf;
  NodeContainer pr;
  NodeContainer cr;
  NodeContainer erl;
  NodeContainer erp;

  NodeContainer badlf;
  NodeContainer badpr;
  NodeContainer testlf;

  config.open ("config.txt");

  bool IfFirst_lf = true;
  bool IfFirst_pr = true;

  for_each (NodeList::Begin (), NodeList::End (), [&] (Ptr<Node> node)
 {
    if (  Names::FindName (node).compare (0, 5, "lf-10") == 0
       || Names::FindName (node).compare (0, 5, "lf-11") == 0
       || Names::FindName (node).compare (0, 5, "lf-20") == 0) {
      testlf.Add (node);
    } else if (Names::FindName (node).compare (0, 2, "pr") == 0) {
      if (IfFirst_pr) {
        pr.Add (node);
        IfFirst_pr = false;
      } else {
        badpr.Add (node);
      }
    } else if (Names::FindName (node).compare (0, 2, "cr") == 0) {
      cr.Add (node);
      AppHelper TrustModelHelper ("ns3::ndn::TrustModel");
      TrustModelHelper.SetPrefix (prefixName + "TrustModel");
      TrustModelHelper.SetAttribute ("Frequency", StringValue ("1"));
      TrustModelHelper.SetAttribute ("AppName", StringValue (Names::FindName (node)));
      TrustModelHelper.Install (node);
      config << Names::FindName (node) << "\n";
    } else if (Names::FindName (node).compare (0, 3, "erl") == 0) {
      erl.Add (node);
    } else if (Names::FindName (node).compare (0, 3, "erp") == 0) {
      erp.Add (node);
    } else if (Names::FindName (node).compare (0, 2, "lf") == 0) {
      // We may need not only one aggressor but more
      if (IfFirst_lf) {
        badlf.Add (node);
        IfFirst_lf = false;
      }
      // This Line is just for Big Topology
        else if (Names::FindName (node).compare (0, 4, "lf-4") == 0 ||
                 Names::FindName (node).compare (0, 4, "lf-5") == 0 ||
                 Names::FindName (node).compare (0, 4, "lf-7") == 0 ||
                 Names::FindName (node).compare (0, 4, "lf-8") == 0 ||
                 Names::FindName (node).compare (0, 5, "lf-12") == 0 ||
                 Names::FindName (node).compare (0, 5, "lf-15") == 0 ||
                 Names::FindName (node).compare (0, 5, "lf-19") == 0) {
        badlf.Add (node);
      }
        else {
        lf.Add (node);
      }
    }
  });

  config.close ();

  // Install CS for different nodes
  StackHelper ndnStackHelper1;
  ndnStackHelper1.SetOldContentStore ( "ns3::ndn::cs::slru", "MaxSize", "50");
  ndnStackHelper1.Install (erl);
  ndnStackHelper1.Install (erp);
  ndnStackHelper1.Install (cr);

  StackHelper ndnStackHelper2;
  ndnStackHelper2.SetOldContentStore ("ns3::ndn::cs::Nocache");
  ndnStackHelper2.Install (lf);
  ndnStackHelper2.Install (pr);
  ndnStackHelper2.Install (badlf);
  ndnStackHelper2.Install (badpr);
  ndnStackHelper2.Install (testlf);

  // Install FIB
  GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();
  ndnGlobalRoutingHelper.AddOrigins (prefixName, pr);
  ndnGlobalRoutingHelper.AddOrigins (prefixName, badpr);
  GlobalRoutingHelper::CalculateRoutes ();
/*
  Ptr<Node> er_1 = Names::Find<Node> ("er-1");
  Ptr<Node> er_2 = Names::Find<Node> ("er-2");
  Ptr<Node> er_3 = Names::Find<Node> ("er-3");
  Ptr<Node> er_4 = Names::Find<Node> ("er-4");
  Ptr<Node> er_5 = Names::Find<Node> ("er-5");
  Ptr<Node> er_6 = Names::Find<Node> ("er-6");
*/
  // Install FW for different nodes
//  StrategyChoiceHelper::InstallAll ("/", "localhost/nfd/strategy/MTBFS");
  StrategyChoiceHelper::Install (cr, "/", "localhost/nfd/strategy/MTBFSCR");
  StrategyChoiceHelper::Install (erl, "/", "localhost/nfd/strategy/MTBFSER");
  StrategyChoiceHelper::Install (erp, "/", "localhost/nfd/strategy/MTBFSER_PR");
  StrategyChoiceHelper::Install (pr, "/", "localhost/nfd/strategy/multicast");
  StrategyChoiceHelper::Install (lf, "/", "localhost/nfd/strategy/multicast");

  StrategyChoiceHelper::Install (badpr, "/", "localhost/nfd/strategy/multicast");
  StrategyChoiceHelper::Install (badlf, "/", "localhost/nfd/strategy/multicast"); 
  StrategyChoiceHelper::Install (testlf, "/", "localhost/nfd/strategy/MTBFS");

/*
//  StrategyChoiceHelper::Install (er_1, "/", "localhost/nfd/strategy/multicast");
//  StrategyChoiceHelper::Install (er_2, "/", "localhost/nfd/strategy/multicast");
//  StrategyChoiceHelper::Install (er_3, "/", "localhost/nfd/strategy/multicast");
//  StrategyChoiceHelper::Install (er_4, "/", "localhost/nfd/strategy/multicast");
//  StrategyChoiceHelper::Install (er_5, "/", "localhost/nfd/strategy/multicast");
//  StrategyChoiceHelper::Install (er_6, "/", "localhost/nfd/strategy/multicast");

  StrategyChoiceHelper::Install (er_1, "/", "localhost/nfd/strategy/MTBFSER_PR");
  StrategyChoiceHelper::Install (er_2, "/", "localhost/nfd/strategy/MTBFSER");
  StrategyChoiceHelper::Install (er_3, "/", "localhost/nfd/strategy/MTBFSER_PR");
  StrategyChoiceHelper::Install (er_4, "/", "localhost/nfd/strategy/MTBFSER");
  StrategyChoiceHelper::Install (er_5, "/", "localhost/nfd/strategy/MTBFSER_PR");
  StrategyChoiceHelper::Install (er_6, "/", "localhost/nfd/strategy/MTBFSER");
*/

  // Define consumer's APPs for different Data Consumer 
  AppHelper consumerHelper ("ns3::ndn::ConsumerZipf");
  consumerHelper.SetPrefix (prefixName);
  consumerHelper.SetAttribute ("Frequency", StringValue ("50"));

  // Define Producer's APPs for different Data provider
  AppHelper producerHelper ("ns3::ndn::MProducer");
  producerHelper.SetPrefix (prefixName);
  producerHelper.SetAttribute ("PayloadSize", StringValue ("1100"));

  // Unvarified data
  AppHelper badproducerHelper ("ns3::ndn::BadProducer");
  badproducerHelper.SetPrefix (prefixName);
  badproducerHelper.SetAttribute ("PayloadSize", StringValue ("1100"));

  // cache pollution + interest flooding
  AppHelper badconsumerHelper ("ns3::ndn::ConsumerCbr");
  badconsumerHelper.SetPrefix (prefixName + "bad");
//  badconsumerHelper.SetAttribute ("Frequency", StringValue ("5424"));
  badconsumerHelper.SetAttribute ("Frequency", StringValue ("400"));

  // Interest flooding
  AppHelper IFconsumerHelper1 ("ns3::ndn::ConsumerCbr");
  IFconsumerHelper1.SetPrefix (prefixName + "bad");
//  IFconsumerHelper1.SetAttribute ("Frequency", StringValue ("2712"));
  IFconsumerHelper1.SetAttribute ("Frequency", StringValue ("200"));

  AppHelper IFconsumerHelper2 ("ns3::ndn::ConsumerZipf");
  IFconsumerHelper2.SetPrefix (prefixName + "bad");
//  IFconsumerHelper2.SetAttribute ("Frequency", StringValue ("2712"));
  IFconsumerHelper2.SetAttribute ("Frequency", StringValue ("200"));

  // Install consumer APPs on leaf nodes
  ApplicationContainer consumer = consumerHelper.Install (lf);
  consumer.Start (Seconds (0.0));
  consumer.Stop  (Seconds (100.0));
//  consumer.Stop (Seconds (10.0));

  ApplicationContainer testconsumer = consumerHelper.Install (testlf);
  testconsumer.Start (Seconds (0.0));
  testconsumer.Stop  (Seconds (100.0));
//  testconsumer.Stop (Seconds (10.0));


  // IF + Cache Pollution
  ApplicationContainer badconsumer = badconsumerHelper.Install (badlf);
  badconsumer.Start (Seconds (30.0));
  badconsumer.Stop  (Seconds (70.0));
//  badconsumer.Start (Seconds (3.0));
//  badconsumer.Stop (Seconds (7.0));

  // Interest Flooding Attack

  ApplicationContainer IFconsumer1  = IFconsumerHelper1.Install (badlf);
  IFconsumer1.Start (Seconds (30.0));
  IFconsumer1.Stop  (Seconds (70.0));
//  IFconsumer1.Start (Seconds (3.0));
//  IFconsumer1.Stop  (Seconds (7.0));

  ApplicationContainer IFconsumer2  = IFconsumerHelper2.Install (badlf);
  IFconsumer2.Start (Seconds (30.0));
  IFconsumer2.Stop  (Seconds (70.0));
//  IFconsumer2.Start (Seconds (15.0));
//  IFconsumer2.Stop  (Seconds (35.0));
//  IFconsumer2.Start (Seconds (3.0));
//  IFconsumer2.Stop  (Seconds (7.0));

  // Install producer APPs on pr nodes 
  ApplicationContainer producer = producerHelper.Install (pr);
  producer.Start (Seconds (0.0));
  producer.Stop  (Seconds (30.0));
//  producer.Stop (Seconds (15.0));
//  producer.Stop (Seconds (3.0));

  ApplicationContainer producer1 = producerHelper.Install (badpr);
  producer1.Start (Seconds (0.0));
  producer1.Stop  (Seconds (30.0));
//  producer1.Stop (Seconds (15.0));
//  producer1.Stop (Seconds (3.0));

  // Cache Poison Attack
  ApplicationContainer badproducer = badproducerHelper.Install (badpr);
  badproducer.Start (Seconds (30.0));
  badproducer.Stop  (Seconds (70.0));
//  badproducer.Start (Seconds (15.0));
//  badproducer.Stop  (Seconds (35.0));
//  badproducer.Start (Seconds (3.0));
//  badproducer.Stop  (Seconds (7.0));

  ApplicationContainer producer2 = producerHelper.Install (pr);
  producer2.Start (Seconds (30.0));
  producer2.Stop  (Seconds (70.0));
//  producer2.Start (Seconds (15.0));
//  producer2.Start (Seconds (35.0));
//  producer2.Start (Seconds (3.0));
//  producer2.Start (Seconds (7.0));

  ApplicationContainer producer3 = producerHelper.Install (pr);
  producer3.Start (Seconds (70.0));
  producer3.Stop  (Seconds (100.0));
//  producer3.Start (Seconds (35.0));
//  producer3.Stop  (Seconds (50.0));
//  producer3.Start (Seconds (7.0));
//  producer3.Stop  (Seconds (10.0));

  ApplicationContainer producer4 = producerHelper.Install (badpr);
  producer4.Start (Seconds (70.0));
  producer4.Stop  (Seconds (100.0));
//  producer4.Start (Seconds (35.0));
//  producer4.Stop  (Seconds (50.0));
//  producer4.Start (Seconds (7.0));
//  producer4.Stop  (Seconds (10.0));

//  ApplicationContainer Producer = producerHelper.Install (badpr);

  // Catulate Interest & Data situation
  L3RateTracer::InstallAll ("results/" + topology + ".txt", Seconds (1.0));

  // Catulate CS situation
  CsTracer::InstallAll ("results/ContentStore.txt", Seconds (1.0));

  Simulator::Stop (Seconds (100.0));
//  Simulator::Stop (Seconds (50.0));
//  Simulator::Stop (Seconds (10.0));
  Simulator::Run  ();
  Simulator::Destroy ();

  L3RateTracer::Destroy ();

  system ("rm config.txt");
  system ("rm TrustModel.txt");

  return 0;
}
