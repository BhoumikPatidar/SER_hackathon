//===- x86_64PLT.cpp-------------------------------------------------------===//
// Part of the eld Project, under the BSD License
// See https://github.com/qualcomm/eld/LICENSE.txt for license information.
// SPDX-License-Identifier: BSD-3-Clause
//===----------------------------------------------------------------------===//
#include "x86_64PLT.h"
#include "eld/Readers/ELFSection.h"
#include "eld/Readers/Relocation.h"
#include "eld/Support/Memory.h"

using namespace eld;

// PLT0
x86_64PLT0 *x86_64PLT0::Create(eld::IRBuilder &I, x86_64GOT *G, ELFSection *O,
                               ResolveInfo *R, bool BindNow) {
  // No need of PLT0 when binding now.
  if (BindNow)
    return nullptr;
  x86_64PLT0 *P = make<x86_64PLT0>(G, I, O, R, 16, 16);
  O->addFragmentAndUpdateSize(P);

  // Create relocations to point to GOT.PLT[1] and GOT.PLT[2]
  Relocation *r1 = nullptr;
  Relocation *r2 = nullptr;

  // Create symbol for GOT.PLT[1] (offset 8 from GOT.PLT base)
  std::string name1 = "__gotplt1__";
  LDSymbol *symbol1 = I.addSymbol<IRBuilder::Force, IRBuilder::Resolve>(
      O->getInputFile(), name1, ResolveInfo::NoType, ResolveInfo::Define,
      ResolveInfo::Local,
      8, // size
      0, // value
      make<FragmentRef>(*G, 8), ResolveInfo::Internal,
      true /* isPostLTOPhase */);
  symbol1->setShouldIgnore(false);

  // Create symbol for GOT.PLT[2] (offset 16 from GOT.PLT base)
  std::string name2 = "__gotplt2__";
  LDSymbol *symbol2 = I.addSymbol<IRBuilder::Force, IRBuilder::Resolve>(
      O->getInputFile(), name2, ResolveInfo::NoType, ResolveInfo::Define,
      ResolveInfo::Local,
      8, // size
      0, // value
      make<FragmentRef>(*G, 16), ResolveInfo::Internal,
      true /* isPostLTOPhase */);
  symbol2->setShouldIgnore(false);

  // r1: pushq GOTPLT+8(%rip) - PC-relative relocation at offset 2
  r1 = Relocation::Create(llvm::ELF::R_X86_64_GOTPCREL, 32,
                          make<FragmentRef>(*P, 2), -4);
  r1->setSymInfo(symbol1->resolveInfo());

  // r2: jmp *GOTPLT+16(%rip) - PC-relative relocation at offset 8
  r2 = Relocation::Create(llvm::ELF::R_X86_64_GOTPCREL, 32,
                          make<FragmentRef>(*P, 8), -4);
  r2->setSymInfo(symbol2->resolveInfo());

  O->addRelocation(r1);
  O->addRelocation(r2);

  return P;
}

// PLTN
x86_64PLTN *x86_64PLTN::Create(eld::IRBuilder &I, x86_64GOT *G, ELFSection *O,
                               ResolveInfo *R, bool BindNow) {
  x86_64PLTN *P = make<x86_64PLTN>(G, I, O, R, 16, 16);
  O->addFragmentAndUpdateSize(P);

  // Create relocations for GOT access and PLT0 jump
  Relocation *r1 = nullptr;
  Relocation *r2 = nullptr;
  
  // Create symbol for GOT.PLT entry for this function
  std::string name = "__gotpltn_for_" + std::string(R->name());
  LDSymbol *symbol = I.addSymbol<IRBuilder::Force, IRBuilder::Resolve>(
      O->getInputFile(), name, ResolveInfo::NoType, ResolveInfo::Define,
      ResolveInfo::Local,
      8, // size
      0, // value
      make<FragmentRef>(*G, 0), ResolveInfo::Internal,
      true /* isPostLTOPhase */);
  symbol->setShouldIgnore(false);

  // r1: jmpq *got(%rip) - PC-relative relocation at offset 2
  r1 = Relocation::Create(llvm::ELF::R_X86_64_GOTPCREL, 32,
                          make<FragmentRef>(*P, 2), -4);
  r1->setSymInfo(symbol->resolveInfo());
  O->addRelocation(r1);

  // No PLT0 for immediate binding.
  if (BindNow)
    return P;

  // r2: jmpq plt[0] - PC-relative relocation at offset 12 to PLT0
  Fragment *PLT0Fragment = *(O->getFragmentList().begin());
  std::string plt0_name = "__plt0__";
  LDSymbol *plt0_symbol = I.addSymbol<IRBuilder::Force, IRBuilder::Resolve>(
      O->getInputFile(), plt0_name, ResolveInfo::NoType, ResolveInfo::Define,
      ResolveInfo::Local,
      16, // size
      0, // value
      make<FragmentRef>(*PLT0Fragment, 0), ResolveInfo::Internal,
      true /* isPostLTOPhase */);
  plt0_symbol->setShouldIgnore(false);

  r2 = Relocation::Create(llvm::ELF::R_X86_64_PLT32, 32,
                          make<FragmentRef>(*P, 12), -4);
  r2->setSymInfo(plt0_symbol->resolveInfo());
  O->addRelocation(r2);
  return P;
}

// Implementation of getContent() for x86_64PLTN to patch relocation index
llvm::ArrayRef<uint8_t> x86_64PLTN::getContent() const {
  // If content is already patched, return it
  if (!m_PatchedContent.empty()) {
    return m_PatchedContent;
  }
  
  // Copy the base template
  m_PatchedContent.assign(x86_64_plt1, x86_64_plt1 + sizeof(x86_64_plt1));
  
  // Calculate the relocation index by counting PLTN entries before this one
  // The relocation index corresponds to the order in .rela.plt section
  uint32_t relocation_index = 0;
  
  // Get the PLT section containing this fragment
  ELFSection *plt_section = getOwningSection();
  if (plt_section) {
    // Iterate through all fragments in the PLT section
    for (auto &frag : plt_section->getFragmentList()) {
      // Check if this is the current fragment
      if (&frag == this) {
        // Found ourselves, relocation_index is correct
        break;
      }
      // Count only PLTN entries (PLT0 has no relocation entry)
      if (frag.getKind() == Fragment::Plt) {
        const PLT *plt_frag = static_cast<const PLT*>(&frag);
        if (plt_frag->getType() == PLT::PLTN) {
          relocation_index++;
        }
      }
    }
  }
  
  // Patch the relocation index into the pushq instruction at offset 7
  // pushq instruction: 0x68 followed by 32-bit immediate (little-endian)
  m_PatchedContent[7] = static_cast<uint8_t>(relocation_index & 0xFF);
  m_PatchedContent[8] = static_cast<uint8_t>((relocation_index >> 8) & 0xFF);
  m_PatchedContent[9] = static_cast<uint8_t>((relocation_index >> 16) & 0xFF);
  m_PatchedContent[10] = static_cast<uint8_t>((relocation_index >> 24) & 0xFF);
  
  return m_PatchedContent;
}
