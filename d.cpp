// In lib/Target/x86_64/x86_64PLT.h

class x86_64PLTN : public x86_64PLT {
public:
  x86_64PLTN(x86_64GOT *G, eld::IRBuilder &I, ELFSection *P, ResolveInfo *R,
             uint32_t Align, uint32_t Size)
      : x86_64PLT(PLT::PLTN, I, G, P, R, Align, Size), m_RelocIndex(0) {}

  virtual ~x86_64PLTN() {}

  // Set the relocation index for this PLT entry
  void setRelocIndex(uint32_t index) { m_RelocIndex = index; }
  
  virtual llvm::ArrayRef<uint8_t> getContent() const override {
    // Copy the template and fill in the relocation index
    memcpy(m_Content, x86_64_plt1, sizeof(x86_64_plt1));
    
    // Fill in the relocation index at offset 7 (little-endian 32-bit value)
    uint32_t index = m_RelocIndex;
    m_Content[7] = index & 0xFF;
    m_Content[8] = (index >> 8) & 0xFF;
    m_Content[9] = (index >> 16) & 0xFF;
    m_Content[10] = (index >> 24) & 0xFF;
    
    return llvm::ArrayRef<uint8_t>(m_Content, sizeof(m_Content));
  }

  static x86_64PLTN *Create(eld::IRBuilder &I, x86_64GOT *G, ELFSection *O,
                            ResolveInfo *R, bool HasNow);

private:
  uint32_t m_RelocIndex;
  mutable uint8_t m_Content[16];  // Mutable buffer for PLT stub content
};
