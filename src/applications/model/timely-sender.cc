/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
* Copyright (c) 2007,2008,2009 INRIA, UDCAST
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
* Author: Amine Ismail <amine.ismail@sophia.inria.fr>
*                      <amine.ismail@udcast.com>
*/
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/random-variable.h"
#include "ns3/qbb-net-device.h"
#include "ns3/ipv4-end-point.h"
#include "timely-sender.h"
#include "ns3/seq-ts-header.h"
#include <stdlib.h>
#include <stdio.h>

namespace ns3 {

	NS_LOG_COMPONENT_DEFINE("TimelySender");
	NS_OBJECT_ENSURE_REGISTERED(TimelySender);

	TypeId
		TimelySender::GetTypeId(void)
	{
		static TypeId tid = TypeId("ns3::TimelySender")
			.SetParent<Application>()

			.AddConstructor<TimelySender>()

			.AddAttribute("MaxPackets",
			"The maximum number of packets the application will send",
			UintegerValue(100),
			MakeUintegerAccessor(&TimelySender::m_allowed),
			MakeUintegerChecker<uint32_t>())

			.AddAttribute("RemoteAddress",
			"The destination Address of the outbound packets",
			AddressValue(),
			MakeAddressAccessor(&TimelySender::m_peerAddress),
			MakeAddressChecker())

			.AddAttribute("RemotePort", "The destination port of the outbound packets",
			UintegerValue(100),
			MakeUintegerAccessor(&TimelySender::m_peerPort),
			MakeUintegerChecker<uint16_t>())

			.AddAttribute("PriorityGroup", "The priority group of this flow",
			UintegerValue(0),
			MakeUintegerAccessor(&TimelySender::m_pg),
			MakeUintegerChecker<uint16_t>())

			.AddAttribute("PacketSize",
			"Size of packets generated. The minimum packet size is 14 bytes which is the size of the header carrying the sequence number and the time stamp.",
			UintegerValue(1000),
			MakeUintegerAccessor(&TimelySender::m_pktSize),
			MakeUintegerChecker<uint32_t>(14, 1500))

			.AddAttribute("InitialRate",
			"Initial send rate in bits per second",
			DoubleValue(1.0 * 1000 * 1000 * 1000),
			MakeDoubleAccessor(&TimelySender::m_initRate),
			MakeDoubleChecker<double>())

			.AddAttribute("LinkSpeed",
			"Bottleneck link speed in bits per second",
			DoubleValue(10.0 * 1000 * 1000 * 1000),
			MakeDoubleAccessor(&TimelySender::m_C),
			MakeDoubleChecker<double>())

			.AddAttribute("Delta",
			"Additive increase in bits per second",
			UintegerValue(10 * 1000 * 1000),
			MakeUintegerAccessor(&TimelySender::m_delta),
			MakeUintegerChecker<uint32_t>())

			.AddAttribute("T_high",
			"T_high threshold in microseconds",
			DoubleValue(500.0 / (1000 * 1000)),
			MakeDoubleAccessor(&TimelySender::m_t_high),
			MakeDoubleChecker<double>())

			.AddAttribute("T_low",
			"T_low threshold in microseconds",
			DoubleValue(50.0 / (1000 * 1000)),
			MakeDoubleAccessor(&TimelySender::m_t_low),
			MakeDoubleChecker<double>())

			.AddAttribute("Min RTT",
			"MIN RTT in microseconds",
			DoubleValue(20.0 / (1000 * 1000)),
			MakeDoubleAccessor(&TimelySender::m_min_rtt),
			MakeDoubleChecker<double>())

			.AddAttribute("Beta",
			"Beta",
			DoubleValue(0.8),
			MakeDoubleAccessor(&TimelySender::m_beta),
			MakeDoubleChecker<double>())

			.AddAttribute("Alpha",
			"Alpha",
			DoubleValue(0.875),
			MakeDoubleAccessor(&TimelySender::m_alpha),
			MakeDoubleChecker<double>())

			.AddAttribute("BurstSize",
			"BurstSize",
			UintegerValue(16000),
			MakeUintegerAccessor(&TimelySender::m_burstSize),
			MakeUintegerChecker<uint32_t>())

			.AddAttribute("MinRateMultiple",
			"MinRateMultiple",
			DoubleValue(0.01),
			MakeDoubleAccessor(&TimelySender::m_minRateMultiple),
			MakeDoubleChecker<double>())

			.AddAttribute("MaxRateMultiple",
			"MaxRateMultiple",
			DoubleValue(0.96),
			MakeDoubleAccessor(&TimelySender::m_maxRateMultiple),
			MakeDoubleChecker<double>());
			
		return tid;
	}

	TimelySender::TimelySender()
	{
		NS_LOG_FUNCTION_NOARGS();	
	}

	void
		TimelySender::Init()
	{
		m_sent = 0;
		m_socket = 0;
		m_sendEvent = EventId();
		m_rate = m_initRate;
		m_burst_in_packets = (int)(m_burstSize / m_pktSize);
		m_sleep = GetBurstDuration(m_rate);
		m_rtt_diff = 0;
		m_received = 0;
		m_sdel = GetBurstDuration(1, m_C);
		m_prev_rtt = m_sdel;
		m_maxRate = m_maxRateMultiple * m_C;
		m_minRate = m_minRateMultiple * m_C;
		m_N = 1;
	}

	double
		TimelySender::GetBurstDuration(double rate)
	{
		return (double)m_burst_in_packets * m_pktSize * 8.0 / rate;
	}

	double 
		TimelySender::GetBurstDuration(int packets, double rate)
	{
		return (double)packets * m_pktSize * 8.0 / rate;
	}

