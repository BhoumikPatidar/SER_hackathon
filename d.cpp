// In lib/Target/x86_64/x86_64PLT.cpp

x86_64PLT0 *x86_64PLT0::Create(eld::IRBuilder &I, x86_64GOT *G, ELFSection *O,
                               ResolveInfo *R, bool BindNow) {
  x86_64PLT0 *P = make<x86_64PLT0>(G, I, O, R, 16, 16);
  O->addFragmentAndUpdateSize(P);

  llvm::outs() << "PLT0 created at: " << P << "\n";

  std::string name = "__gotplt0__";
  
  // Check if the symbol already exists to avoid duplicate definition
  LDSymbol *existingSymbol = I.getModule().getNamePool().findSymbol(name);
  if (existingSymbol && !existingSymbol->shouldIgnore()) {
    // Symbol already exists, just use it for relocations
    LDSymbol *symbol = existingSymbol;
  } else {
    // Create the symbol only if it doesn't exist
    LDSymbol *symbol = I.addSymbol<IRBuilder::Force, IRBuilder::Resolve>(
        O->getInputFile(), name, ResolveInfo::NoType, ResolveInfo::Define,
        ResolveInfo::Local, 8, 0, make<FragmentRef>(*G, 0), ResolveInfo::Default,
        true /* isPostLTOPhase */);
    symbol->setShouldIgnore(false);
  }

  // Always create relocations regardless of symbol existence
  Relocation *r1 = Relocation::Create(llvm::ELF::R_X86_64_PC32, 32,
                                      make<FragmentRef>(*P, 2), 4);
  r1->setSymInfo(existingSymbol ? existingSymbol->resolveInfo() : symbol->resolveInfo());
  O->addRelocation(r1);

  Relocation *r2 = Relocation::Create(llvm::ELF::R_X86_64_PC32, 32,
                                      make<FragmentRef>(*P, 8), 12);
  r2->setSymInfo(existingSymbol ? existingSymbol->resolveInfo() : symbol->resolveInfo());
  O->addRelocation(r2);

  return P;
}

x86_64PLTN *x86_64PLTN::Create(eld::IRBuilder &I, x86_64GOT *G, ELFSection *O,
                               ResolveInfo *R, bool BindNow) {
  x86_64PLTN *P = make<x86_64PLTN>(G, I, O, R, 16, 16);
  O->addFragmentAndUpdateSize(P);

  if (x86_64GOTPLTN *gotplt = llvm::dyn_cast<x86_64GOTPLTN>(G)) {
    gotplt->setPLTEntry(P);
  }

  std::string name = "__gotpltn_for_" + std::string(R->name());
  LDSymbol *symbol = I.addSymbol<IRBuilder::Force, IRBuilder::Resolve>(
      O->getInputFile(), name, ResolveInfo::NoType, ResolveInfo::Define,
      ResolveInfo::Local, 8, 0, make<FragmentRef>(*G, 0), ResolveInfo::Default,
      true);
  symbol->setShouldIgnore(false);

  Relocation *r1 = Relocation::Create(llvm::ELF::R_X86_64_PC32, 32,
                                      make<FragmentRef>(*P, 2), -4);
  r1->setSymInfo(symbol->resolveInfo());
  O->addRelocation(r1);

  if (!BindNow) {
    Fragment *PLT0 = *(O->getFragmentList().begin());
    llvm::outs() << "x86_64PLTN::Create PLT0 offset " << PLT0->getOffset() << "\n";
    
    // Use a unique name for PLT0 symbol or check if it exists
    std::string plt0_name = "__plt0__";
    LDSymbol *plt0_symbol = I.getModule().getNamePool().findSymbol(plt0_name);
    
    if (!plt0_symbol || plt0_symbol->shouldIgnore()) {
      // Create the symbol only if it doesn't exist
      plt0_symbol = I.addSymbol<IRBuilder::Force, IRBuilder::Resolve>(
          O->getInputFile(), plt0_name, ResolveInfo::NoType, ResolveInfo::Define,
          ResolveInfo::Local, 16, 0, make<FragmentRef>(*PLT0, 0),
          ResolveInfo::Default, true);
      plt0_symbol->setShouldIgnore(false);
    }

    Relocation *r2 = Relocation::Create(llvm::ELF::R_X86_64_PC32, 32,
                                        make<FragmentRef>(*P, 12), -4);
    r2->setSymInfo(plt0_symbol->resolveInfo());
    O->addRelocation(r2);
  }

  return P;
}
