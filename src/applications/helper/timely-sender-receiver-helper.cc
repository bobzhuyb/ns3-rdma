/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
* Copyright (c) 2008 INRIA
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
* Author: Mohamed Amine Ismail <amine.ismail@sophia.inria.fr>
*/
#include "timely-sender-receiver-helper.h"
#include "ns3/timely-sender.h"
#include "ns3/timely-receiver.h"
#include "ns3/timely-trace-client.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"

namespace ns3 {

	TimelyReceiverHelper::TimelyReceiverHelper()
	{
	}

	TimelyReceiverHelper::TimelyReceiverHelper(uint16_t port)
	{
		m_factory.SetTypeId(TimelyReceiver::GetTypeId());
		SetAttribute("Port", UintegerValue(port));
	}

	TimelyReceiverHelper::TimelyReceiverHelper(uint16_t port, uint16_t pg)
	{
		m_factory.SetTypeId(TimelyReceiver::GetTypeId());
		SetAttribute("Port", UintegerValue(port));
		SetAttribute("PriorityGroup", UintegerValue(pg));
	}

	void
		TimelyReceiverHelper::SetAttribute(std::string name, const AttributeValue &value)
	{
		m_factory.Set(name, value);
	}

	ApplicationContainer
		TimelyReceiverHelper::Install(NodeContainer c)
	{
		ApplicationContainer apps;
		for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
		{
			Ptr<Node> node = *i;

			m_receiver = m_factory.Create<TimelyReceiver>();
			node->AddApplication(m_receiver);
			apps.Add(m_receiver);

		}
		return apps;
	}

	Ptr<TimelyReceiver>
		TimelyReceiverHelper::GetServer(void)
	{
		return m_receiver;
	}

	TimelySenderHelper::TimelySenderHelper()
	{
	}

	TimelySenderHelper::TimelySenderHelper(Address address, uint16_t port)
	{
		m_factory.SetTypeId(TimelySender::GetTypeId());
		SetAttribute("RemoteAddress", AddressValue(address));
		SetAttribute("RemotePort", UintegerValue(port));
	}

	TimelySenderHelper::TimelySenderHelper(Ipv4Address address, uint16_t port)
	{
		m_factory.SetTypeId(TimelySender::GetTypeId());
		SetAttribute("RemoteAddress", AddressValue(Address(address)));
		SetAttribute("RemotePort", UintegerValue(port));
	}

	TimelySenderHelper::TimelySenderHelper(Ipv4Address address, uint16_t port, uint16_t pg)
	{
		m_factory.SetTypeId(TimelySender::GetTypeId());
		SetAttribute("RemoteAddress", AddressValue(Address(address)));
		SetAttribute("RemotePort", UintegerValue(port));
		SetAttribute("PriorityGroup", UintegerValue(pg));
	}


	TimelySenderHelper::TimelySenderHelper(Ipv6Address address, uint16_t port)
	{
		m_factory.SetTypeId(TimelySender::GetTypeId());
		SetAttribute("RemoteAddress", AddressValue(Address(address)));
		SetAttribute("RemotePort", UintegerValue(port));
	}

	void
		TimelySenderHelper::SetAttribute(std::string name, const AttributeValue &value)
	{
		m_factory.Set(name, value);
	}

	ApplicationContainer
		TimelySenderHelper::Install(NodeContainer c)
	{
		ApplicationContainer apps;
		for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
		{
			Ptr<Node> node = *i;
			Ptr<TimelySender> client = m_factory.Create<TimelySender>();
			node->AddApplication(client);
			apps.Add(client);
		}
		return apps;
	}

	TimelyTraceClientHelper::TimelyTraceClientHelper()
	{
	}

	TimelyTraceClientHelper::TimelyTraceClientHelper(Address address, uint16_t port, std::string filename)
	{
		m_factory.SetTypeId(TimelyTraceClient::GetTypeId());
		SetAttribute("RemoteAddress", AddressValue(address));
		SetAttribute("RemotePort", UintegerValue(port));
		SetAttribute("TraceFilename", StringValue(filename));
	}

	TimelyTraceClientHelper::TimelyTraceClientHelper(Ipv4Address address, uint16_t port, std::string filename)
	{
		m_factory.SetTypeId(TimelyTraceClient::GetTypeId());
		SetAttribute("RemoteAddress", AddressValue(Address(address)));
		SetAttribute("RemotePort", UintegerValue(port));
		SetAttribute("TraceFilename", StringValue(filename));
	}

	TimelyTraceClientHelper::TimelyTraceClientHelper(Ipv6Address address, uint16_t port, std::string filename)
	{
		m_factory.SetTypeId(TimelyTraceClient::GetTypeId());
		SetAttribute("RemoteAddress", AddressValue(Address(address)));
		SetAttribute("RemotePort", UintegerValue(port));
		SetAttribute("TraceFilename", StringValue(filename));
	}

	void
		TimelyTraceClientHelper::SetAttribute(std::string name, const AttributeValue &value)
	{
		m_factory.Set(name, value);
	}

	ApplicationContainer
		TimelyTraceClientHelper::Install(NodeContainer c)
	{
		ApplicationContainer apps;
		for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
		{
			Ptr<Node> node = *i;
			Ptr<TimelyTraceClient> client = m_factory.Create<TimelyTraceClient>();
			node->AddApplication(client);
			apps.Add(client);
		}
		return apps;
	}

} // namespace ns3
