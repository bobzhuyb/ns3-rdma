/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
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
 */

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/seq-ts-header.h"

#include "timely-receiver.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TimelyReceiverApplication");
NS_OBJECT_ENSURE_REGISTERED (TimelyReceiver);

TypeId
TimelyReceiver::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TimelyReceiver")
    .SetParent<Application> ()
    .AddConstructor<TimelyReceiver> ()
    .AddAttribute ("Port", "Port on which we listen for incoming packets.",
                   UintegerValue (9),
                   MakeUintegerAccessor (&TimelyReceiver::m_port),
                   MakeUintegerChecker<uint16_t> ())
	.AddAttribute ("PriorityGroup", "The priority group of this flow",
				   UintegerValue (0),
				   MakeUintegerAccessor (&TimelyReceiver::m_pg),
				   MakeUintegerChecker<uint16_t> ())
	.AddAttribute ("ChunkSize", 
				   "The chunk size can be sent before getting an ack",
				   UintegerValue (1000),
				   MakeUintegerAccessor (&TimelyReceiver::m_chunk),
				   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

TimelyReceiver::TimelyReceiver ()
{
	m_received = 0;
	m_sent = 0;
	count = 0;
	NS_LOG_FUNCTION_NOARGS ();
}

TimelyReceiver::~TimelyReceiver()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_socket = 0;
  m_socket6 = 0;
}

void
TimelyReceiver::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Application::DoDispose ();
}

void 
TimelyReceiver::StartApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
      m_socket->Bind (local);
      if (addressUtils::IsMulticast (m_local))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, m_local);
            }
          else
            {
              NS_FATAL_ERROR ("Error: Failed to join multicast group");
            }
        }
    }

  if (m_socket6 == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket6 = Socket::CreateSocket (GetNode (), tid);
      Inet6SocketAddress local6 = Inet6SocketAddress (Ipv6Address::GetAny (), m_port);
      m_socket6->Bind (local6);
      if (addressUtils::IsMulticast (local6))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket6);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, local6);
            }
          else
            {
              NS_FATAL_ERROR ("Error: Failed to join multicast group");
            }
        }
    }

  m_socket->SetRecvCallback (MakeCallback (&TimelyReceiver::HandleRead, this));
  m_socket6->SetRecvCallback (MakeCallback (&TimelyReceiver::HandleRead, this));
}

void 
TimelyReceiver::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();

  if (m_socket != 0) 
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
  if (m_socket6 != 0) 
    {
      m_socket6->Close ();
      m_socket6->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void 
TimelyReceiver::HandleRead(Ptr<Socket> socket)
{
	Ptr<Packet> packet;
	Address from;
	while ((packet = socket->RecvFrom(from)))
	{
		m_received++;
		SeqTsHeader receivedSeqTs;
		packet->RemoveHeader(receivedSeqTs);
		if (receivedSeqTs.GetAckNeeded() == 1)
		{
			double timeNow = Simulator::Now().GetSeconds();
			double rcvdTs = receivedSeqTs.GetTs().GetSeconds();
			double oneWayDelay = timeNow - rcvdTs;
			int sz = packet->GetSize();

			count++;
			SeqTsHeader seqTs;
			seqTs.SetSeq(m_sent);
			seqTs.SetPG(m_pg);
			seqTs.SetTsAsUint64(receivedSeqTs.GetTsAsUint64());
			Ptr<Packet> p = Create<Packet>(0);
			p->AddHeader(seqTs);
			NS_LOG_LOGIC("Echoing packet");
			socket->SendTo(p, 0, from);
			m_sent++;

		}
	}
}

} // Namespace ns3
