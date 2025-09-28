bool x86_64LDBackend::finalizeScanRelocations() {
  Fragment *frag = nullptr;
  if (auto *GOTPLT = getGOTPLT())
    if (GOTPLT->hasSectionData())
      frag = *GOTPLT->getFragmentList().begin();
      
  if (frag) {
    // Create relocation to patch GOTPLT0 with .dynamic address
    ELFSection *dynamicSection = m_Module.getSection(".dynamic");
    if (dynamicSection && dynamicSection->getFragmentList().size()) {
      // Create a local symbol for .dynamic
      std::string symName = "__dynamic_section__";
      LDSymbol *dynSym = m_Module.getIRBuilder()
          ->addSymbol<IRBuilder::Force, IRBuilder::Resolve>(
              m_Module.getInternalInput(Module::Script), symName,
              ResolveInfo::NoType, ResolveInfo::Define,
              ResolveInfo::Local, 0, 0,
              make<FragmentRef>(*dynamicSection->getFragmentList().begin(), 0),
              ResolveInfo::Hidden);
              
      if (dynSym) {
        dynSym->setShouldIgnore(false);
        
        // Create relocation for GOTPLT0[0]
        Relocation *r = Relocation::Create(llvm::ELF::R_X86_64_64, 64,
                                          make<FragmentRef>(*frag, 0), 0);
        r->setSymInfo(dynSym->resolveInfo());
        getGOTPLT()->addRelocation(r);
      }
    }
    
    defineGOTSymbol(*frag);
  }
  
  return true;
}
