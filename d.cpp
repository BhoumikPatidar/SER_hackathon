bool isTruncatedX86_64(const RelocationInfo &RelocInfo, uint64_t Value) {
  // For signed relocations, check sign-extension overflow
  if (x86_64Relocs[RelocInfo.Type].IsSigned) {
    int64_t sValue = static_cast<int64_t>(Value);
    switch (RelocInfo.EncType) {
    case EncTy_8:
      return sValue < INT8_MIN || sValue > INT8_MAX;
    case EncTy_16:
      return sValue < INT16_MIN || sValue > INT16_MAX;
    case EncTy_32:
      return sValue < INT32_MIN || sValue > INT32_MAX;
    default:
      return false; // 64-bit values don't truncate
    }
  }
  
  // For unsigned relocations, check zero-extension overflow
  unsigned N = getNumberOfBits(RelocInfo.EncType);
  if (N >= 64) return false; // 64-bit values don't truncate
  
  uint64_t mask = (1ULL << N) - 1;
  return (Value & ~mask) != 0;
}
