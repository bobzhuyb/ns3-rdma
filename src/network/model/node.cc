/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006 Georgia Tech Research Corporation, INRIA
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
 * Authors: George F. Riley<riley@ece.gatech.edu>
 *          Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include <iostream>
#include "node.h"
#include "node-list.h"
#include "net-device.h"
#include "application.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/object-vector.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/global-value.h"
#include "ns3/boolean.h"
#include "ns3/simulator.h"

NS_LOG_COMPONENT_DEFINE ("Node");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (Node);

GlobalValue g_checksumEnabled  = GlobalValue ("ChecksumEnabled",
                                              "A global switch to enable all checksums for all protocols",
                                              BooleanValue (false),
                                              MakeBooleanChecker ());

TypeId 
Node::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Node")
    .SetParent<Object> ()
    .AddConstructor<Node> ()
    .AddAttribute ("DeviceList", "The list of devices associated to this Node.",
                   ObjectVectorValue (),
                   MakeObjectVectorAccessor (&Node::m_devices),
                   MakeObjectVectorChecker<NetDevice> ())
    .AddAttribute ("ApplicationList", "The list of applications associated to this Node.",
                   ObjectVectorValue (),
                   MakeObjectVectorAccessor (&Node::m_applications),
                   MakeObjectVectorChecker<Application> ())
    .AddAttribute ("Id", "The id (unique integer) of this Node.",
                   TypeId::ATTR_GET, // allow only getting it.
                   UintegerValue (0),
                   MakeUintegerAccessor (&Node::m_id),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

Node::Node()
  : m_id (0),
    m_sid (0)
{

	/*
	m_maxBufferBytes=100*1500;
	m_usedTotalBytes=0;

	for (int i=0;i<32;i++)
	{
		m_usedIngressPortMinBytes[i]=0;
		m_usedIngressPortSharedBytes[i]=0;
		m_usedIngressPortMinExceed[i]=0;
		m_usedIngressPortSharedExceed[i]=0;
		m_usedEgressPortBytes[i]=0;
		for (int j=0;j<8;j++)
		{
			m_usedIngressPGMinBytes[i][j]=0;
			m_usedIngressPGSharedBytes[i][j]=0;
			m_usedIngressPGMinExceed[i][j]=0;
			m_usedIngressPGSharedExceed[i][j]=0;
			m_usedEgressQMinBytes[i][j]=0;
			m_usedEgressQSharedBytes[i][j]=0;
		}
	}
	for (int i=0;i<4;i++)
	{
		m_usedIngressSPBytes[i]=0;
		m_usedIngressSPExceed[i]=0;
		m_usedEgressSPBytes[i]=0;
	}

	m_usedIngressSPSharedBytes=0;
	m_usedIngressSPSharedExceed=0;

	m_usedIngressHeadroomBytes=0;


	//ingress params
	m_buffer_cell_limit_sp=30*1500; //ingress sp buffer threshold p.120
	m_buffer_cell_limit_sp_shared=30*1500; //ingress sp buffer shared threshold p.120, nonshare -> share
	m_pg_min_cell=1500; //ingress pg guarantee p.121					---1
	m_port_min_cell=1500; //ingress port guarantee						---2
	m_pg_shared_limit_cell=8*1500; //max buffer for an ingress pg			---3	PAUSE
	m_port_max_shared_cell=20*1500; //max buffer for an ingress port		---4	PAUSE
	m_pg_hdrm_limit=30*1500; //ingress pg headroom
	m_port_max_pkt_size=5*1500; //ingress global headroom
	//still needs reset limits..

	//egress params
	m_op_buffer_shared_limit_cell=100*1500; //per egress sp limit
	m_op_uc_port_config_cell=100*1500; //per egress port limit
	m_q_min_cell=100*1500;
	m_op_uc_port_config1_cell=100*1500;
	
	*/
	Construct ();
}

Node::Node(uint32_t sid)
  : m_id (0),
    m_sid (sid)
{
	/*
	m_maxBufferBytes=100*1500;
	m_usedTotalBytes=0;

	for (int i=0;i<32;i++)
	{
		m_usedIngressPortMinBytes[i]=0;
		m_usedIngressPortSharedBytes[i]=0;
		m_usedIngressPortMinExceed[i]=0;
		m_usedIngressPortSharedExceed[i]=0;
		m_usedEgressPortBytes[i]=0;
		for (int j=0;j<8;j++)
		{
			m_usedIngressPGMinBytes[i][j]=0;
			m_usedIngressPGSharedBytes[i][j]=0;
			m_usedIngressPGMinExceed[i][j]=0;
			m_usedIngressPGSharedExceed[i][j]=0;
			m_usedEgressQMinBytes[i][j]=0;
			m_usedEgressQSharedBytes[i][j]=0;
		}
	}
	for (int i=0;i<4;i++)
	{
		m_usedIngressSPBytes[i]=0;
		m_usedIngressSPExceed[i]=0;
		m_usedEgressSPBytes[i]=0;
	}

	m_usedIngressSPSharedBytes=0;
	m_usedIngressSPSharedExceed=0;

	m_usedIngressHeadroomBytes=0;

	//ingress params
	m_buffer_cell_limit_sp=30*1500; //ingress sp buffer threshold p.120
	m_buffer_cell_limit_sp_shared=30*1500; //ingress sp buffer shared threshold p.120, nonshare -> share
	m_pg_min_cell=1500; //ingress pg guarantee p.121					---1
	m_port_min_cell=1500; //ingress port guarantee						---2
	m_pg_shared_limit_cell=8*1500; //max buffer for an ingress pg			---3	PAUSE
	m_port_max_shared_cell=20*1500; //max buffer for an ingress port		---4	PAUSE
	m_pg_hdrm_limit=30*1500; //ingress pg headroom
	m_port_max_pkt_size=5*1500; //ingress global headroom
	//still needs reset limits..

	//egress params
	m_op_buffer_shared_limit_cell=100*1500; //per egress sp limit
	m_op_uc_port_config_cell=100*1500; //per egress port limit
	m_op_uc_port_config1_cell=100*1500;
	m_q_min_cell=100*1500;
	*/

	Construct ();
}

void
Node::Construct (void)
{
	m_node_type = 0;
  m_id = NodeList::Add (this);
}

Node::~Node ()
{
}

uint32_t
Node::GetId (void) const
{
  return m_id;
}

uint32_t
Node::GetSystemId (void) const
{
  return m_sid;
}

uint32_t
Node::AddDevice (Ptr<NetDevice> device)
{
  uint32_t index = m_devices.size ();
  m_devices.push_back (device);
  device->SetNode (this);
  device->SetIfIndex (index);
  device->SetReceiveCallback (MakeCallback (&Node::NonPromiscReceiveFromDevice, this));
  Simulator::ScheduleWithContext (GetId (), Seconds (0.0), 
                                  &NetDevice::Start, device);
  NotifyDeviceAdded (device);
  return index;
}
Ptr<NetDevice>
Node::GetDevice (uint32_t index) const
{
  NS_ASSERT_MSG (index < m_devices.size (), "Device index " << index <<
                 " is out of range (only have " << m_devices.size () << " devices).");
  return m_devices[index];
}
uint32_t 
Node::GetNDevices (void) const
{
  return m_devices.size ();
}

uint32_t 
Node::AddApplication (Ptr<Application> application)
{
  uint32_t index = m_applications.size ();
  m_applications.push_back (application);
  application->SetNode (this);
  Simulator::ScheduleWithContext (GetId (), Seconds (0.0), 
                                  &Application::Start, application);
  return index;
}
Ptr<Application> 
Node::GetApplication (uint32_t index) const
{
  NS_ASSERT_MSG (index < m_applications.size (), "Application index " << index <<
                 " is out of range (only have " << m_applications.size () << " applications).");
  return m_applications[index];
}
uint32_t 
Node::GetNApplications (void) const
{
  return m_applications.size ();
}

void 
Node::DoDispose ()
{
  m_deviceAdditionListeners.clear ();
  m_handlers.clear ();
  for (std::vector<Ptr<NetDevice> >::iterator i = m_devices.begin ();
       i != m_devices.end (); i++)
    {
      Ptr<NetDevice> device = *i;
      device->Dispose ();
      *i = 0;
    }
  m_devices.clear ();
  for (std::vector<Ptr<Application> >::iterator i = m_applications.begin ();
       i != m_applications.end (); i++)
    {
      Ptr<Application> application = *i;
      application->Dispose ();
      *i = 0;
    }
  m_applications.clear ();
  Object::DoDispose ();
}
void 
Node::DoStart (void)
{
  for (std::vector<Ptr<NetDevice> >::iterator i = m_devices.begin ();
       i != m_devices.end (); i++)
    {
      Ptr<NetDevice> device = *i;
      device->Start();
    }
  for (std::vector<Ptr<Application> >::iterator i = m_applications.begin ();
       i != m_applications.end (); i++)
    {
      Ptr<Application> application = *i;
      application->Start();
    }

  Object::DoStart();
}

void
Node::RegisterProtocolHandler (ProtocolHandler handler, 
                               uint16_t protocolType,
                               Ptr<NetDevice> device,
                               bool promiscuous)
{
  struct Node::ProtocolHandlerEntry entry;
  entry.handler = handler;
  entry.protocol = protocolType;
  entry.device = device;
  entry.promiscuous = promiscuous;

  // On demand enable promiscuous mode in netdevices
  if (promiscuous)
    {
      if (device == 0)
        {
          for (std::vector<Ptr<NetDevice> >::iterator i = m_devices.begin ();
               i != m_devices.end (); i++)
            {
              Ptr<NetDevice> dev = *i;
              dev->SetPromiscReceiveCallback (MakeCallback (&Node::PromiscReceiveFromDevice, this));
            }
        }
      else
        {
          device->SetPromiscReceiveCallback (MakeCallback (&Node::PromiscReceiveFromDevice, this));
        }
    }

  m_handlers.push_back (entry);
}

void
Node::UnregisterProtocolHandler (ProtocolHandler handler)
{
  for (ProtocolHandlerList::iterator i = m_handlers.begin ();
       i != m_handlers.end (); i++)
    {
      if (i->handler.IsEqual (handler))
        {
          m_handlers.erase (i);
          break;
        }
    }
}

bool
Node::ChecksumEnabled (void)
{
  BooleanValue val;
  g_checksumEnabled.GetValue (val);
  return val.Get ();
}

bool
Node::PromiscReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol,
                                const Address &from, const Address &to, NetDevice::PacketType packetType)
{
  NS_LOG_FUNCTION (this);
  return ReceiveFromDevice (device, packet, protocol, from, to, packetType, true);
}

