#ifndef TIMELY_SENDER_H
#define TIMELY_SENDER_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"

namespace ns3 {

	class Socket;
	class Packet;

	/**
	* \ingroup timelysenderreceiver
	* \class Timely sender
	* \brief UDP sender that implements TIMELY protocol.
	*
	*/
	class TimelySender : public Application
	{
	public:
		static TypeId
			GetTypeId(void);

		TimelySender();

		virtual ~TimelySender();

		/**
		* \brief set the remote address and port
		* \param ip remote IP address
		* \param port remote port
		*/
		void SetRemote(Ipv4Address ip, uint16_t port);
		void SetRemote(Ipv6Address ip, uint16_t port);
		void SetRemote(Address ip, uint16_t port);
		void SetPG(uint16_t pg);

	protected:
		virtual void DoDispose(void);

	private:

		virtual void StartApplication(void);
		virtual void StopApplication(void);

		void Init();
		void ScheduleTransmit(Time dt);
		void Send(void);
		void SendBurst();
		void SendPaced();
		void SendPacket();
		void Receive(Ptr<Socket> socket);
		void UpdateSendRate();
		double GenerateRTTSample(Time ts);
		double GetBurstDuration(double rate);
		double GetBurstDuration(int packets, double rate);

		// Basic networking parameters. 
		Ptr<Socket> m_socket;
		Address m_peerAddress;
		uint16_t m_peerPort;
		EventId m_sendEvent;
		uint16_t m_pg;

		// General paremetrs.
		uint64_t m_allowed; // max packets to send
		uint32_t m_pktSize; // packets size. 
		uint32_t m_burstSize; // we send these many packets in a burst - this is the smallest unit of rate control.

		// Timely algorithm parameters.
		double m_C;		// link speed in bits per second.
		double m_initRate; // initial sending rate.
		uint32_t m_delta;	// additive increase step in bits per second. 
		double m_t_high;	// t_high in seconds
		double m_t_low;	// t_low in seconds
		double m_min_rtt;	// min rtt in seconds.
		double m_beta;		// beta;
		double m_alpha;		// alpha;
		double m_maxRateMultiple; // the rate cannot exceed this value times m_C. This simulates the fact that the host cannot send faster than link speed, minus header.
		double m_minRateMultiple; // the rate cannot go below this value times m_C. This is mostly a safeguard.

		// Timely variables.
		double m_rate; // current sending rate.
		double m_prev_rtt; // previous RTT sample.
		double m_new_rtt; // new RTT sample.
		double m_rtt_diff; // new RTT diff
		double m_N; // 5 if we are in HAI mode, 1 otherwise.

		// Bookkeeping
		double m_sleep;
		int m_burst_in_packets;
		uint32_t m_sent; 
		uint32_t m_received;
		double m_sdel; // serialization delay.
		double m_maxRate;
		double m_minRate;
	};

} // namespace ns3

#endif /* TIMELy_SENDER_H */
