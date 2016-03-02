/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink.h"
#include "ns3/point-to-point-dumbbell.h"
#include "ns3/random-variable-stream.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/netanim-module.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/drop-tail-queue.h"
#include <iostream>
#include <string>
#include <fstream>
#include <math.h>

using namespace ns3;
using namespace std;
NS_LOG_COMPONENT_DEFINE ("ECE6110Project2");

int main( int argc, char* argv[])
{
    Time::SetResolution(Time::NS);
    LogComponentEnable("BulkSendApplication",LOG_LEVEL_INFO);
    //LogComponentEnable("BulkSendApplication",LOG_LEVEL_INFO);
    LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
    
    /*-------Set basic constant---------*/
    //uint32_t windowSize = 16000;
    //uint32_t queueSize = 32000;
    //uint32_t segSize = 512;
    //uint32_t packetSize = 1024;
    //uint16_t queueMode = 1; // 0 for DropTail 1 for RED
    //uint16_t port=9;
    
    //double thmin = 0.32;
    //double thmax = 0.64;
    
    /*--------batch process----------*/
    uint32_t windowSize[5] = {2000,8000,16000,32000,64000};
    uint32_t queueSize[5] = {2000,8000,16000,32000,64000};
    uint32_t segSize = 512;
    //uint32_t packetSize = 1024;
    uint16_t queueMode = 0; // 0 for DropTail 1 for RED
    uint16_t wait[2] = {0,1};
    uint16_t gentle[2] = {0,1};
    uint16_t port=9;
    
    //double thmin[10] = {0.01, 0.02, 0.04, 0.08, 0.08, 0.16, 0.16, 0.32, 0.32, 0.48};
    //double thmax[10] = {0.02, 0.04, 0.08, 0.16, 0.32, 0.32, 0.64, 0.64, 0.96, 0.96};
    double thmin[10] = {0.02, 0.04, 0.08, 0.16, 0.24, 0.32, 0.32, 0.4, 0.48, 0.6 };
    double thmax[10] = {0.04, 0.08, 0.16, 0.32, 0.48, 0.64, 0.8,  0.8, 0.96, 0.85};
                        // need to be over two times of thmin
                        // however, unsure about the unit bytes/packet?
    uint16_t count=0;
    
    /*-------Set command line-----------*/
    //CommandLine cmd;
    //cmd.AddValue("segSize", "Segment Size", segSize);
    //cmd.AddValue("queueSize", "Queue Size", queueSize);
    //cmd.AddValue("windowSize", "Window Size", windowSize);
    //cmd.AddValue("packetSize", "Packet Size", packetSize);
    //cmd.AddValue("queueMode","Queue Mode", queueMode);
    //cmd.Parse(argc,argv);
    
    
    /*-------file output ------*/
    fstream myfile;
    myfile.open ("/Users/luozhongyi/Desktop/Tools/ns-allinone-3.24.1/ns-3.24.1/scratch/DROPTAIL6.txt");
    //myfile << "wait,gentle,window,queuesize,thmin,thmax,TCP1,TCP2,UDP1,UDP2,TCPt,UDPt,total\n";
    myfile << "window,queuesize,TCP1,TCP2,UDP1,UDP2,TCPt,UDPt,total\n";
    
    for(uint16_t w=0; w<2; w++){
        for (uint16_t g=0; g<2;g++){
            for(uint16_t win=0; win<5; win++){
                for(uint16_t que=0; que<5; que++){
                    for(uint16_t thr=0; thr<10; thr++){
    
    
    /*--------Set Attribute for TCP and UDP--------*/
    //GlobalValue::Bind ("ChecksumEnabled", BooleanValue (false));
    //generall settings
    
    Config::SetDefault("ns3::TcpSocketBase::MaxWindowSize", UintegerValue(windowSize[win]));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(segSize));
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpTahoe"));
    Config::SetDefault("ns3::TcpSocketBase::WindowScaling", BooleanValue(false));
    
    //settings for droptail
    Config::SetDefault("ns3::DropTailQueue::Mode", EnumValue(DropTailQueue::QUEUE_MODE_BYTES));
    Config::SetDefault("ns3::DropTailQueue::MaxBytes", UintegerValue(queueSize[que]));
    
    //settings for RED
    //double maxpacket = thmax * packetSize;
    //double minpacket = thmin * packetSize;
    double maxpacket = floor( thmax[thr] * queueSize[que] );
    double minpacket = ceil( thmin[thr] * queueSize[que] );

    Config::SetDefault ("ns3::RedQueue::Mode", EnumValue (RedQueue::QUEUE_MODE_BYTES));
    Config::SetDefault ("ns3::RedQueue::MinTh", DoubleValue (minpacket));
    Config::SetDefault ("ns3::RedQueue::MaxTh", DoubleValue (maxpacket));
    Config::SetDefault ("ns3::RedQueue::QueueLimit", UintegerValue (queueSize[que])); // --> check.
    
    if( wait[w] == 1 )
        Config::SetDefault ("ns3::RedQueue::Wait", BooleanValue(true)); // --> check.
    else
        Config::SetDefault ("ns3::RedQueue::Wait", BooleanValue(false)); // --> check.
    
    if( gentle[g] ==1 )
        Config::SetDefault ("ns3::RedQueue::Gentle", BooleanValue(true)); // --> check.
    else
        Config::SetDefault ("ns3::RedQueue::Gentle", BooleanValue(false)); // --> check.
                        
                        
                        
    
                        /*--------Set Topology for Network------*/
                        NodeContainer node;
                        node.Create(13);
                        
                        //set position
                        Vector temp0(0, 0, 0);
                        Ptr<Node> n0 = node.Get(0);
                        Ptr<ConstantPositionMobilityModel> p0 = n0->GetObject<ConstantPositionMobilityModel>();
                        p0 = CreateObject<ConstantPositionMobilityModel>();
                        n0->AggregateObject(p0);
                        p0->SetPosition(temp0);
                        
                        Vector temp1(2, 0, 0);
                        Ptr<Node> n1 = node.Get(1);
                        Ptr<ConstantPositionMobilityModel> p1 = n1->GetObject<ConstantPositionMobilityModel>();
                        p1 = CreateObject<ConstantPositionMobilityModel>();
                        n1->AggregateObject(p1);
                        p1->SetPosition(temp1);
                        
                        Vector temp2(-2, 0, 0);
                        Ptr<Node> n2 = node.Get(2);
                        Ptr<ConstantPositionMobilityModel> p2 = n2->GetObject<ConstantPositionMobilityModel>();
                        p2 = CreateObject<ConstantPositionMobilityModel>();
                        n2->AggregateObject(p2);
                        p2->SetPosition(temp2);
                        
                        Vector temp3(-2, 2, 0);
                        Ptr<Node> n3 = node.Get(3);
                        Ptr<ConstantPositionMobilityModel> p3 = n3->GetObject<ConstantPositionMobilityModel>();
                        p3 = CreateObject<ConstantPositionMobilityModel>();
                        n3->AggregateObject(p3);
                        p3->SetPosition(temp3);
                        
                        Vector temp4(-2, -2, 0);
                        Ptr<Node> n4 = node.Get(4);
                        Ptr<ConstantPositionMobilityModel> p4 = n4->GetObject<ConstantPositionMobilityModel>();
                        p4 = CreateObject<ConstantPositionMobilityModel>();
                        n4->AggregateObject(p4);
                        p4->SetPosition(temp4);
                        
                        Vector temp5(-4, 2, 0);
                        Ptr<Node> n5 = node.Get(5);
                        Ptr<ConstantPositionMobilityModel> p5 = n5->GetObject<ConstantPositionMobilityModel>();
                        p5 = CreateObject<ConstantPositionMobilityModel>();
                        n5->AggregateObject(p5);
                        p5->SetPosition(temp5);
                        
                        Vector temp6(-4, -2, 0);
                        Ptr<Node> n6 = node.Get(6);
                        Ptr<ConstantPositionMobilityModel> p6 = n6->GetObject<ConstantPositionMobilityModel>();
                        p6 = CreateObject<ConstantPositionMobilityModel>();
                        n6->AggregateObject(p6);
                        p6->SetPosition(temp6);
                        
                        Vector temp7(3, 1, 0);
                        Ptr<Node> n7 = node.Get(7);
                        Ptr<ConstantPositionMobilityModel> p7 = n7->GetObject<ConstantPositionMobilityModel>();
                        p7 = CreateObject<ConstantPositionMobilityModel>();
                        n7->AggregateObject(p7);
                        p7->SetPosition(temp7);
                        
                        Vector temp8(3, -1, 0);
                        Ptr<Node> n8 = node.Get(8);
                        Ptr<ConstantPositionMobilityModel> p8 = n8->GetObject<ConstantPositionMobilityModel>();
                        p8 = CreateObject<ConstantPositionMobilityModel>();
                        n8->AggregateObject(p8);
                        p8->SetPosition(temp8);
                        
                        Vector temp9(3, 3, 0);
                        Ptr<Node> n9 = node.Get(9);
                        Ptr<ConstantPositionMobilityModel> p9 = n9->GetObject<ConstantPositionMobilityModel>();
                        p9 = CreateObject<ConstantPositionMobilityModel>();
                        n9->AggregateObject(p9);
                        p9->SetPosition(temp9);
                        
                        Vector temp10(5, 1, 0);
                        Ptr<Node> n10 = node.Get(10);
                        Ptr<ConstantPositionMobilityModel> p10 = n10->GetObject<ConstantPositionMobilityModel>();
                        p10 = CreateObject<ConstantPositionMobilityModel>();
                        n10->AggregateObject(p10);
                        p10->SetPosition(temp10);
                        
                        Vector temp11(5, -1, 0);
                        Ptr<Node> n11 = node.Get(11);
                        Ptr<ConstantPositionMobilityModel> p11 = n11->GetObject<ConstantPositionMobilityModel>();
                        p11 = CreateObject<ConstantPositionMobilityModel>();
                        n11->AggregateObject(p11);
                        p11->SetPosition(temp11);
                        
                        Vector temp12(3, -3, 0);
                        Ptr<Node> n12 = node.Get(12);
                        Ptr<ConstantPositionMobilityModel> p12 = n12->GetObject<ConstantPositionMobilityModel>();
                        p12 = CreateObject<ConstantPositionMobilityModel>();
                        n12->AggregateObject(p12);
                        p12->SetPosition(temp12);
                        //set interface
                        
                        NodeContainer link01 (node.Get(0), node.Get(1));
                        NodeContainer link17 (node.Get(1), node.Get(7));
                        NodeContainer link18 (node.Get(1), node.Get(8));
                        NodeContainer link79 (node.Get(7), node.Get(9));
                        NodeContainer link710 (node.Get(7), node.Get(10));
                        NodeContainer link811 (node.Get(8), node.Get(11));
                        NodeContainer link812 (node.Get(8), node.Get(12));
                        NodeContainer link02 (node.Get(0), node.Get(2));
                        NodeContainer link03 (node.Get(0), node.Get(3));
                        NodeContainer link04 (node.Get(0), node.Get(4));
                        NodeContainer link25 (node.Get(2), node.Get(5));
                        NodeContainer link26 (node.Get(2), node.Get(6));

                        
                        
                        
                        
    
    /*--------set channel parameter-----------*/
    
    // setting for router channel
    PointToPointHelper rc;
    rc.SetDeviceAttribute ("DataRate", StringValue ("0.5Mbps")); // --> Vary
    rc.SetChannelAttribute ("Delay", StringValue ("20ms")); // --> Vary
    if(queueMode==1){
        rc.SetQueue ("ns3::RedQueue");
        cout<<" RedQueue Mode Operating"<<endl;
    }
    else{
        rc.SetQueue ("ns3::DropTailQueue");
        cout<<"DropTailQueue Mode Operating"<<endl;
    }
    
    NetDeviceContainer dev01 = rc.Install(link01);
    NetDeviceContainer dev02 = rc.Install(link02);
    NetDeviceContainer dev17 = rc.Install(link17);
    NetDeviceContainer dev18 = rc.Install(link18);
    
    //setting for leaf channel
    PointToPointHelper lc;
    lc.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    lc.SetChannelAttribute ("Delay", StringValue ("2ms"));
    NetDeviceContainer dev03 = lc.Install(link03);
    NetDeviceContainer dev04 = lc.Install(link04);
    NetDeviceContainer dev25 = lc.Install(link25);
    NetDeviceContainer dev26 = lc.Install(link26);
    NetDeviceContainer dev79 = lc.Install(link79);
    NetDeviceContainer dev710 = lc.Install(link710);
    NetDeviceContainer dev811 = lc.Install(link811);
    NetDeviceContainer dev812 = lc.Install(link812);
    
    /*---------install stack-----------*/
    InternetStackHelper stack;
    stack.Install (node);
    
    /*---------assign IP address-------*/
    Ipv4AddressHelper add;
    
    string setbase1 = "192.160." + to_string(count) +".0";
    add.SetBase (Ipv4Address(setbase1.c_str()), "255.255.255.0");
    Ipv4InterfaceContainer int01 = add.Assign (dev01);
    
    string setbase2 = "192.162." + to_string(count) +".0";
    add.SetBase (Ipv4Address(setbase2.c_str()), "255.255.255.0");
    Ipv4InterfaceContainer int02 = add.Assign (dev02);
    
    string setbase3 = "192.164." + to_string(count) +".0";
    add.SetBase (Ipv4Address(setbase3.c_str()), "255.255.255.0");
    Ipv4InterfaceContainer int17 = add.Assign (dev17);
    
    string setbase4 = "192.166." + to_string(count) +".0";
    add.SetBase (Ipv4Address(setbase4.c_str()), "255.255.255.0");
    Ipv4InterfaceContainer int18 = add.Assign (dev18);
    
    string setbase5 = "192.168." + to_string(count) +".0";
    add.SetBase (Ipv4Address(setbase5.c_str()), "255.255.255.0");
    Ipv4InterfaceContainer int03 = add.Assign (dev03);

    string setbase6 = "192.170." + to_string(count) +".0";
    add.SetBase (Ipv4Address(setbase6.c_str()), "255.255.255.0");
    Ipv4InterfaceContainer int04 = add.Assign (dev04);

    string setbase7 = "192.172." + to_string(count) +".0";
    add.SetBase (Ipv4Address(setbase7.c_str()), "255.255.255.0");
    Ipv4InterfaceContainer int25 = add.Assign (dev25);

    string setbase8 = "192.174." + to_string(count) +".0";
    add.SetBase (Ipv4Address(setbase8.c_str()), "255.255.255.0");
    Ipv4InterfaceContainer int26 = add.Assign (dev26);

    string setbase9 = "192.176." + to_string(count) +".0";
    add.SetBase (Ipv4Address(setbase9.c_str()), "255.255.255.0");
    Ipv4InterfaceContainer int79 = add.Assign (dev79);

    string setbase10 = "192.178." + to_string(count) +".0";
    add.SetBase (Ipv4Address(setbase10.c_str()), "255.255.255.0");
    Ipv4InterfaceContainer int710 = add.Assign (dev710);

    string setbase11 = "192.180." + to_string(count) +".0";
    add.SetBase (Ipv4Address(setbase11.c_str()), "255.255.255.0");
    Ipv4InterfaceContainer int811 = add.Assign (dev811);

    string setbase12 = "192.182." + to_string(count) +".0";
    add.SetBase (Ipv4Address(setbase12.c_str()), "255.255.255.0");
    Ipv4InterfaceContainer int812 = add.Assign (dev812);

    count++;
                        
    /*-------------set TCP UDP application---------*/
    
    ApplicationContainer sendBox, sinkBox;
    
    //TCP application:
    BulkSendHelper Tsender1 ("ns3::TcpSocketFactory",InetSocketAddress (int812.GetAddress(1), port));
    Tsender1.SetAttribute ("MaxBytes", UintegerValue (1000000));
    Tsender1.SetAttribute ("SendSize", UintegerValue (512)); // Or use packet size instead?
    sendBox.Add(Tsender1.Install(node.Get(6)));
    
    BulkSendHelper Tsender2 ("ns3::TcpSocketFactory",InetSocketAddress (int811.GetAddress(1), port));
    Tsender2.SetAttribute ("MaxBytes", UintegerValue (1000000));
    Tsender2.SetAttribute ("SendSize", UintegerValue (512)); // Or use packet size instead?
    sendBox.Add(Tsender2.Install(node.Get(4)));
    
    //UDP application
    OnOffHelper Usender1("ns3::UdpSocketFactory", Address(InetSocketAddress(int79.GetAddress(1), port)));
    Usender1.SetConstantRate(DataRate("500kb/s")); // can be changed
    Usender1.SetAttribute ("PacketSize", UintegerValue (1024));
    Usender1.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=0.5]")); // 50% duty cycle
    Usender1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.5]"));
    sendBox.Add(Usender1.Install(node.Get(5)));
    
    OnOffHelper Usender2("ns3::UdpSocketFactory", Address(InetSocketAddress(int710.GetAddress(1), port)));
    Usender2.SetConstantRate(DataRate("500kb/s")); // can be changed
    Usender2.SetAttribute ("PacketSize", UintegerValue (1024));
    Usender2.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=0.5]")); // 50% duty cycle
    Usender2.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.5]"));
    sendBox.Add(Usender2.Install(node.Get(3)));
    
    
    //sink box
    PacketSinkHelper Tsink1("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    sinkBox.Add(Tsink1.Install(node.Get(12)));
    
    PacketSinkHelper Tsink2("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    sinkBox.Add(Tsink2.Install(node.Get(11)));
    
    PacketSinkHelper Usink1("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    sinkBox.Add(Usink1.Install(node.Get(9)));
    
    PacketSinkHelper Usink2("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    sinkBox.Add(Usink2.Install(node.Get(10)));
    
    for(uint16_t i=0;i<4;i++){
        (sendBox.Get(i)) -> SetStartTime(Seconds(0.0));
        (sendBox.Get(i)) -> SetStopTime(Seconds(10.0));
        (sinkBox.Get(i)) -> SetStartTime(Seconds(0.0));
        (sinkBox.Get(i)) -> SetStopTime(Seconds(10.0));
    }
    
    /*-------------simulation and output---------*/
    
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    Simulator::Stop(Seconds(10.0));
    Simulator::Run();
    
    cout <<"\nGoodput of each flow: " <<endl;
    Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkBox.Get(0));
    double recvTCP1 = sink1->GetTotalRx ();
    double opTCP1 = recvTCP1/10.0;
    cout << "Goodput of TCP flow 1: " << opTCP1 <<endl;
    
    Ptr<PacketSink> sink2 = DynamicCast<PacketSink> (sinkBox.Get(1));
    double recvTCP2 = sink2->GetTotalRx ();
    double opTCP2 = recvTCP2/10.0;
    cout << "Goodput of TCP flow 2: " << opTCP2 <<endl;
    
    Ptr<PacketSink> sink3 = DynamicCast<PacketSink> (sinkBox.Get(2));
    double recvUDP1 = sink3->GetTotalRx ();
    double opUDP1 = recvUDP1/10.0;
    cout << "Goodput of UDP flow 1: " << opUDP1 <<endl;
    
    Ptr<PacketSink> sink4 = DynamicCast<PacketSink> (sinkBox.Get(3));
    double recvUDP2 = sink4->GetTotalRx ();
    double opUDP2 = recvUDP2/10.0;
    cout << "Goodput of UDP flow 2: " << opUDP2 <<endl;
    
    double TCPsum = opTCP1 + opTCP2;
    cout << "Goodput of Total TCP: " << TCPsum <<endl;
    
    double UDPsum = opUDP1 + opUDP2;
    cout << "Goodput of Total UDP: " << UDPsum <<endl;
    
    double Totalsum = TCPsum + UDPsum;
    cout << "Goodput of Total: " << Totalsum <<endl;
    
    //myfile << wait[w]<< "," << gentle[g]<< ","<< windowSize[win]<< ","<<queueSize[que]<< ","<<thmin[thr]<<","<<thmax[thr]<<","
                        //<<opTCP1<<","<<opTCP2<<","<<opUDP1<< ","<<opUDP2<<","<< TCPsum<<","<<UDPsum<< ","<< Totalsum <<"\n";
      
    myfile << windowSize[win]<< ","<<queueSize[que]<< ","
                        <<opTCP1<<","<<opTCP2<<","<<opUDP1<< ","<<opUDP2<<","<< TCPsum<<","<<UDPsum<< ","<< Totalsum <<"\n";
                        
    Simulator::Destroy();
                        
                    }
                }
            }
        }
    }
    myfile.close();
    return 0;
}