#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"
#include <fstream>
#include <math.h>

#define TestCase1_NumCommPair 4
#define TestCase2_NumCommPair 8
#define TestCase3_NumCommPair 12

using namespace ns3;


double LinearScale2dB( double LinearScale_Value )
{
  	double dB_Value = 10 * ( std::log10( LinearScale_Value ) );
  	return dB_Value;
}

double dB2LinearScale( double dB_Value )
{
  	double LinearScale_Value = std::pow(10.0,dB_Value / 10.0);
  	return LinearScale_Value;
}

double dBm2Watt( double dBm_Value )
{
  	double mW_Value = std::pow (10.0,dBm_Value / 10.0);
  	return mW_Value / 1000.0;
}

double Watt2dBm( double Watt_Value )
{
  	double dBm_Value = std::log10 (Watt_Value * 1000.0) * 10.0;
  	return dBm_Value;
}

double TwoRayGround_CS_RX_Threshold_Calculator( double TX_Power_dBm , double TX_Antenna_Gain , double RX_Antenna_Gain , double TX_Height_meter , double RX_Height_meter , double System_Loss , double Range_meter , double Lambda_meter , double MinDistance_meter )
{
	if( Range_meter <= MinDistance_meter )
	{
	    return TX_Power_dBm;
	}
	
	double Crossover_Dist = (4 * M_PI * TX_Height_meter * RX_Height_meter) / Lambda_meter ;
	double tmp = 0;
	if( Range_meter <= Crossover_Dist )
	{
		double numerator = Lambda_meter * Lambda_meter;
		tmp = M_PI * Range_meter;
		double denominator = 16 * tmp * tmp * System_Loss;
		double pr = 10 * std::log10 (numerator / denominator);

		return TX_Power_dBm + pr;
	}
	else
	{
		tmp = TX_Height_meter * RX_Height_meter;
		double rayNumerator = tmp * tmp;
		tmp = Range_meter * Range_meter;
		double rayDenominator = tmp * tmp * System_Loss;
		double rayPr = 10 * std::log10 (rayNumerator / rayDenominator);

		return TX_Power_dBm + rayPr;
	}
}

