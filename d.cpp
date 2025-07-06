// //===-
// x86_64PLT.cpp-------------------------------------------------------===//
// // Part of the eld Project, under the BSD License
// // See https://github.com/qualcomm/eld/LICENSE.txt for license information.
// // SPDX-License-Identifier: BSD-3-Clause
// //===----------------------------------------------------------------------===//
// #include "x86_64PLT.h"
// #include "eld/Readers/ELFSection.h"
// #include "eld/Readers/Relocation.h"
// #include "eld/Support/Memory.h"

// using namespace eld;

// // PLT0
// x86_64PLT0 *x86_64PLT0::Create(eld::IRBuilder &I, x86_64GOT *G, ELFSection
// *O,
//                                ResolveInfo *R, bool BindNow) {
//   // No need of PLT0 when binding now.
//   if (BindNow)
//     return nullptr;
//   x86_64PLT0 *P = make<x86_64PLT0>(G, I, O, R, 16, 16);
//   O->addFragmentAndUpdateSize(P);

//   Relocation *r1 = Relocation::Create(llvm::ELF::R_X86_64_PC32, 32,
//                                       make<FragmentRef>(*P, 2), 8);
//   r1->setSymInfo(G->symInfo());
//   O->addRelocation(r1);

//   Relocation *r2 = Relocation::Create(llvm::ELF::R_X86_64_PC32, 32,
//                                       make<FragmentRef>(*P, 10), 8);
//   r2->setSymInfo(G->symInfo());
//   O->addRelocation(r2);

//   // // Create a relocation and point to the GOT.
//   // Relocation *r1 = nullptr;
//   // Relocation *r2 = nullptr;

//   // std::string name = "__gotplt0__";
//   // // create LDSymbol for the stub
//   // LDSymbol *symbol = I.addSymbol<IRBuilder::Force, IRBuilder::Resolve>(
//   //     O->getInputFile(), name, ResolveInfo::NoType, ResolveInfo::Define,
//   //     ResolveInfo::Local,
//   //     8, // size
//   //     0, // value
//   //     make<FragmentRef>(*G, 0), ResolveInfo::Internal,
//   //     true /* isPostLTOPhase */);
//   // symbol->setShouldIgnore(false);

//   // r1 = Relocation::Create(llvm::ELF::R_X86_64_JUMP_SLOT, 64,
//   //                         make<FragmentRef>(*P, 0), 0);
//   // r1->setSymInfo(symbol->resolveInfo());
//   // r2 = Relocation::Create(llvm::ELF::R_X86_64_JUMP_SLOT, 64,
//   //                         make<FragmentRef>(*P, 8), 4);
//   // r2->setSymInfo(symbol->resolveInfo());
//   // O->addRelocation(r1);
//   // O->addRelocation(r2);

//   return P;
// }

// // PLTN
// x86_64PLTN *x86_64PLTN::Create(eld::IRBuilder &I, x86_64GOT *G, ELFSection
// *O,
//                                ResolveInfo *R, bool BindNow) {
//   x86_64PLTN *P = make<x86_64PLTN>(G, I, O, R, 16, 16);
//   O->addFragmentAndUpdateSize(P);

//   Relocation *r1 = Relocation::Create(llvm::ELF::R_X86_64_PC32, 32,
//                                       make<FragmentRef>(*P, 2), 0);
//   r1->setSymInfo(G->symInfo());
//   O->addRelocation(r1);

//   if (!BindNow) {
//     Fragment *PLT0 = *O->getFragmentList().begin();
//     Relocation *r2 = Relocation::Create(llvm::ELF::R_X86_64_PC32, 32,
//                                         make<FragmentRef>(*P, 12), 0);
//     FragmentRef *PLT0Ref = make<FragmentRef>(*PLT0, 0);
//     r2->modifyRelocationFragmentRef(PLT0Ref);
//     O->addRelocation(r2);
//   }

//   // // Create a relocation and point to the GOT.
//   // Relocation *r1 = nullptr;
//   // Relocation *r2 = nullptr;
//   // std::string name = "__gotpltn_for_" + std::string(R->name());
//   // // create LDSymbol for the stub
//   // LDSymbol *symbol = I.addSymbol<IRBuilder::Force, IRBuilder::Resolve>(
//   //     O->getInputFile(), name, ResolveInfo::NoType, ResolveInfo::Define,
//   //     ResolveInfo::Local,
//   //     8, // size
//   //     0, // value
//   //     make<FragmentRef>(*G, 0), ResolveInfo::Internal,
//   //     true /* isPostLTOPhase */);
//   // symbol->setShouldIgnore(false);
//   // r1 = Relocation::Create(llvm::ELF::R_X86_64_JUMP_SLOT, 64,
//   //                         make<FragmentRef>(*P, 0), 0);
//   // r1->setSymInfo(symbol->resolveInfo());
//   // r2 = Relocation::Create(llvm::ELF::R_X86_64_JUMP_SLOT, 64,
//   //                         make<FragmentRef>(*P, 8), 8);
//   // r2->setSymInfo(symbol->resolveInfo());
//   // O->addRelocation(r1);
//   // O->addRelocation(r2);

