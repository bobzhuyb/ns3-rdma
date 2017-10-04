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

		void ScheduleTransmit(Time dt);
		void Send(void);
		void Receive(Ptr<Socket> socket);

		uint32_t m_count;
		uint64_t m_allowed;
		Time m_interval;
		uint32_t m_size;

		uint32_t m_sent;
		Ptr<Socket> m_socket;
		Address m_peerAddress;
		uint16_t m_peerPort;
		EventId m_sendEvent;

		uint16_t m_pg;

		// Timely parameters.
		uint32_t m_C;		// link speed in bits per second.
		uint32_t m_delta;	// additive increase step in bits per second. 
		uint32_t m_t_high;	// t_high in microseconds.
		uint32_t m_t_low;	// t_low in microseconds.
		uint32_t m_min_rtt;	// min rtt in seconds.
		double m_beta;		// beta;
		double m_alpha;		// alpha;
	};

} // namespace ns3

#endif /* TIMELy_SENDER_H */