bool
Node::NonPromiscReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol,
                                   const Address &from)
{
  NS_LOG_FUNCTION (this);
  return ReceiveFromDevice (device, packet, protocol, from, device->GetAddress (), NetDevice::PacketType (0), false);
}

bool
Node::ReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol,
                         const Address &from, const Address &to, NetDevice::PacketType packetType, bool promiscuous)
{
  NS_ASSERT_MSG (Simulator::GetContext () == GetId (), "Received packet with erroneous context ; " <<
                 "make sure the channels in use are correctly updating events context " <<
                 "when transfering events from one node to another.");
  NS_LOG_DEBUG ("Node " << GetId () << " ReceiveFromDevice:  dev "
                        << device->GetIfIndex () << " (type=" << device->GetInstanceTypeId ().GetName ()
                        << ") Packet UID " << packet->GetUid ());
  bool found = false;

  for (ProtocolHandlerList::iterator i = m_handlers.begin ();
       i != m_handlers.end (); i++)
    {
      if (i->device == 0 ||
          (i->device != 0 && i->device == device))
        {
          if (i->protocol == 0 || 
              i->protocol == protocol)
            {
              if (promiscuous == i->promiscuous)
                {
                  i->handler (device, packet, protocol, from, to, packetType);
                  found = true;
                }
            }
        }
    }
  return found;
}
void 
Node::RegisterDeviceAdditionListener (DeviceAdditionListener listener)
{
  m_deviceAdditionListeners.push_back (listener);
  // and, then, notify the new listener about all existing devices.
  for (std::vector<Ptr<NetDevice> >::const_iterator i = m_devices.begin ();
       i != m_devices.end (); ++i)
    {
      listener (*i);
    }
}
void 
Node::UnregisterDeviceAdditionListener (DeviceAdditionListener listener)
{
  for (DeviceAdditionListenerList::iterator i = m_deviceAdditionListeners.begin ();
       i != m_deviceAdditionListeners.end (); i++)
    {
      if ((*i).IsEqual (listener))
        {
          m_deviceAdditionListeners.erase (i);
          break;
         }
    }
}
 
