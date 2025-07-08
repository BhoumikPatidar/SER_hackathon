//===- x86_64ELFDynamic.h-------------------------------------------------===//
// Part of the eld Project, under the BSD License
// See https://github.com/qualcomm/eld/LICENSE.txt for license information.
// SPDX-License-Identifier: BSD-3-Clause
//===----------------------------------------------------------------------===//

#ifndef TARGET_X86_64_X86_64ELFDYNAMIC_H
#define TARGET_X86_64_X86_64ELFDYNAMIC_H

#include "eld/Target/ELFDynamic.h"

namespace eld {

class x86_64ELFDynamic : public ELFDynamic {
public:
  x86_64ELFDynamic(GNULDBackend &pParent, LinkerConfig &pConfig);
  ~x86_64ELFDynamic();

private:
  void reserveTargetEntries() override;
  void applyTargetEntries() override;
};

} // namespace eld

#endif

//===- x86_64ELFDynamic.cpp-----------------------------------------------===//
// Part of the eld Project, under the BSD License
// See https://github.com/qualcomm/eld/LICENSE.txt for license information.
// SPDX-License-Identifier: BSD-3-Clause
//===----------------------------------------------------------------------===//

#include "x86_64ELFDynamic.h"
#include "x86_64LDBackend.h"
#include "eld/Config/LinkerConfig.h"
#include "eld/Target/ELFFileFormat.h"

using namespace eld;

x86_64ELFDynamic::x86_64ELFDynamic(GNULDBackend &pParent, LinkerConfig &pConfig)
    : ELFDynamic(pParent, pConfig) {}

x86_64ELFDynamic::~x86_64ELFDynamic() {}

void x86_64ELFDynamic::reserveTargetEntries() {
  // For x86_64, we don't need any target-specific dynamic entries
  // The base ELFDynamic class handles all the standard PLT/GOT entries
}

void x86_64ELFDynamic::applyTargetEntries() {
  // For x86_64, we don't need to apply any target-specific dynamic entries
  // The base ELFDynamic class handles all the standard PLT/GOT entries
}
