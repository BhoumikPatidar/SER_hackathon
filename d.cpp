#include "x86_64ELFDynamic.h"
#include "eld/Target/GNULDBackend.h"
#include "x86_64LDBackend.h"

#include "eld/Config/LinkerConfig.h"
#include "eld/Target/ELFFileFormat.h"

using namespace eld;

x86_64ELFDynamic::x86_64ELFDynamic(GNULDBackend &pParent, LinkerConfig &pConfig)
    : ELFDynamic(pParent, pConfig) {}

x86_64ELFDynamic::~x86_64ELFDynamic() {}

void x86_64ELFDynamic::reserveTargetEntries() {
  reserveOne(llvm::ELF::DT_RELACOUNT);
  reserveOne(llvm::ELF::DT_JMPREL);
  reserveOne(llvm::ELF::DT_PLTRELSZ);
  reserveOne(llvm::ELF::DT_PLTGOT);
  reserveOne(llvm::ELF::DT_RELA);
  reserveOne(llvm::ELF::DT_RELASZ);
  reserveOne(llvm::ELF::DT_RELAENT);
}

void x86_64ELFDynamic::applyTargetEntries() {
  uint32_t relaCount = 0;
  for (auto &R : m_Backend.getRelaDyn()->getRelocations()) {
    if (R->type() == llvm::ELF::R_X86_64_RELATIVE)
      relaCount++;
  }
  applyOne(llvm::ELF::DT_RELACOUNT, relaCount);

  // Add PLT-related dynamic entries
  if (m_Backend.getRelaPLT()->size() > 0) {
    applyOne(llvm::ELF::DT_JMPREL, m_Backend.getRelaPLT()->addr());
    applyOne(llvm::ELF::DT_PLTRELSZ, m_Backend.getRelaPLT()->size());
  }

  if (m_Backend.getGOTPLT()) {
    applyOne(llvm::ELF::DT_PLTGOT, m_Backend.getGOTPLT()->addr());
  }

  // Add general relocation entries
  if (m_Backend.getRelaDyn()->size() > 0) {
    applyOne(llvm::ELF::DT_RELA, m_Backend.getRelaDyn()->addr());
    applyOne(llvm::ELF::DT_RELASZ, m_Backend.getRelaDyn()->size());
    applyOne(llvm::ELF::DT_RELAENT, m_Backend.getRelaEntrySize());
  }
}
