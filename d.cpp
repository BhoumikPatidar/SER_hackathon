/// finalizeSymbol - finalize the symbol value
  bool x86_64LDBackend::finalizeTargetSymbols() {
    if (config().codeGenType() == LinkerConfig::Object)
      return true;

    // Get the pointer to the real end of the image.
    if (m_pEndOfImage && !m_pEndOfImage->scriptDefined()) {
      uint64_t imageEnd = 0;
      for (auto &seg : elfSegmentTable()) {
        if (seg->type() != llvm::ELF::PT_LOAD)
          continue;
        uint64_t segSz = seg->paddr() + seg->memsz();
        if (imageEnd < segSz)
          imageEnd = segSz;
      }
      alignAddress(imageEnd, 8);
      m_pEndOfImage->setValue(imageEnd + 1);
    }

    // x86_64-specific TLS template size calculation
    ELFSegment *tlsSegment = elfSegmentTable().find(llvm::ELF::PT_TLS, llvm::ELF::PF_R, 0);
    if (tlsSegment) {
      uint64_t templateSize = tlsSegment->memsz();

      // Use segment p_align (which should match max section alignment)
      uint64_t alignment = tlsSegment->align();

      // Ensure minimum 16-byte alignment for x86_64 ABI
      if (alignment < 16)
        alignment = 16;

      // Align template size to the calculated alignment
      templateSize = llvm::alignTo(templateSize, alignment);

      setTLSTemplateSize(templateSize);
    }

    return true;
  }
