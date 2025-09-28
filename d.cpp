// In lib/Target/x86_64/x86_64LDBackend.cpp
bool x86_64LDBackend::finalizeScanRelocations() {
  Fragment *frag = nullptr;
  if (auto *GOTPLT = getGOTPLT())
    if (GOTPLT->hasSectionData())
      frag = *GOTPLT->getFragmentList().begin();
      
  if (frag) {
    // Create relocation for GOTPLT0[0] to point to .dynamic section
    ELFSection *dynamicSection = m_Module.getSection(".dynamic");
    if (dynamicSection) {
      // Get or create the section symbol for .dynamic
      ResolveInfo *dynamicSectionSymbol = m_Module.getSectionSymbol(dynamicSection);
      
      if (!dynamicSectionSymbol) {
        // Section symbol doesn't exist yet, create it
        InputFile *I = m_Module.getInternalInput(Module::InternalInputType::Sections);
        dynamicSectionSymbol = m_Module.getNamePool().createSymbol(
            I, ".dynamic", false, ResolveInfo::Section, ResolveInfo::Define,
            ResolveInfo::Local, 0x0, ResolveInfo::Default, true);
        
        LDSymbol *sym = make<LDSymbol>(dynamicSectionSymbol, false);
        dynamicSectionSymbol->setOutSymbol(sym);
        
        // Section symbol will be properly set up later when the section has fragments
        m_Module.recordSectionSymbol(dynamicSection, dynamicSectionSymbol);
      }
      
      // Create relocation to patch GOTPLT0[0] with .dynamic address
      Relocation *r1 = Relocation::Create(llvm::ELF::R_X86_64_64, 64,
                                          make<FragmentRef>(*frag, 0), 0);
      r1->setSymInfo(dynamicSectionSymbol);
      getGOTPLT()->addRelocation(r1);
    }
    
    defineGOTSymbol(*frag);
  }
  
  return true;
}
