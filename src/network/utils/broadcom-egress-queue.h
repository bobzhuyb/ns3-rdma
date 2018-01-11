//yibo: one queue per port, also handle scheduling

#ifndef BROADCOM_EGRESS_H
#define BROADCOM_EGRESS_H

#include <queue>
#include "ns3/packet.h"
#include "queue.h"
#include "drop-tail-queue.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/event-id.h"

namespace ns3 {

class TraceContainer;

class BEgressQueue : public Queue {
public:
  static TypeId GetTypeId (void);
  static const unsigned fCnt = 128; //max number of queues, 128 for NICs
  static const unsigned qCnt = 8; //max number of queues, 8 for switches
  BEgressQueue ();
  virtual ~BEgressQueue();
  bool Enqueue (Ptr<Packet> p, uint32_t qIndex);
  Ptr<Packet> Dequeue (bool paused[]);
  Ptr<Packet> DequeueRR (bool paused[]);
  Ptr<Packet> DequeueNIC (bool paused[]);//QCN disable NIC
  Ptr<Packet> DequeueQCN (bool paused[], Time avail[], uint32_t m_findex_qindex_map[]);//QCN enable NIC
  uint32_t GetNBytes (uint32_t qIndex) const;
  uint32_t GetNBytesTotal () const;
  uint32_t GetLastQueue ();
  uint32_t m_fcount;
  void RecoverQueue(Ptr<DropTailQueue> buffer, uint32_t i);

private:
  bool DoEnqueue (Ptr<Packet> p, uint32_t qIndex);
  Ptr<Packet> DoDequeue (bool paused[]);
  Ptr<Packet> DoDequeueNIC (bool paused[]);
  Ptr<Packet> DoDequeueRR (bool paused[]);
  Ptr<Packet> DoDequeueQCN (bool paused[], Time avail[], uint32_t m_findex_qindex_map[]);
  //for compatibility
  virtual bool DoEnqueue (Ptr<Packet> p);
  virtual Ptr<Packet> DoDequeue (void);
  virtual Ptr<const Packet> DoPeek (void) const;
  std::queue<Ptr<Packet> > m_packets;
  uint32_t m_maxPackets;
  double m_maxBytes; //total bytes limit
  uint32_t m_qmincell; //guaranteed see page 126
  uint32_t m_queuelimit; //limit for each queue
  uint32_t m_shareused; //used bytes by sharing
  uint32_t m_bytesInQueue[fCnt];
  uint32_t m_bytesInQueueTotal;
  uint32_t m_rrlast;
  uint32_t m_qlast;
  QueueMode m_mode;
  std::vector<Ptr<Queue> > m_queues; // uc queues
  //For strict priority
  Time m_bwsatisfied[qCnt];
  DataRate m_minBW[qCnt];
};

} // namespace ns3

#endif /* DROPTAIL_H */