//   // // No PLT0 for immediate binding.
//   // if (BindNow)
//   //   return P;

//   // Fragment *F = *(O->getFragmentList().begin());
//   // FragmentRef *PLT0FragRef = make<FragmentRef>(*F, 0);
//   // Relocation *r3 = Relocation::Create(llvm::ELF::R_X86_64_JUMP_SLOT, 64,
//   //                                     make<FragmentRef>(*G, 0), 0);
//   // O->addRelocation(r3);
//   // r3->modifyRelocationFragmentRef(PLT0FragRef);
//   return P;
// }

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
    // Create symbols for GOT+8 and GOT+16 references
    std::string name1 = "__gotplt_resolver_link_map__";
    LDSymbol *symbol1 = I.addSymbol<IRBuilder::Force, IRBuilder::Resolve>(
        O->getInputFile(), name1, ResolveInfo::NoType, ResolveInfo::Define,
        ResolveInfo::Local, 8, 0, make<FragmentRef>(*G, 8),
        ResolveInfo::Default, true);
    symbol1->setShouldIgnore(false);

    std::string name2 = "__gotplt_resolver_function__";
    LDSymbol *symbol2 = I.addSymbol<IRBuilder::Force, IRBuilder::Resolve>(
        O->getInputFile(), name2, ResolveInfo::NoType, ResolveInfo::Define,
        ResolveInfo::Local, 8, 0, make<FragmentRef>(*G, 16),
        ResolveInfo::Default, true);
    symbol2->setShouldIgnore(false);

    // pushq GOTPLT+8(%rip) at offset 2 - link_map pointer
    Relocation *r1 = Relocation::Create(llvm::ELF::R_X86_64_PC32, 32,
                                        make<FragmentRef>(*P, 2), -4);
    r1->setSymInfo(symbol1->resolveInfo());
    O->addRelocation(r1);

    // jmp *GOTPLT+16(%rip) at offset 8 - resolver function  
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
    // Create symbol for this PLT entry's GOT reference
    std::string name = "__gotpltn_for_" + std::string(R->name());
    LDSymbol *symbol = I.addSymbol<IRBuilder::Force, IRBuilder::Resolve>(
        O->getInputFile(), name, ResolveInfo::NoType, ResolveInfo::Define,
        ResolveInfo::Local, 8, 0, make<FragmentRef>(*G, 0),
        ResolveInfo::Default, true);
    symbol->setShouldIgnore(false);

    // jmp *GOTPLT_entry(%rip) at offset 2 - jump to function
    Relocation *r1 = Relocation::Create(llvm::ELF::R_X86_64_PC32, 32,
                                        make<FragmentRef>(*P, 2), -4);
    r1->setSymInfo(symbol->resolveInfo());
    O->addRelocation(r1);

    // pushq $index at offset 7 - relocation index 
    // Calculate PLT entry index (PLT entries after PLT0)
    size_t pltIndex = O->getFragmentList().size() - 1; // -1 because PLT0 is first
    Relocation *r_index = Relocation::Create(llvm::ELF::R_X86_64_32, 32,
                                           make<FragmentRef>(*P, 7), 0);
    r_index->setAddend(pltIndex);
    O->addRelocation(r_index);

    // jmp PLT0 at offset 12 - jump to resolver
    if (!BindNow && O->getFragmentList().size() > 1) {
      Fragment *PLT0 = *(O->getFragmentList().begin());
      std::string plt0_name = "__plt0_resolver__";
      LDSymbol *plt0_symbol = I.addSymbol<IRBuilder::Force, IRBuilder::Resolve>(
          O->getInputFile(), plt0_name, ResolveInfo::NoType,
          ResolveInfo::Define, ResolveInfo::Local, 16, 0,
          make<FragmentRef>(*PLT0, 0), ResolveInfo::Default, true);
      plt0_symbol->setShouldIgnore(false);

      Relocation *r2 = Relocation::Create(llvm::ELF::R_X86_64_PC32, 32,
                                          make<FragmentRef>(*P, 12), -4);
      r2->setSymInfo(plt0_symbol->resolveInfo());
      O->addRelocation(r2);
    }
  }

  return P;
}
