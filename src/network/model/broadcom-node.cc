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
* Author: Yibo Zhu <yibzh@microsoft.com>
*/
#include <iostream>
#include <fstream>
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
#include "ns3/broadcom-node.h"
#include "ns3/random-variable.h"

NS_LOG_COMPONENT_DEFINE("BroadcomNode");

namespace ns3 {

	NS_OBJECT_ENSURE_REGISTERED(BroadcomNode);

	TypeId
		BroadcomNode::GetTypeId(void)
	{
		static TypeId tid = TypeId("ns3::BroadcomNode")
			.SetParent<Object>()
			.AddConstructor<BroadcomNode>();
		return tid;
	}


	BroadcomNode::BroadcomNode()
	{
		m_maxBufferBytes = 9000000; //9MB
		m_usedTotalBytes = 0;

		for (uint32_t i = 0; i < pCnt; i++)
		{
			m_usedIngressPortBytes[i] = 0;
			m_usedEgressPortBytes[i] = 0;
			for (uint32_t j = 0; j < qCnt; j++)
			{
				m_usedIngressPGBytes[i][j] = 0;
				m_usedIngressPGHeadroomBytes[i][j] = 0;
				m_usedEgressQMinBytes[i][j] = 0;
				m_usedEgressQSharedBytes[i][j] = 0;
				m_pause_remote[i][j] = false;
			}
		}
		for (int i = 0; i < 4; i++)
		{
			m_usedIngressSPBytes[i] = 0;
			m_usedEgressSPBytes[i] = 0;
		}
		//ingress params
		m_buffer_cell_limit_sp = 4000 * 1030; //ingress sp buffer threshold
		//m_buffer_cell_limit_sp_shared=4000*1030; //ingress sp buffer shared threshold, nonshare -> share
		m_pg_min_cell = 1030; //ingress pg guarantee
		m_port_min_cell = 1030; //ingress port guarantee
		m_pg_shared_limit_cell = 20 * 1030; //max buffer for an ingress pg
		m_port_max_shared_cell = 4800 * 1030; //max buffer for an ingress port
		m_pg_hdrm_limit = 100 * 1030; //ingress pg headroom
		m_port_max_pkt_size = 100 * 1030; //ingress global headroom
		//still needs reset limits..
		m_port_min_cell_off = 4700 * 1030;
		m_pg_shared_limit_cell_off = m_pg_shared_limit_cell - 2 * 1030;
		//egress params
		m_op_buffer_shared_limit_cell = m_maxBufferBytes; //per egress sp limit
		m_op_uc_port_config_cell = m_maxBufferBytes; //per egress port limit
		m_q_min_cell = 1 * 1030;
		m_op_uc_port_config1_cell = m_maxBufferBytes;
		//qcn
		m_pg_qcn_threshold = 60 * 1030;
		m_pg_qcn_threshold_max = 60 * 1030;
		m_pg_qcn_maxp = 0.1;
		//dynamic threshold
		m_dynamicth = false;
		//pfc and dctcp
		m_enable_pfc_on_dctcp = 1;
		m_dctcp_threshold = 40 * 1030;
		m_dctcp_threshold_max = 400 * 1030;
		m_pg_shared_alpha_cell = 16;
		m_port_shared_alpha_cell = 128;   //not used for now. not sure whether this is used on switches
		m_pg_shared_alpha_cell_off_diff = 16;
		m_port_shared_alpha_cell_off_diff = 16;
		m_log_start = 2.1;
		m_log_end = 2.2;
		m_log_step = 0.00001;

	}

	BroadcomNode::~BroadcomNode()
	{}

