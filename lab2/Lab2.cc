#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cassert>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiSimpleInfra");


int main (int argc, char *argv[])
{

        std::string phyMode ("DsssRate1Mbps");
        double rss = -80;  // -dBm

        std::string CBR_Rate_bps = "860kbps";	// [ 860 , 960 , 1060 , 1160 ]
        uint32_t Queue_Size_Packet = 30;	// [ 20 , 30 , 40 , 50 ]

        NodeContainer c;
        c.Create (2);

        // Set wifi NIC
        WifiHelper wifi;
        wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

        YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
        // This is one parameter that matters when using FixedRssLossModel
        // set it to zero; otherwise, gain will be added
        wifiPhy.Set ("RxGain", DoubleValue (0) ); 
        // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
        wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO); 

        YansWifiChannelHelper wifiChannel;
        wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
        // The below FixedRssLossModel will cause the rss to be fixed regardless
        // of the distance between the two stations, and the transmit power
        wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue (rss));
        wifiPhy.SetChannel (wifiChannel.Create ());

        // Add a non-QoS upper mac, and disable rate control
        NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
        wifi.SetRemoteStationManager( "ns3::ConstantRateWifiManager" , "DataMode" , StringValue(phyMode) , "ControlMode" , StringValue(phyMode) );

        // Setup the rest of the upper mac
        Ssid ssid = Ssid ("wifi-default");

        // setup sta.
        wifiMac.SetType( "ns3::StaWifiMac" , "Ssid" , SsidValue (ssid) , "ActiveProbing", BooleanValue( false ) );
	
	
        //Config::SetDefault( "ns3::WifiMacQueue::Mode" , StringValue ("QUEUE_MODE_BYTES"));
        Config::SetDefault( "ns3::WifiMacQueue::MaxPacketNumber" , UintegerValue (Queue_Size_Packet));
	
        NetDeviceContainer staDevice = wifi.Install (wifiPhy, wifiMac, c.Get (0));
        NetDeviceContainer devices = staDevice;

        // setup ap.
        wifiMac.SetType( "ns3::ApWifiMac" , "Ssid" , SsidValue(ssid) );
        NetDeviceContainer apDevice = wifi.Install (wifiPhy, wifiMac, c.Get (1));
        devices.Add (apDevice);

        // Note that with FixedRssLossModel, the positions below are not 
        // used for received signal strength. 
        MobilityHelper mobility;
        Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
        positionAlloc->Add (Vector (0.0, 0.0, 0.0));
        positionAlloc->Add (Vector (100.0, 0.0, 0.0));
        mobility.SetPositionAllocator (positionAlloc);
        mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
        mobility.Install (c);

        InternetStackHelper internet;
        internet.Install (c);

        Ipv4AddressHelper ipv4;
        NS_LOG_INFO ("Assign IP Addresses.");
        ipv4.SetBase ("10.1.1.0", "255.255.255.0");
        Ipv4InterfaceContainer i = ipv4.Assign (devices);


    // Create FlowMonitor
    FlowMonitorHelper flowmon;
	Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    int CBR_PacketSize_Byte = 1000;
	int CBR_Port = 812;
	double Traffic_StartTime_sec = 1 , Traffic_EndTime_sec = 15;
	double TotalSimulationTime_sec = 30;
	ApplicationContainer CBR_Tx_App , CBR_Rx_App;

	// Transmitter
	for( int counter = 0 ; counter < 1 ; ++counter )
	{
		//set your transmitter(page 18 19)
	}

	// Receiver
	for( int counter = 0 ; counter < 1 ; ++counter )
	{
		//set your receiver(page 20)
	}
  
        Simulator::Stop (Seconds (TotalSimulationTime_sec));
        Simulator::Run ();

    // Show Statistics Information
	double Avg_e2eDelaySec = 0.0;
	int Total_RxByte = 0;
	double Avg_PDR = 0.0;
	int Avg_Base = 0;

	std::cout << "Lab2 Simulation Start\n\n";
    std::cout << "[ CBR Rate : " << CBR_Rate_bps << " + " << "Queue Size : " << Queue_Size_Packet << " Byte ] \n";

	monitor->CheckForLostPackets ();
	Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
	FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
	//-------------------------------------------------------------------------------------------
	for( std::map<FlowId , FlowMonitor::FlowStats>::const_iterator counter = stats.begin () ; counter != stats.end () ; ++counter)
	{
 		Ipv4FlowClassifier::FiveTuple ft = classifier->FindFlow( counter->first );

		if( ft.sourceAddress == Ipv4Address( i.GetAddress( 0 , 0 ) ) && ft.destinationAddress == Ipv4Address( i.GetAddress( 1 , 0 ) ) )
		{
			if( ft.destinationPort == CBR_Port )
			{
				// insert your statistics calculation here(ppt page26)
			}
		}
	}

	//-------------------------------------------------------------------------------------------
	//Avg_e2eDelaySec =	
	std::cout << "Average System End-to-End Delay : ";
	std::cout << Avg_e2eDelaySec << " [sec]\n";

    //Total_RxByte =
	std::cout << "Total Rx Byte : ";
	std::cout << Total_RxByte << "\n";
	
	//Avg_PDR =
	std::cout << "Average Packet Delivery Ratio : ";
	std::cout << Avg_PDR << "\n";

    Simulator::Destroy ();

	std::cout << "\nLab2 Simulation End\n";

    return 0;
}

