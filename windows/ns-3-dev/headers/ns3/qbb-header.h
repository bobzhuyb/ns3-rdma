//yibo

#ifndef QBB_HEADER_H
#define QBB_HEADER_H

#include <stdint.h>
#include "ns3/header.h"
#include "ns3/buffer.h"

namespace ns3 {

/**
 * \ingroup Pause
 * \brief Header for the Congestion Notification Message
 *
 * This class has two fields: The five-tuple flow id and the quantized
 * congestion level. This can be serialized to or deserialzed from a byte
 * buffer.
 */
 
class qbbHeader : public Header
{
public:
  qbbHeader (uint32_t pg);
  qbbHeader ();
  virtual ~qbbHeader ();

//Setters
  /**
   * \param pg The PG
   */
  void SetPG (uint32_t pg);

//Getters
  /**
   * \return The pg
   */
  uint32_t GetPG () const;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  uint32_t m_pg;
};

}; // namespace ns3

#endif /* QBB_HEADER */
