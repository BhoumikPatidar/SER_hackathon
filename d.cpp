// In lib/Target/x86_64/x86_64GOT.h
#include "eld/Core/Module.h"

class x86_64GOTPLT0 : public x86_64GOT {
public:
  x86_64GOTPLT0(ELFSection *O, Module *M)
      : x86_64GOT(GOT::GOTPLT0, O, nullptr, /*Align=*/8, /*Size=*/24),
        m_Module(M) {}

  x86_64GOT *getFirst() override { return this; }
  x86_64GOT *getNext() override { return nullptr; }

  // Override getContent to return .dynamic address for first 8 bytes
  virtual llvm::ArrayRef<uint8_t> getContent() const override {
    memset(Value, 0, sizeof(Value));
    
    // Get .dynamic section address for GOTPLT[0]
    if (m_Module) {
      ELFSection *dynSection = m_Module->getSection(".dynamic");
      if (dynSection) {
        uint64_t dynAddr = dynSection->addr();
        memcpy(Value, &dynAddr, sizeof(uint64_t));
      }
    }
    
    // GOTPLT[1] and GOTPLT[2] remain 0 (filled by dynamic linker)
    return llvm::ArrayRef(Value, sizeof(Value));
  }

  static x86_64GOTPLT0 *Create(ELFSection *O, Module *M);

private:
  Module *m_Module;
  mutable uint8_t Value[24];  // 3 x 8-byte entries
};
