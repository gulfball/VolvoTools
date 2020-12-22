#pragma once

#include <cstdint>
#include <vector>

#include "../j2534/J2534_v0404.h"

namespace common {

class CEMCanMessage {
public:
  explicit CEMCanMessage(const std::vector<uint8_t> &data);

  std::vector<uint8_t> data() const;

  PASSTHRU_MSG toPassThruMsg(unsigned long ProtocolID,
                             unsigned long Flags) const;

private:
  const std::vector<uint8_t> _data;
};

} // namespace common