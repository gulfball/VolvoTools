#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "../j2534/J2534_v0404.h"

namespace common {

enum class ECUType : uint8_t {
  ADM = 0x0B,
  AEM = 0x52,
  AUD = 0x6D,
  BCM = 0x01,
  CCM = 0x29,
  CEM = 0x50,
  CPM = 0x18,
  DDM = 0x43,
  DEM = 0x1A,
  DIM = 0x51,
  ECM_ME = 0x7A,
  EPS = 0x30,
  GPS = 0x72,
  IAM = 0x75,
  ICM = 0x54,
  KVM = 0x2D,
  MMM = 0x66,
  PAM = 0x63,
  PDM = 0x45,
  PHM = 0x64,
  PSM = 0x2E,
  SRS = 0x58,
  SUB = 0x68,
  SWM = 0x49,
  TCM = 0x6E,
  TMC = 0x73,
  TRM = 0x23,
};

class D2Message {
public:
  static constexpr size_t CanPayloadSize = 8;
  using DataType = std::array<uint8_t, CanPayloadSize>;

  static ECUType getECUType(const uint8_t *const buffer);
  static ECUType getECUType(const std::vector<uint8_t> &buffer);
  static D2Message makeD2Message(ECUType ecuType,
                                      std::vector<uint8_t> request);
  template <typename... Args>
  static D2Message makeD2Message(Args... args) {
    std::vector<uint8_t> request{static_cast<uint8_t>(args)...};
    return D2Message(request);
  }

  explicit D2Message(const std::vector<uint8_t> &data);
  explicit D2Message(const std::vector<DataType> &data);

//  std::vector<uint8_t> data() const;

  std::vector<PASSTHRU_MSG> toPassThruMsgs(unsigned long ProtocolID,
                                           unsigned long Flags) const;

private:
  const std::vector<DataType> _data;
};

} // namespace common