int main( int argc , char *argv[] )
{
	int Topology_Size = 3, // { 3 , 4 , 5 } 
	    CW_Size_Min = 2, // { 2 , 7 , 15 , 31 , 63 , 127 }
	    CW_Size_Max = 2; // { 2 , 7 , 15 , 31 , 63 , 127 } 


	std::cout << "Lab4 Simulation Start\n\n";
	std::cout << "Topology = " << Topology_Size << "x" << Topology_Size << "\n";
	
	std::cout << "MinCW = " << CW_Size_Min << " , " << "MaxCW = " << CW_Size_Max << "\n";

	
	// Create node
	int Num_WiFiSTA_Node = Topology_Size * Topology_Size;
	NodeContainer WiFiSTA_Node;
	WiFiSTA_Node.Create( Num_WiFiSTA_Node );


	// set position
	int Start_XPos_meter , Start_YPos_meter , Start_ZPos_meter;
	int Node_Spacing_meter = 40;
	MobilityHelper WiFiSTA_Mobility;

	Start_XPos_meter = Start_YPos_meter = Start_ZPos_meter = 0;
	//your code，建立topology，node的分布參考ppt p.15、p.16、p.17
	//建立方式可以使用GridPositionAllocator
	//使用方式為mobility.SetPositionAllocator ("ns3::GridPositionAllocator","MinX",DoubleValue(???),"MinY",DoubleValue(???),...,UintegerValue(???),...,StringValue(???));
	//總共有6種不同的parameter需要給值，分別為MinX,MinY,DeltaX,DeltaY,GridWidth,LayoutType
	//MinX, MinY為起始位置、DeltaX，DeltaY為節點之間的距離、GridWidth為每行節點數目、LayoutType為佈局方式(Ex:RowFirst、ColumnFirst)
	WiFiSTA_Mobility.SetMobilityModel( "ns3::ConstantPositionMobilityModel" );
	WiFiSTA_Mobility.Install( WiFiSTA_Node );


	// Set wifi NIC
	WifiHelper WiFi = WifiHelper::Default();

	std::string WiFi_Standard_Protocol = "802.11a";
	if( WiFi_Standard_Protocol == "802.11a" )
		WiFi.SetStandard( WIFI_PHY_STANDARD_80211a );
	else if( WiFi_Standard_Protocol == "802.11b" )
		WiFi.SetStandard( WIFI_PHY_STANDARD_80211b);
	else if( WiFi_Standard_Protocol == "802.11g" )
		WiFi.SetStandard( WIFI_PHY_STANDARD_80211g);
	else if( WiFi_Standard_Protocol == "802.11n-2.4GHz" )
		WiFi.SetStandard( WIFI_PHY_STANDARD_80211n_2_4GHZ );
	else if( WiFi_Standard_Protocol == "802.11n-5GHz" )
		WiFi.SetStandard( WIFI_PHY_STANDARD_80211n_5GHZ );
	else if( WiFi_Standard_Protocol == "802.11ac" )
		WiFi.SetStandard( WIFI_PHY_STANDARD_80211ac );
	WiFi.SetRemoteStationManager( "ns3::IdealWifiManager" );

	// Set RTS_CTS_Threshold
	std::string RtsCts_Activate_LowerBound = "100000";
	Config::SetDefault( "ns3::WifiRemoteStationManager::RtsCtsThreshold" , StringValue( RtsCts_Activate_LowerBound ) );

	// Set wifi MAC
	YansWifiChannelHelper WiFiChannel =  YansWifiChannelHelper::Default();
	YansWifiPhyHelper WiFiPhy =  YansWifiPhyHelper::Default();
	WiFiPhy.SetChannel( WiFiChannel.Create() );

	NqosWifiMacHelper WiFiMAC = NqosWifiMacHelper::Default();
	WiFiMAC.SetType( "ns3::AdhocWifiMac" );
    	NetDeviceContainer WifiSTA_PhyMac_Wrapper = WiFi.Install( WiFiPhy , WiFiMAC , WiFiSTA_Node );
	 
	PointerValue ptr;
	for( NetDeviceContainer::Iterator NDCI = WifiSTA_PhyMac_Wrapper.Begin() ; NDCI != WifiSTA_PhyMac_Wrapper.End() ; ++NDCI )
	{
		(*NDCI)->GetAttribute ("Mac", ptr);
		Ptr<RegularWifiMac> RegularWiFiMac = ptr.Get<RegularWifiMac> ();
		RegularWiFiMac->GetAttribute( "DcaTxop" , ptr );
		Ptr<DcaTxop> DCA_CWSize_Adjuster = ptr.Get<DcaTxop> ();
		DCA_CWSize_Adjuster->SetMinCw( CW_Size_Min );
		DCA_CWSize_Adjuster->SetMaxCw( CW_Size_Max );
	}


	// Set TCP/IP
	InternetStackHelper Internet_Installer;
    NodeContainer WiFi_Node( WiFiSTA_Node );
	Internet_Installer.Install( WiFi_Node );

	Ipv4AddressHelper IPv4Addr_Assigner;
	IPv4Addr_Assigner.SetBase( "140.113.0.0" , "255.255.0.0" );
    NetDeviceContainer Wifi_PhyMac_Wrapper( WifiSTA_PhyMac_Wrapper );
	Ipv4InterfaceContainer WiFiLink_InterfaceIP = IPv4Addr_Assigner.Assign( Wifi_PhyMac_Wrapper );

	Ipv4GlobalRoutingHelper::PopulateRoutingTables();


	// generate CBR
	int CBR_PacketSize_Byte = 1024;
	std::string CBR_Rate_bps = "500kbps";
	double Traffic_StartTime_sec = 1.0 , Traffic_EndTime_sec = 3.0 , TotalSimulationTime_sec = 4.0;
	int CBR_Port = 812;
	ApplicationContainer CBR_Tx_App , CBR_Rx_App; 

		// Topology = 3x3
		//your code，參考ppt p.15、p.16、p.17，看一下他們的node是怎麼傳的(Ex:node 0 to node 1)
		//Tx是傳送端的
		//Rx是接收端的
		int Tx_Index[ TestCase1_NumCommPair ] = {???}, Rx_Index[ TestCase1_NumCommPair ] = {???};

		// Transmitter
		for( int counter = 0 ; counter < TestCase1_NumCommPair ; ++counter )
		{
			//your code,與lab2的code一樣，但要注意一下參數(Ex:Ipv4InterfaceContainer、Rx_Index[]、CBR_Port...等)
		}
		// Receiver
		for( int counter = 0 ; counter < TestCase1_NumCommPair ; ++counter )
		{
			//your code,與lab2的code一樣，但要注意一下參數(Ex:Ipv4InterfaceContainer、Rx_Index[]、CBR_Port...等)
		}
	
	


	// Create FlowMonitor
	FlowMonitorHelper flowmon;
	Ptr<FlowMonitor> monitor = flowmon.InstallAll();

	// Start Simulation
  	AnimationInterface anim ("Lab3_2.xml");

	Simulator::Stop( Seconds( TotalSimulationTime_sec ) );
	Simulator::Run();

	// Show Statistics Information
	double Avg_SystemThroughput_bps = 0.0;
	int Total_NumLostPacket = 0;
	int Avg_Base = 0;

	monitor->CheckForLostPackets ();
	Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
	FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
	for( std::map<FlowId , FlowMonitor::FlowStats>::const_iterator counter = stats.begin () ; counter != stats.end () ; ++counter )
	{
  		Ipv4FlowClassifier::FiveTuple ft = classifier->FindFlow( counter->first );
		if( ft.destinationPort == CBR_Port )
		{
				Avg_SystemThroughput_bps +=
					( 
						//your code
					);

				Total_NumLostPacket += 
					(
						//your code 
					);
		
				++Avg_Base;
		
		}
	}

	
			Avg_SystemThroughput_bps = Avg_SystemThroughput_bps / Avg_Base;
			std::cout << "Avg System Throughput : ";
			std::cout << Avg_SystemThroughput_bps
				  << " [bps]\n";

			std::cout << "Total Lost Packets : ";
			std::cout << Total_NumLostPacket
				  << "\n";

		
	
	Simulator::Destroy ();

	std::cout << "\nLab4 Simulation End\n";


	return 0;
}
