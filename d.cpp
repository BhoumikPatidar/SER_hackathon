// In lib/Target/x86_64/x86_64GOT.cpp
#include "x86_64GOT.h"
#include "eld/Core/Module.h"
#include "eld/Readers/ELFSection.h"
#include "eld/Readers/Relocation.h"
#include "eld/SymbolResolver/IRBuilder.h"

using namespace eld;

x86_64GOTPLT0 *x86_64GOTPLT0::Create(IRBuilder &I, ELFSection *O) {
  x86_64GOTPLT0 *G = make<x86_64GOTPLT0>(O, nullptr);
  
  // Get the module from IRBuilder
  Module &module = I.getModule();
  
  // Get the .dynamic section
  ELFSection *dynamicSection = module.getSection(".dynamic");
  if (dynamicSection && dynamicSection->hasSectionData()) {
    // Get first fragment - dereference iterator to get Fragment*
    Fragment *firstFrag = *(dynamicSection->getFragmentList().begin());
    
    // Create a local symbol pointing to .dynamic section
    std::string symName = "__dynamic_for_gotplt0__";
    LDSymbol *symbol = I.addSymbol<IRBuilder::Force, IRBuilder::Resolve>(
        O->getInputFile(), symName, ResolveInfo::NoType, ResolveInfo::Define,
        ResolveInfo::Local, 8, 0, 
        make<FragmentRef>(*firstFrag, 0),  // Dereference Fragment* to Fragment&
        ResolveInfo::Default, true);
    symbol->setShouldIgnore(false);
    
    // Create relocation to patch GOTPLT0[0] with .dynamic address
    Relocation *r1 = Relocation::Create(llvm::ELF::R_X86_64_64, 64, 
                                        make<FragmentRef>(*G, 0), 0);
    r1->setSymInfo(symbol->resolveInfo());
    O->addRelocation(r1);
  }
  
  return G;
}
