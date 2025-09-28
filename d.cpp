// In lib/Target/x86_64/x86_64GOT.h
class x86_64GOTPLTN : public x86_64GOT {
public:
  x86_64GOTPLTN(ELFSection *O, ResolveInfo *R)
      : x86_64GOT(GOT::GOTPLTN, O, R, /*Align=*/8, /*Size=*/8), 
        m_PLTEntry(nullptr) {}

  x86_64GOT *getFirst() override { return this; }
  x86_64GOT *getNext() override { return nullptr; }

  void setPLTEntry(Fragment *plt) { m_PLTEntry = plt; }

  // Override getContent to return PLT+6 address (second instruction)
  virtual llvm::ArrayRef<uint8_t> getContent() const override {
    Value = 0;
    
    // If we have a PLT entry, point to its second instruction (offset 6)
    // This is the "push" instruction in the PLT stub
    if (m_PLTEntry) {
      ELFSection *pltSection = m_PLTEntry->getOwningSection();
      if (pltSection) {
        Value = pltSection->addr() + m_PLTEntry->getOffset() + 6;
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