	TimelySender::~TimelySender()
	{
		NS_LOG_FUNCTION_NOARGS();
	}

	void
		TimelySender::SetRemote(Ipv4Address ip, uint16_t port)
	{
		m_peerAddress = Address(ip);
		m_peerPort = port;
	}

	void
		TimelySender::SetRemote(Ipv6Address ip, uint16_t port)
	{
		m_peerAddress = Address(ip);
		m_peerPort = port;
	}

	void
		TimelySender::SetRemote(Address ip, uint16_t port)
	{
		m_peerAddress = ip;
		m_peerPort = port;
	}

	void
		TimelySender::DoDispose(void)
	{
		NS_LOG_FUNCTION_NOARGS();
		Application::DoDispose();
	}

	void
		TimelySender::StartApplication(void)
	{
		NS_LOG_FUNCTION_NOARGS();

		Init();

		if (m_socket == 0)
		{
			TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
			m_socket = Socket::CreateSocket(GetNode(), tid);
			if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
			{
				m_socket->Bind();
				m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
			}
			else if (Ipv6Address::IsMatchingType(m_peerAddress) == true)
			{
				m_socket->Bind6();
				m_socket->Connect(Inet6SocketAddress(Ipv6Address::ConvertFrom(m_peerAddress), m_peerPort));
			}
		}

		m_socket->SetRecvCallback(MakeCallback(&TimelySender::Receive, this));
		m_sendEvent = Simulator::Schedule(Seconds(0.0), &TimelySender::Send, this);
	}

	void
		TimelySender::StopApplication()
	{
		NS_LOG_FUNCTION_NOARGS();
		Simulator::Cancel(m_sendEvent);
	}

	void TimelySender::Send()
	{
		NS_ASSERT(m_sendEvent.IsExpired());
		if (m_sent < m_allowed)
		{
			
			/*SendPacket();
			m_sleep = GetBurstDuration(1, m_rate);
			Simulator::Schedule(Seconds(m_sleep), &TimelySender::Send, this);*/
			
			SendBurst();
			m_sendEvent = Simulator::Schedule(Seconds(m_sleep), &TimelySender::Send, this);
		}
	}

	
	void
		TimelySender::SendBurst(void)
	{
		NS_LOG_FUNCTION_NOARGS();
		for (int i = 0; i < m_burst_in_packets && m_sent < m_allowed; i++)
		{
			SendPacket();
		}
	}

	void 
		TimelySender::SendPacket()
	{
		SeqTsHeader seqTs;
		seqTs.SetSeq(m_sent);
		seqTs.SetPG(m_pg);
		double x = seqTs.GetTs().GetSeconds();
		if ((m_sent+1) % (int)m_burst_in_packets == 0)
			seqTs.SetAckNeeded();
		Ptr<Packet> p = Create<Packet>(m_pktSize);
		p->AddHeader(seqTs);
		if (m_socket->Send(p) >= 0)
		{
			m_sent++;
		}
		else
		{
			NS_LOG_INFO("Error while sending");
			exit(-1);
		}
	}

	void
		TimelySender::SetPG(uint16_t pg)
	{
		m_pg = pg;
		return;
	}

	void
		TimelySender::Receive(Ptr<Socket> socket)
	{
		Ptr<Packet> packet;
		Address from;
		while ((packet = socket->RecvFrom(from)))
		{
			m_received++;
			int x = packet->GetSize();
			SeqTsHeader seqTs;
			packet->RemoveHeader(seqTs);
			m_new_rtt = GenerateRTTSample(seqTs.GetTs());
			UpdateSendRate();
		}
	}

	double
		TimelySender::GenerateRTTSample(Time ts)
	{
		int nodeId = m_node->GetId();
		double timeNow = Simulator::Now().GetSeconds();
		double x = ts.GetSeconds();
		Time rtt_sample = Simulator::Now() - ts;
		double adjustedSample = rtt_sample.GetSeconds() - m_sdel;
		return adjustedSample;
	}

	void
		TimelySender::UpdateSendRate()
	{
		double m_new_rtt_diff = m_new_rtt - m_prev_rtt;
		double m_newRate = 0;
		int nodeId = m_node->GetId();
		double timeNow = Simulator::Now().GetSeconds();

		m_rtt_diff = (1 - m_alpha) * m_rtt_diff + m_alpha * m_new_rtt_diff;

		double normalized_gradiant =  m_rtt_diff / m_min_rtt;

		char *updaterule;
		if (m_new_rtt < m_t_low)
		{
			updaterule = "t_low";
			m_newRate = m_rate + m_delta;
		}
		else if (m_new_rtt > m_t_high)
		{
			updaterule = "t_high";
			m_newRate = m_rate * (1 - m_beta * (1 - m_t_high / m_new_rtt));
		}
		else if (normalized_gradiant < 0)
		{
			updaterule = "negative_grad";
			m_newRate = m_rate + m_N * m_delta;
		}
		else
		{
			updaterule = "pos_grad";
			m_newRate = m_rate * (1 - m_beta * normalized_gradiant);
		}

		m_newRate = std::min(m_newRate, m_maxRate);
		m_newRate = std::max(m_newRate, m_minRate);
		
		printf("%f Z%d %f %f\n", Simulator::Now().GetSeconds(), m_node->GetId(), m_rate/1000000000.0, m_newRate/1000000000.0);

		m_rate = m_newRate;
		m_prev_rtt = m_new_rtt;
		m_sleep = GetBurstDuration(m_rate);

		//printf("node=%d, time=%f, sent=%d, recvd=%dm sample=%f updaterule=%s newrate=%f\n", m_node->GetId(), x, m_sent, m_received, m_new_rtt, updaterule, m_rate/1000000000.0); 
	}

} // Namespace ns3