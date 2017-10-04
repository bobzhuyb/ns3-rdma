//yibo: not used for now

#include <stdint.h>
#include <iostream>
#include "qbb-header.h"
#include "ns3/buffer.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("qbbHeader");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (qbbHeader);

qbbHeader::qbbHeader (uint32_t pg)
  : m_pg(pg)
{
}

qbbHeader::qbbHeader ()
  : m_pg(0)
{}

qbbHeader::~qbbHeader ()
{}

void qbbHeader::SetPG (uint32_t pg)
{
  m_pg = pg;
}

uint32_t qbbHeader::GetPG () const
{
  return m_pg;
}


TypeId
qbbHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::qbbHeader")
    .SetParent<Header> ()
    .AddConstructor<qbbHeader> ()
    ;
  return tid;
}
TypeId
qbbHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
void qbbHeader::Print (std::ostream &os) const
{
  os << "qbb=" << m_pg ;
}
uint32_t qbbHeader::GetSerializedSize (void)  const
{
  return 4;
}
void qbbHeader::Serialize (Buffer::Iterator start)  const
{
  start.WriteU32 (m_pg);
}

uint32_t qbbHeader::Deserialize (Buffer::Iterator start)
{
  m_pg = start.ReadU32 ();
  return GetSerializedSize ();
}


}; // namespace ns3
