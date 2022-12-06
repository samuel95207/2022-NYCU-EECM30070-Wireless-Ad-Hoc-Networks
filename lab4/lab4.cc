/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 University of Washington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */



/*
        LAB Assignment #4

        Solution by: Konstantinos Katsaros (K.Katsaros@surrey.ac.uk)
        based on wifi-simple-adhoc-grid.cc
*/

// The default layout is like this, on a 2-D grid.
//
// n20  n21  n22  n23  n24
// n15  n16  n17  n18  n19
// n10  n11  n12  n13  n14
// n5   n6   n7   n8   n9
// n0   n1   n2   n3   n4
//
// the layout is affected by the parameters given to GridPositionAllocator;
// by default, GridWidth is 5 and numNodes is 25..
//
// Flow 1: 0->24
// Flow 2: 20->4


#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "myapp.h"
#include "ns3/aodv-module.h"
#include "ns3/config-store-module.h"
#include "ns3/core-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/wifi-module.h"

NS_LOG_COMPONENT_DEFINE("Lab4");

using namespace ns3;



int main(int argc, char *argv[]) {
    std::string phyMode("DsssRate1Mbps");
    double distance = 500;  //(m)
    uint32_t topologySize = 5;
    uint32_t numNodes = topologySize * topologySize;  // 5x5
    double interval = 0.01;                           // seconds(Default = 0.001)
    uint32_t packetSize = 500;                        // bytes(Default = 600)
    uint32_t numPackets = 10000;                      // 1 vs 10000
    std::string rtslimit = "1500";                    //(Default = 1000000)
    std::string routingProtocal = "DSDV";
    int numFlow = 2;
    CommandLine cmd;

    cmd.AddValue("phyMode", "Wifi Phy mode", phyMode);
    cmd.AddValue("distance", "distance (m)", distance);
    cmd.AddValue("packetSize", "distance (m)", packetSize);
    cmd.AddValue("rtslimit", "RTS/CTS Threshold (bytes)", rtslimit);
    cmd.Parse(argc, argv);
    // Convert to time object
    Time interPacketInterval = Seconds(interval);

    // turn off RTS/CTS for frames below 2200 bytes
    Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue(rtslimit));
    // Fix non-unicast data rate to be the same as that of unicast
    Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));

    NodeContainer c;
    c.Create(numNodes);

    // The below set of helpers will help us to put together the wifi NICs we want
    WifiHelper wifi;

    YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
    // set it to zero; otherwise, gain will be added
    wifiPhy.Set("RxGain", DoubleValue(-10));
    // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
    wifiPhy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
    wifiPhy.SetChannel(wifiChannel.Create());

    // Add a non-QoS upper mac, and disable rate control
    NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default();
    wifi.SetStandard(WIFI_PHY_STANDARD_80211b);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue(phyMode), "ControlMode",
                                 StringValue(phyMode));
    // Set it to adhoc mode
    wifiMac.SetType("ns3::AdhocWifiMac");
    NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, c);

    MobilityHelper mobility;
    /*----------------------------------------------- */
    // 可以參考lab3 part2
    // set position allocator
    // your code
    mobility.SetPositionAllocator("ns3::GridPositionAllocator", "MinX", DoubleValue(0), "MinY", DoubleValue(0),
                                  "DeltaX", DoubleValue(distance), "DeltaY", DoubleValue(distance), "GridWidth",
                                  UintegerValue(topologySize), "LayoutType", StringValue("ColumnFirst"));
    /*----------------------------------------------- */
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(c);

    // Enable Routing Protocol (AODV/DSDV)
    OlsrHelper olsr;
    AodvHelper aodv;
    DsdvHelper dsdv;

    Ipv4ListRoutingHelper list;

    if (routingProtocal == "DSDV") {
        list.Add(dsdv, 10);  // install Protocol to node
    } else if (routingProtocal == "AODV") {
        list.Add(aodv, 10);  // install Protocol to node
    } else if (routingProtocal == "OLSR") {
        list.Add(olsr, 10);  // install Protocol to node
    }

    InternetStackHelper internet;
    internet.SetRoutingHelper(list);  // has effect on the next Install ()
    internet.Install(c);

    Ipv4AddressHelper ipv4;
    NS_LOG_INFO("Assign IP Addresses.");
    ipv4.SetBase("10.1.1.0", "255.255.255.0");  // 給予ipv4address
    Ipv4InterfaceContainer ifcont = ipv4.Assign(devices);

    // Create Apps
    uint16_t sinkPort = 6;  // use the same for all apps


    // flow 1
    //  UDP connection from N0 to N24
    Address sinkAddress1(InetSocketAddress(ifcont.GetAddress(24), sinkPort));  // interface of n24
    PacketSinkHelper packetSinkHelper1("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), sinkPort));
    ApplicationContainer sinkApps1 = packetSinkHelper1.Install(c.Get(24));  // n24 as sink
    sinkApps1.Start(Seconds(0.));
    sinkApps1.Stop(Seconds(100.));

    Ptr<Socket> ns3UdpSocket1 = Socket::CreateSocket(c.Get(0), UdpSocketFactory::GetTypeId());  // source at n0

    // Create UDP application at n0
    Ptr<MyApp> app1 = CreateObject<MyApp>();
    app1->Setup(ns3UdpSocket1, sinkAddress1, packetSize, numPackets, DataRate("1Mbps"));
    c.Get(0)->AddApplication(app1);
    app1->SetStartTime(Seconds(31.));
    app1->SetStopTime(Seconds(100.));


    if (numFlow == 2) {
        // flow 2
        //  UDP connection from N20 to N4
        /*----------------------------------------------- */
        // your code
        Address sinkAddress2(InetSocketAddress(ifcont.GetAddress(4), sinkPort));  // interface of n4
        PacketSinkHelper packetSinkHelper2("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), sinkPort));
        ApplicationContainer sinkApps2 = packetSinkHelper2.Install(c.Get(4));  // n4 as sink
        sinkApps2.Start(Seconds(0.));
        sinkApps2.Stop(Seconds(100.));

        Ptr<Socket> ns3UdpSocket2 = Socket::CreateSocket(c.Get(20), UdpSocketFactory::GetTypeId());  // source at n20
        /*----------------------------------------------- */


        // Create UDP application at n20
        /*----------------------------------------------- */
        // your code
        Ptr<MyApp> app2 = CreateObject<MyApp>();
        app2->Setup(ns3UdpSocket2, sinkAddress2, packetSize, numPackets, DataRate("1Mbps"));
        c.Get(20)->AddApplication(app2);
        app2->SetStartTime(Seconds(31.));
        app2->SetStopTime(Seconds(100.));
        /*----------------------------------------------- */
    }


    // Install FlowMonitor on all nodes
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    std::ostringstream ssPcapFilename;
    ssPcapFilename << "lab-4-solved-" << routingProtocal << "-" << numPackets << "-" << numFlow;
    wifiPhy.EnablePcap(ssPcapFilename.str().c_str(), devices);

    Simulator::Stop(Seconds(100.0));
    Simulator::Run();


    int Tx_Packets_1 = 0;
    int Rx_Packets_1 = 0;
    double Avg_e2eDelay_sec_1 = 0.0;
    double Avg_SystemThroughput_bps_1 = 0.0;
    double Avg_PDR_1 = 0;
    int Avg_Base_1 = 0;

    int Tx_Packets_2 = 0;
    int Rx_Packets_2 = 0;
    double Avg_e2eDelay_sec_2 = 0.0;
    double Avg_SystemThroughput_bps_2 = 0.0;
    double Avg_PDR_2 = 0;
    int Avg_Base_2 = 0;


    // Print per flow statistics
    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator counter = stats.begin(); counter != stats.end();
         ++counter) {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(counter->first);

        // Print per flow statistics
        /*implement flow monitor */
        /*----------------------------------------------- */
        // your code
        // 需先判斷source address跟destination addresses是否為我們需要的
        // Ex:if(t.sourceAddress == Ipv4Address("??.?.?.?.?") && t.destinationAddress == Ipv4Address("??.?.?.??"))
        if (t.sourceAddress == ifcont.GetAddress(0) && t.destinationAddress == ifcont.GetAddress(24)) {
            Tx_Packets_1 += counter->second.txPackets;
            Rx_Packets_1 += counter->second.rxPackets;
            Avg_e2eDelay_sec_1 += (double)counter->second.delaySum.GetSeconds() / (double)counter->second.rxPackets;
            Avg_SystemThroughput_bps_1 += (double)counter->second.rxBytes * 8 /
                                          (double)(counter->second.timeLastRxPacket.GetSeconds() -
                                                   counter->second.timeFirstTxPacket.GetSeconds());
            Avg_PDR_1 += ((double)(counter->second.rxPackets) / (double)counter->second.txPackets);
            Avg_Base_1++;
        } else if (t.sourceAddress == ifcont.GetAddress(20) && t.destinationAddress == ifcont.GetAddress(4)) {
            Tx_Packets_2 += counter->second.txPackets;
            Rx_Packets_2 += counter->second.rxPackets;
            Avg_e2eDelay_sec_2 += (double)counter->second.delaySum.GetSeconds() / (double)counter->second.rxPackets;
            Avg_SystemThroughput_bps_2 += (double)counter->second.rxBytes * 8 /
                                          (double)(counter->second.timeLastRxPacket.GetSeconds() -
                                                   counter->second.timeFirstTxPacket.GetSeconds());
            Avg_PDR_2 += ((double)(counter->second.rxPackets) / (double)counter->second.txPackets);
            Avg_Base_2++;
        }
        /*----------------------------------------------- */
    }

    monitor->SerializeToXmlFile("lab-4.flowmon", true, true);


    std::cout << "Lab3 Simulation Start\n\n";

    std::cout << routingProtocal << " " << numFlow << " Flow:" << std::endl;
    std::cout << "Flow ID 1 Src Addr " << ifcont.GetAddress(0) << " Dst Addr " << ifcont.GetAddress(24) << std::endl;

    Avg_SystemThroughput_bps_1 = Avg_SystemThroughput_bps_1 / Avg_Base_1;
    Avg_PDR_1 = Avg_PDR_1 / Avg_Base_1;
    Avg_e2eDelay_sec_1 = Avg_e2eDelay_sec_1 / Avg_Base_1;

    std::cout << "Tx Packets: " << Tx_Packets_1 << std::endl;
    std::cout << "Rx Packets: " << Rx_Packets_1 << std::endl;
    std::cout << "Avg PDR: " << Avg_PDR_1 << std::endl;
    std::cout << "Avg System Throughput: " << Avg_SystemThroughput_bps_1 / 1024 << " [Kbps]" << std::endl;
    std::cout << "Avg Delay: " << Avg_e2eDelay_sec_1 << std::endl;

    if (numFlow == 2) {
        std::cout << std::endl;
        std::cout << "Flow ID 2 Src Addr " << ifcont.GetAddress(20) << " Dst Addr " << ifcont.GetAddress(4)
                  << std::endl;

        Avg_SystemThroughput_bps_2 = Avg_SystemThroughput_bps_2 / Avg_Base_2;
        Avg_PDR_2 = Avg_PDR_2 / Avg_Base_2;
        Avg_e2eDelay_sec_2 = Avg_e2eDelay_sec_2 / Avg_Base_2;

        std::cout << "Tx Packets: " << Tx_Packets_2 << std::endl;
        std::cout << "Rx Packets: " << Rx_Packets_2 << std::endl;
        std::cout << "Avg PDR: " << Avg_PDR_2 << std::endl;
        std::cout << "Avg System Throughput: " << Avg_SystemThroughput_bps_2 / 1024 << " [Kbps]" << std::endl;
        std::cout << "Avg Delay: " << Avg_e2eDelay_sec_2 << std::endl;
    }

    Simulator::Destroy();

    return 0;
}
