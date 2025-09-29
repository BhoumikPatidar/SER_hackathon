// In lib/Target/x86_64/x86_64GOT.h

class x86_64GOTPLTN : public x86_64GOT {
public:
  x86_64GOTPLTN(ELFSection *O, ResolveInfo *R)
      : x86_64GOT(GOT::GOTPLTN, O, R, /*Align=*/8, /*Size=*/8), 
        m_PLTEntry(nullptr) {}

  x86_64GOT *getFirst() override { return this; }
  x86_64GOT *getNext() override { return nullptr; }

  void setPLTEntry(Fragment *plt) { m_PLTEntry = plt; }

  virtual llvm::ArrayRef<uint8_t> getContent() const override {
    Value = 0;
    
    // Calculate PLT entry address correctly
    if (m_PLTEntry && m_PLTEntry->getOwningSection()) {
      ELFSection *pltSection = m_PLTEntry->getOwningSection();
      
      // Count how many PLT entries come before ours
      unsigned pltIndex = 0;
      bool foundEntry = false;
      for (Fragment *F : pltSection->getFragmentList()) {
        if (F == m_PLTEntry) {
          foundEntry = true;
          break;
        }
        pltIndex++;
      }
      
      // If this is the first PLTN (index 1, after PLT0), offset is 16
      // For subsequent PLTNs, add 16 for each
      if (foundEntry && pltIndex > 0) {
        // PLT0 is 16 bytes, each PLTN is 16 bytes
        // We want to point to the second instruction (+6) of our PLTN
        Value = pltSection->addr() + (pltIndex * 16) + 6;
      }
    }
    
    return llvm::ArrayRef(reinterpret_cast<const uint8_t *>(&Value),
                          sizeof(Value));
  }

  static x86_64GOTPLTN *Create(ELFSection *O, ResolveInfo *R);

private:
  Fragment *m_PLTEntry;
  mutable uint64_t Value;
};
