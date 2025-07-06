#include "x86_64PLT.h"
#include "eld/Readers/ELFSection.h"
#include "eld/Readers/Relocation.h"
#include "eld/Support/Memory.h"

using namespace eld;

// PLT0
x86_64PLT0 *x86_64PLT0::Create(eld::IRBuilder &I, x86_64GOT *G, ELFSection *O,
                               ResolveInfo *R, bool BindNow) {
  if (BindNow)
    return nullptr;

  x86_64PLT0 *P = make<x86_64PLT0>(G, I, O, R, 16, 16);
  O->addFragmentAndUpdateSize(P);

  if (G) {
    // pushq GOTPLT+8(%rip) at offset 2 - link_map pointer
    Relocation *r1 = Relocation::Create(llvm::ELF::R_X86_64_PC32, 32,
                                        make<FragmentRef>(*P, 2), 4);
    r1->setTargetRef(make<FragmentRef>(*G, 8));
    O->addRelocation(r1);

    // jmp *GOTPLT+16(%rip) at offset 8 - resolver function
    Relocation *r2 = Relocation::Create(llvm::ELF::R_X86_64_PC32, 32,
                                        make<FragmentRef>(*P, 8), 4);
    r2->setTargetRef(make<FragmentRef>(*G, 16));
    O->addRelocation(r2);
  }

  return P;
}

// PLTN
x86_64PLTN *x86_64PLTN::Create(eld::IRBuilder &I, x86_64GOT *G, ELFSection *O,
                               ResolveInfo *R, bool BindNow) {
  x86_64PLTN *P = make<x86_64PLTN>(G, I, O, R, 16, 16);
  O->addFragmentAndUpdateSize(P);

  if (G && R) {
    // jmp *GOTPLT_entry(%rip) at offset 2 - jump to function
    Relocation *r1 = Relocation::Create(llvm::ELF::R_X86_64_PC32, 32,
                                        make<FragmentRef>(*P, 2), 2);
    r1->setTargetRef(make<FragmentRef>(*G, 0));
    O->addRelocation(r1);

    // pushq $index at offset 7 - relocation index (set during layout)
    // The index will be set when the PLT is finalized

    // jmp PLT0 at offset 12 - jump to resolver
    if (!BindNow && O->getFragmentList().size() > 1) {
      Fragment *PLT0 = *(O->getFragmentList().begin());
      Relocation *r2 = Relocation::Create(llvm::ELF::R_X86_64_PC32, 32,
                                          make<FragmentRef>(*P, 12), 0);
      r2->setTargetRef(make<FragmentRef>(*PLT0, 0));
      O->addRelocation(r2);
    }
  }

  return P;
}