void 
Node::NotifyDeviceAdded (Ptr<NetDevice> device)
{
  for (DeviceAdditionListenerList::iterator i = m_deviceAdditionListeners.begin ();
       i != m_deviceAdditionListeners.end (); i++)
    {
      (*i) (device);
    }  
}
 

//yibo
void 
Node::SetNodeType(uint32_t type, bool dynamicth)
{
	m_node_type = type;
	if (type==1)
	{
		m_broadcom = CreateObject<BroadcomNode>();
		if (dynamicth)
		{
			m_broadcom->SetDynamicThreshold();
		}
	}
}

void 
Node::SetNodeType(uint32_t type)
{
	m_node_type = type;
	if (type==1)
	{
		m_broadcom = CreateObject<BroadcomNode>();
	}
}

uint32_t 
Node::GetNodeType()
{
	return m_node_type;
}


/*
void
Node::SetBroadcomParams(uint32_t maxBufferBytes, 
						uint32_t maxIngressPortBytes,
						uint32_t maxIngressSPBytes,
						uint32_t maxIngressPGBytes,
						uint32_t maxEgressPortBytes,
						uint32_t maxEgressSPBytes,
						uint32_t maxEgressPGBytes,
						uint32_t buffer_cell_limit_sp, //ingress sp buffer threshold p.120
						uint32_t buffer_cell_limit_sp_shared, //ingress sp buffer shared threshold p.120, nonshare -> share
						uint32_t pg_min_cell, //ingress pg guarantee p.121					---1
						uint32_t port_min_cell, //ingress port guarantee						---2
						uint32_t pg_shared_limit_cell, //max buffer for an ingress pg			---3	PAUSE
						uint32_t port_max_shared_cell, //max buffer for an ingress port		---4	PAUSE
						uint32_t pg_hdrm_limit, //ingress pg headroom
						uint32_t port_max_pkt_size, //ingress global headroom
						uint32_t op_buffer_shared_limit_cell, //per egress sp limit
						uint32_t uc_port_config_cell //per egress port limit
						)
{
	m_maxBufferBytes = maxBufferBytes;
	return;
}

bool 
Node::CheckIngressAdmission(uint32_t port,uint32_t qIndex,uint32_t psize)
{
	if (m_usedTotalBytes+psize>m_maxBufferBytes)  //buffer full, usually should not reach here.
	{
		std::cout<<"WARNING: Drop because ingress buffer full\n";
		return false;
	}
	if (m_usedIngressPGMinBytes[port][qIndex]+psize>m_pg_min_cell && m_usedIngressPortMinBytes[qIndex]+psize>m_port_min_cell) // exceed guaranteed, use share buffer
	{
		//after pg/port share limit reached, do packets go to service pool buffer or headroom?
		if (m_usedIngressPGSharedBytes[port][qIndex]+psize>m_pg_shared_limit_cell					//exceed pg share limit, use headroom
			|| m_usedIngressPortSharedBytes[port]+psize>m_port_max_shared_cell				//exceed port share limit, use headroom
			|| (m_usedIngressSPBytes[GetIngressSP(port,qIndex)] + psize > m_buffer_cell_limit_sp	// exceed SP buffer limit and...
			&& m_usedIngressSPSharedBytes + psize > m_buffer_cell_limit_sp_shared)					// exceed shared sp buffer, use headroom
		   )
		{
			if (m_usedIngressHeadroomBytes + psize > m_pg_hdrm_limit) // exceed headroom space
			{
				std::cout<<"WARNING: Drop because ingress headroom full\n";
				return false;
			}
		}
	}
	return true;
}




bool 
Node::CheckEgressAdmission(uint32_t port,uint32_t qIndex,uint32_t psize)
{
	if (m_usedEgressSPBytes[GetEgressSP(port,qIndex)]+psize>m_op_buffer_shared_limit_cell)  //exceed the sp limit
		return false;

	if (m_usedEgressPortBytes[port]+psize>m_op_uc_port_config_cell)	//exceed the port limit
		return false;

	if (m_usedEgressQSharedBytes[port][qIndex]+psize>m_op_uc_port_config1_cell) //exceed the queue limit
		return false;

	return true;
}


void
Node::UpdateIngressAdmission(uint32_t port,uint32_t qIndex,uint32_t psize)
{
	m_usedTotalBytes += psize; //count total buffer usage
	if (m_usedIngressPGMinBytes[port][qIndex]+psize<m_pg_min_cell) //use guaranteed pg buffer
	{
		m_usedIngressPGMinBytes[port][qIndex]+=psize;
		return;
	}
	else
	{
		m_usedIngressPGMinExceed[port][qIndex]++;
	}
	
	if (m_usedIngressPortMinBytes[port]+psize<m_port_min_cell) //use guaranteed port buffer
	{
		// if the packet is using port_min, is it also counted for pg?
		m_usedIngressPortMinBytes[port]+=psize;
		return;
	}
	else
	{
		m_usedIngressPortMinExceed[port]++;
	}

	//begin to use shared buffer
	if (m_usedIngressPGSharedBytes[port][qIndex]+psize<m_pg_shared_limit_cell)								//doesn't exceed pg share limit
	{
		if (m_usedIngressPortSharedBytes[port]+psize<m_port_max_shared_cell)								//doesn't exceed port share limit
		{
			if (m_usedIngressSPBytes[GetIngressSP(port,qIndex)]+psize < m_buffer_cell_limit_sp)				//doesn't exceed sp limit
			{
				m_usedIngressSPBytes[GetIngressSP(port,qIndex)]+=psize;
				m_usedIngressPortSharedBytes[port]+=psize;
				m_usedIngressPGSharedBytes[port][qIndex]+=psize;
				return;
			}
			else
			{
				m_usedIngressSPExceed[GetIngressSP(port,qIndex)] ++;
				if (m_usedIngressSPSharedBytes+psize<m_buffer_cell_limit_sp_shared)							//exceeds sp limit bu not sp share limit
				{
					m_usedIngressSPSharedBytes+=psize;
					m_usedIngressPortSharedBytes[port]+=psize;
					m_usedIngressPGSharedBytes[port][qIndex]+=psize;
				}
				else																						//exceeds sp share limit, use headroom
				{
					m_usedIngressHeadroomBytes += psize;
					//std::cout<<"Exceed SP pg#" << qIndex << "share limit!    "<<m_usedIngressHeadroomBytes<<"\n";
					//std::cout<<m_usedIngressSPSharedBytes<<"\t"<<m_usedIngressSPBytes[GetIngressSP(port,qIndex)]<<"\n";
					m_usedIngressSPSharedExceed ++;
					return;
				}
			}
		}
		else																								//exceeds port share limit, use headroom
		{
			m_usedIngressHeadroomBytes += psize;
			//std::cout<<"Exceed Port pg#" << qIndex << "share limit!    "<<m_usedIngressHeadroomBytes<<"\n";
			m_usedIngressPortSharedExceed[port] ++;
			return;
		}
	}
	else																									//exceeds pg share limit, use headroom
	{

		m_usedIngressHeadroomBytes += psize;
		//std::cout<<"Exceed PG pg#" << qIndex << "share limit!    "<<m_usedIngressHeadroomBytes<<"\n";
		m_usedIngressPGSharedExceed[port][qIndex] ++;
		return;
	}
	return;
}

void 
Node::UpdateEgressAdmission(uint32_t port,uint32_t qIndex,uint32_t psize)
{

	if (m_usedEgressQMinBytes[port][qIndex]+psize<m_q_min_cell)	//guaranteed
	{
		m_usedEgressQMinBytes[port][qIndex]+=psize;
		return;
	}
	else
	{
		m_usedEgressQSharedBytes[port][qIndex]+=psize;
		m_usedEgressPortBytes[port]+=psize;
		m_usedEgressSPBytes[GetEgressSP(port,qIndex)]+=psize;
	}

	return;
}

void
Node::RemoveFromIngressAdmission(uint32_t port,uint32_t qIndex,uint32_t psize)
{
	m_usedTotalBytes -= psize;
	if (m_usedIngressPGMinExceed[port][qIndex]==0)																		//doesn't exceed pg min
	{
		//std::cout<<"!!!!!!!!!!!!\t"<<port<<"\t"<<qIndex<<"\n";
		m_usedIngressPGMinBytes[port][qIndex]-=psize;
		return;
	}
	else
	{
		m_usedIngressPGMinExceed[port][qIndex]--;
	}

	if (m_usedIngressPortMinExceed[port]==0)																		//doesn't exceed port min
	{
		//std::cout<<"@@@@@@@@@@@@\t"<<port<<"\t"<<qIndex<<"\n";
		m_usedIngressPortMinBytes[port]-=psize;
		return;
	}
	else
	{
		m_usedIngressPortMinExceed[port]--;
	}

	if (m_usedIngressPGSharedExceed[port][qIndex]>0)																//exceeds pg share limit, remove from headroom
	{

		m_usedIngressHeadroomBytes -= psize;
		//std::cout<<"Remove from headroom for pg#" << qIndex <<"!           " << m_usedIngressHeadroomBytes<<"\n";
		m_usedIngressPGSharedExceed[port][qIndex] --;
		return;
	}
	if (m_usedIngressPortSharedExceed[port]>0)																		//exceeds port share limit, remove from headroom
	{
		m_usedIngressHeadroomBytes -= psize;
		m_usedIngressPortSharedExceed[port] --;
		return;
	}
	if (m_usedIngressSPSharedExceed>0)																				//exceeds sp share limit, remove from headroom
	{
		m_usedIngressHeadroomBytes -= psize;
		m_usedIngressSPSharedExceed --;
		return;
	}
	if (m_usedIngressSPExceed[GetIngressSP(port,qIndex)]>0)															//exceeds sp limit, remove from shared sp buffer
	{
		m_usedIngressSPSharedBytes-=psize;
		m_usedIngressPortSharedBytes[port]-=psize;
		m_usedIngressPGSharedBytes[port][qIndex]-=psize;
		m_usedIngressSPExceed[GetIngressSP(port,qIndex)]--;
		return;
	}
	else																											//nothing special
	{
		m_usedIngressSPBytes[GetIngressSP(port,qIndex)]-=psize;
		m_usedIngressPortSharedBytes[port]-=psize;
		m_usedIngressPGSharedBytes[port][qIndex]-=psize;
	}
	return;
}

void 
Node::RemoveFromEgressAdmission(uint32_t port,uint32_t qIndex,uint32_t psize)
{
	//m_usedEgressPGBytes[qIndex] -= psize;
	//m_usedEgressPortBytes[port] -= psize;
	//m_usedEgressSPBytes[GetIngressSP(port,qIndex)] -= psize;

	if (m_usedEgressQSharedBytes[port][qIndex]>0)
	{
		m_usedEgressQSharedBytes[port][qIndex]-=psize;
		m_usedEgressPortBytes[port]-=psize;
		m_usedEgressSPBytes[GetIngressSP(port,qIndex)]-=psize;
	}
	else
	{
		m_usedEgressQMinBytes[port][qIndex]-=psize;
	}

	return;
}

void
Node::GetPauseClasses(uint32_t port, uint32_t qIndex, bool pClasses[])
{
	if (m_usedIngressPortSharedExceed[port]>0)					//pause the whole port
	{
		for (int i=0;i<8;i++)
		{
			pClasses[i]=true;
		}
		return;
	}
	else
	{
		for (int i=0;i<8;i++)
		{
			pClasses[i]=false;
		}
	}
	

	if (m_usedIngressPGSharedExceed[port][qIndex]>0)
	{
		pClasses[qIndex]=true;
	}
	//what if sp share buffer is exceeded?

	return;
}

uint32_t 
Node::GetIngressSP(uint32_t port,uint32_t pgIndex)
{
	return 0;
}

uint32_t 
Node::GetEgressSP(uint32_t port,uint32_t qIndex)
{
	return 0;
}


*/



} // namespace ns3
