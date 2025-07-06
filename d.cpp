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
    // Create symbol for GOT+8 reference (like AArch64 does)
    std::string name1 = "__gotplt_resolver_link_map__";
    LDSymbol *symbol1 = I.addSymbol<IRBuilder::Force, IRBuilder::Resolve>(
        O->getInputFile(), name1, ResolveInfo::NoType, ResolveInfo::Define,
        ResolveInfo::Local, 8, 0, make<FragmentRef>(*G, 8),  // ← GOT+8
        ResolveInfo::Default, true);
    symbol1->setShouldIgnore(false);

    // Create symbol for GOT+16 reference  
    std::string name2 = "__gotplt_resolver_function__";
    LDSymbol *symbol2 = I.addSymbol<IRBuilder::Force, IRBuilder::Resolve>(
        O->getInputFile(), name2, ResolveInfo::NoType, ResolveInfo::Define,
        ResolveInfo::Local, 8, 0, make<FragmentRef>(*G, 16), // ← GOT+16
        ResolveInfo::Default, true);
    symbol2->setShouldIgnore(false);

    // Fix pushq GOTPLT+8(%rip) at offset 2 - zero addend like AArch64
    Relocation *r1 = Relocation::Create(llvm::ELF::R_X86_64_PC32, 32,
                                        make<FragmentRef>(*P, 2), -4);
    r1->setSymInfo(symbol1->resolveInfo());
    O->addRelocation(r1);
    
    // Fix jmp *GOTPLT+16(%rip) at offset 8 - zero addend like AArch64
    Relocation *r2 = Relocation::Create(llvm::ELF::R_X86_64_PC32, 32,
                                        make<FragmentRef>(*P, 8), -4);
    r2->setSymInfo(symbol2->resolveInfo());
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
    // Create symbol for this PLT entry's GOT reference (exactly like AArch64)
    std::string name = "__gotpltn_for_" + std::string(R->name());
    LDSymbol *symbol = I.addSymbol<IRBuilder::Force, IRBuilder::Resolve>(
        O->getInputFile(), name, ResolveInfo::NoType, ResolveInfo::Define,
        ResolveInfo::Local, 8, 0, make<FragmentRef>(*G, 0), 
        ResolveInfo::Default, true);
    symbol->setShouldIgnore(false);

    // Fix jmp *GOT_symbol(%rip) at offset 2 - zero addend like AArch64
    Relocation *r1 = Relocation::Create(llvm::ELF::R_X86_64_PC32, 32,
                                        make<FragmentRef>(*P, 2), -4);
    r1->setSymInfo(symbol->resolveInfo());
    O->addRelocation(r1);

    // PLT0 jump back - create symbol like AArch64 does
    if (!BindNow && O->getFragmentList().size() > 1) {
      Fragment *PLT0 = *(O->getFragmentList().begin());
      std::string plt0_name = "__plt0_resolver__";
      LDSymbol *plt0_symbol = I.addSymbol<IRBuilder::Force, IRBuilder::Resolve>(
          O->getInputFile(), plt0_name, ResolveInfo::NoType, ResolveInfo::Define,
          ResolveInfo::Local, 16, 0, make<FragmentRef>(*PLT0, 0), 
          ResolveInfo::Default, true);
      plt0_symbol->setShouldIgnore(false);
      
      Relocation *r2 = Relocation::Create(llvm::ELF::R_X86_64_PC32, 32,
                                          make<FragmentRef>(*P, 12), -4);
      r2->setSymInfo(plt0_symbol->resolveInfo());
      O->addRelocation(r2);
    }
  }

  return P;
}