	bool
		BroadcomNode::CheckIngressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize)
	{
		if (m_usedTotalBytes + psize > m_maxBufferBytes)  //buffer full, usually should not reach here.
		{
			std::cout << "WARNING: Drop because ingress buffer full\n";
			return false;
		}
		if (m_usedIngressPGBytes[port][qIndex] + psize > m_pg_min_cell && m_usedIngressPortBytes[port] + psize > m_port_min_cell) // exceed guaranteed, use share buffer
		{
			if (m_usedIngressSPBytes[GetIngressSP(port, qIndex)] > m_buffer_cell_limit_sp)
			{
				if (m_usedIngressPGHeadroomBytes[port][qIndex] + psize > m_pg_hdrm_limit) // exceed headroom space
				{
					std::cout << "WARNING: Drop because ingress headroom full:" << m_usedIngressPGHeadroomBytes[port][qIndex] << "\t" << m_pg_hdrm_limit << "\n";
					return false;
				}
			}
		}
		return true;
	}




	bool
		BroadcomNode::CheckEgressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize)
	{
		if (m_usedEgressSPBytes[GetEgressSP(port, qIndex)] + psize > m_op_buffer_shared_limit_cell)  //exceed the sp limit
		{
			std::cout << "WARNING: Drop because egress SP buffer full\n";
			return false;
		}
		if (m_usedEgressPortBytes[port] + psize > m_op_uc_port_config_cell)	//exceed the port limit
		{
			std::cout << "WARNING: Drop because egress Port buffer full\n";
			return false;
		}
		if (m_usedEgressQSharedBytes[port][qIndex] + psize > m_op_uc_port_config1_cell) //exceed the queue limit
		{
			std::cout << "WARNING: Drop because egress Q buffer full\n";
			return false;
		}
		return true;
	}


	void
		BroadcomNode::UpdateIngressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize)
	{
		m_usedTotalBytes += psize; //count total buffer usage
		m_usedIngressSPBytes[GetIngressSP(port, qIndex)] += psize;
		m_usedIngressPortBytes[port] += psize;
		m_usedIngressPGBytes[port][qIndex] += psize;
		if (m_usedIngressSPBytes[GetIngressSP(port, qIndex)] > m_buffer_cell_limit_sp)	//begin to use headroom buffer
		{
			m_usedIngressPGHeadroomBytes[port][qIndex] += psize;
		}
		return;
	}

	void
		BroadcomNode::UpdateEgressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize)
	{

		if (m_usedEgressQMinBytes[port][qIndex] + psize < m_q_min_cell)	//guaranteed
		{
			m_usedEgressQMinBytes[port][qIndex] += psize;
			return;
		}
		else
		{
			m_usedEgressQSharedBytes[port][qIndex] += psize;
			m_usedEgressPortBytes[port] += psize;
			m_usedEgressSPBytes[GetEgressSP(port, qIndex)] += psize;
			if (m_usedEgressQMinBytes[port][qIndex] < 1030 && m_usedEgressQSharedBytes[port][qIndex]>1030)		//patch for different size of packets
			{
				m_usedEgressQSharedBytes[port][qIndex] = m_usedEgressQSharedBytes[port][qIndex] - 1030 + m_usedEgressQMinBytes[port][qIndex];
				m_usedEgressPortBytes[port] = m_usedEgressPortBytes[port] - 1030 + m_usedEgressQMinBytes[port][qIndex];
				m_usedEgressSPBytes[GetEgressSP(port, qIndex)] = m_usedEgressSPBytes[GetEgressSP(port, qIndex)] - 1030 + m_usedEgressQMinBytes[port][qIndex];
				m_usedEgressQMinBytes[port][qIndex] = 1030;
			}

		}
		return;
	}

	void
		BroadcomNode::RemoveFromIngressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize)
	{
		m_usedTotalBytes -= psize;
		m_usedIngressSPBytes[GetIngressSP(port, qIndex)] -= psize;
		m_usedIngressPortBytes[port] -= psize;
		m_usedIngressPGBytes[port][qIndex] -= psize;
		if ((double)m_usedIngressPGHeadroomBytes[port][qIndex] - psize > 0)
			m_usedIngressPGHeadroomBytes[port][qIndex] -= psize;
		else
			m_usedIngressPGHeadroomBytes[port][qIndex] = 0;
		return;
	}

	void
		BroadcomNode::RemoveFromEgressAdmission(uint32_t port, uint32_t qIndex, uint32_t psize)
	{
		if (m_usedEgressQSharedBytes[port][qIndex] >= psize)
		{
			m_usedEgressQSharedBytes[port][qIndex] -= psize;
			m_usedEgressPortBytes[port] -= psize;
			m_usedEgressSPBytes[GetIngressSP(port, qIndex)] -= psize;
		}
		else
		{
			m_usedEgressQMinBytes[port][qIndex] -= psize;
			if (m_usedEgressQMinBytes[port][qIndex] < 1030 && m_usedEgressQSharedBytes[port][qIndex]>1030)	//patch for different size of packets
			{
				m_usedEgressQSharedBytes[port][qIndex] = m_usedEgressQSharedBytes[port][qIndex] - 1030 + m_usedEgressQMinBytes[port][qIndex];
				m_usedEgressPortBytes[port] = m_usedEgressPortBytes[port] - 1030 + m_usedEgressQMinBytes[port][qIndex];
				m_usedEgressSPBytes[GetEgressSP(port, qIndex)] = m_usedEgressSPBytes[GetEgressSP(port, qIndex)] - 1030 + m_usedEgressQMinBytes[port][qIndex];
				m_usedEgressQMinBytes[port][qIndex] = 1030;
			}
		}
		return;
	}

	void
		BroadcomNode::GetPauseClasses(uint32_t port, uint32_t qIndex, bool pClasses[])
	{
		if (m_dynamicth)
		{
			for (uint32_t i = 0; i < qCnt; i++)
			{
				pClasses[i] = false;
				if (m_usedIngressPGBytes[port][i] <= m_pg_min_cell + m_port_min_cell)
					continue;
				if (i == 1 && !m_enable_pfc_on_dctcp)			//dctcp
					continue;

				if ((double)m_usedIngressPGBytes[port][i] - m_pg_min_cell - m_port_min_cell > m_pg_shared_alpha_cell*((double)m_buffer_cell_limit_sp - m_usedIngressSPBytes[GetIngressSP(port, qIndex)]))
				{
					pClasses[i] = true;
				}
			}
		}
		else
		{
			if (m_usedIngressPortBytes[port] > m_port_max_shared_cell)					//pause the whole port
			{
				for (uint32_t i = 0; i < qCnt; i++)
				{
					if (i == 1 && !m_enable_pfc_on_dctcp)	//dctcp
						pClasses[i] = false;

					pClasses[i] = true;
				}
				return;
			}
			else
			{
				for (uint32_t i = 0; i < qCnt; i++)
				{
					pClasses[i] = false;
				}
			}
			if (m_usedIngressPGBytes[port][qIndex] > m_pg_shared_limit_cell)
			{
				if (qIndex == 1 && !m_enable_pfc_on_dctcp)
					return;

				pClasses[qIndex] = true;
			}
		}
		return;
	}


	bool
		BroadcomNode::GetResumeClasses(uint32_t port, uint32_t qIndex)
	{
		if (m_dynamicth)
		{
			if ((double)m_usedIngressPGBytes[port][qIndex] - m_pg_min_cell - m_port_min_cell < m_pg_shared_alpha_cell*((double)m_buffer_cell_limit_sp - m_usedIngressSPBytes[GetIngressSP(port, qIndex)] - m_pg_shared_alpha_cell_off_diff))
			{
				return true;
			}
		}
		else
		{
			if (m_usedIngressPGBytes[port][qIndex] < m_pg_shared_limit_cell_off
				&& m_usedIngressPortBytes[port] < m_port_min_cell_off)
			{
				return true;
			}
		}
		return false;
	}

	uint32_t
		BroadcomNode::GetIngressSP(uint32_t port, uint32_t pgIndex)
	{
		if (pgIndex == 1)
			return 1;
		else
			return 0;
	}

	uint32_t
		BroadcomNode::GetEgressSP(uint32_t port, uint32_t qIndex)
	{
		if (qIndex == 1)
			return 1;
		else
			return 0;
	}

	bool
		BroadcomNode::ShouldSendCN(uint32_t indev, uint32_t ifindex, uint32_t qIndex)
	{
		if (qIndex == qCnt - 1)
			return false;
		if (qIndex == 1)	//dctcp
		{
			if (m_usedEgressQSharedBytes[ifindex][qIndex] > m_dctcp_threshold_max)
			{
				return true;
			}
			else
			{
				if (m_usedEgressQSharedBytes[ifindex][qIndex] > m_dctcp_threshold && m_dctcp_threshold != m_dctcp_threshold_max)
				{
					double p = 1.0 * (m_usedEgressQSharedBytes[ifindex][qIndex] - m_dctcp_threshold) / (m_dctcp_threshold_max - m_dctcp_threshold);
					if (UniformVariable(0, 1).GetValue() < p)
						return true;
				}
			}
			return false;
		}
		else
		{
			if (m_usedEgressQSharedBytes[ifindex][qIndex] > m_pg_qcn_threshold_max)
			{
				return true;
			}
			else if (m_usedEgressQSharedBytes[ifindex][qIndex] > m_pg_qcn_threshold && m_pg_qcn_threshold != m_pg_qcn_threshold_max)
			{
				double p = 1.0 * (m_usedEgressQSharedBytes[ifindex][qIndex] - m_pg_qcn_threshold) / (m_pg_qcn_threshold_max - m_pg_qcn_threshold) * m_pg_qcn_maxp;
				if (UniformVariable(0, 1).GetValue() < p)
					return true;
			}
			return false;
		}
	}


	void
		BroadcomNode::SetBroadcomParams(
			uint32_t buffer_cell_limit_sp, //ingress sp buffer threshold p.120
			uint32_t buffer_cell_limit_sp_shared, //ingress sp buffer shared threshold p.120, nonshare -> share
			uint32_t pg_min_cell, //ingress pg guarantee p.121					---1
			uint32_t port_min_cell, //ingress port guarantee						---2
			uint32_t pg_shared_limit_cell, //max buffer for an ingress pg			---3	PAUSE
			uint32_t port_max_shared_cell, //max buffer for an ingress port		---4	PAUSE
			uint32_t pg_hdrm_limit, //ingress pg headroom
			uint32_t port_max_pkt_size, //ingress global headroom
			uint32_t q_min_cell,	//egress queue guaranteed buffer
			uint32_t op_uc_port_config1_cell, //egress queue threshold
			uint32_t op_uc_port_config_cell, //egress port threshold
			uint32_t op_buffer_shared_limit_cell, //egress sp threshold
			uint32_t q_shared_alpha_cell,
			uint32_t port_share_alpha_cell,
			uint32_t pg_qcn_threshold)
	{
		m_buffer_cell_limit_sp = buffer_cell_limit_sp;
		m_buffer_cell_limit_sp_shared = buffer_cell_limit_sp_shared;
		m_pg_min_cell = pg_min_cell;
		m_port_min_cell = port_min_cell;
		m_pg_shared_limit_cell = pg_shared_limit_cell;
		m_port_max_shared_cell = port_max_shared_cell;
		m_pg_hdrm_limit = pg_hdrm_limit;
		m_port_max_pkt_size = port_max_pkt_size;
		m_q_min_cell = q_min_cell;
		m_op_uc_port_config1_cell = op_uc_port_config1_cell;
		m_op_uc_port_config_cell = op_uc_port_config_cell;
		m_op_buffer_shared_limit_cell = op_buffer_shared_limit_cell;
		m_pg_shared_alpha_cell = q_shared_alpha_cell;
		m_port_shared_alpha_cell = port_share_alpha_cell;
		m_pg_qcn_threshold = pg_qcn_threshold;
	}

	uint32_t
		BroadcomNode::GetUsedBufferTotal()
	{
		return m_usedTotalBytes;
	}

	void
		BroadcomNode::SetDynamicThreshold()
	{
		m_dynamicth = true;
		m_pg_shared_limit_cell = m_maxBufferBytes;	//using dynamic threshold, we don't respect the static thresholds anymore
		m_port_max_shared_cell = m_maxBufferBytes;
		return;
	}

	void
		BroadcomNode::SetMarkingThreshold(uint32_t kmin, uint32_t kmax, double pmax)
	{
		m_pg_qcn_threshold = kmin * 1030;
		m_pg_qcn_threshold_max = kmax * 1030;
		m_pg_qcn_maxp = pmax;
	}

	void
		BroadcomNode::SetTCPMarkingThreshold(uint32_t kmin, uint32_t kmax)
	{
		m_dctcp_threshold = kmin * 1030;
		m_dctcp_threshold_max = kmax * 1030;
	}


} // namespace ns3
