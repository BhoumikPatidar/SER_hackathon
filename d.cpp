// R_X86_64_64: 64-bit absolute relocation (no overflow checking needed)
Relocator::Result eld::relocAbs64(Relocation &pReloc, x86_64Relocator &pParent,
                                  RelocationDescription &pRelocDesc) {
  DiagnosticEngine *DiagEngine = pParent.config().getDiagEngine();
  ResolveInfo *rsym = pReloc.symInfo();
  Relocator::Address S = pReloc.symValue(pParent.module());
  Relocator::DWord A = pReloc.addend();
  const GeneralOptions &options = pParent.config().options();

  if (rsym && rsym->isWeakUndef() &&
      (pParent.config().codeGenType() == LinkerConfig::Exec)) {
    S = 0;
  }

  if (!pReloc.targetRef()->getOutputELFSection()->isAlloc()) {
    return ApplyReloc(pReloc, S + A, pRelocDesc, DiagEngine, options);
  }

  return ApplyReloc(pReloc, S + A, pRelocDesc, DiagEngine, options);
}

// R_X86_64_32: 32-bit unsigned absolute relocation (zero-extension overflow checking)
Relocator::Result eld::relocAbs32(Relocation &pReloc, x86_64Relocator &pParent,
                                  RelocationDescription &pRelocDesc) {
  DiagnosticEngine *DiagEngine = pParent.config().getDiagEngine();
  ResolveInfo *rsym = pReloc.symInfo();
  Relocator::Address S = pReloc.symValue(pParent.module());
  Relocator::DWord A = pReloc.addend();
  const GeneralOptions &options = pParent.config().options();
  uint64_t result = S + A;

  if (rsym && rsym->isWeakUndef() &&
      (pParent.config().codeGenType() == LinkerConfig::Exec)) {
    S = 0;
    result = S + A;
  }

  // Check for 32-bit unsigned overflow (zero-extension)
  if (result > UINT32_MAX) {
    DiagEngine->raise(Diag::reloc_truncated)
        << "R_X86_64_32" << pReloc.symInfo()->name()
        << pReloc.getTargetPath(options) << pReloc.getSourcePath(options);
    return Relocator::Overflow;
  }

  if (!pReloc.targetRef()->getOutputELFSection()->isAlloc()) {
    return ApplyReloc(pReloc, result, pRelocDesc, DiagEngine, options);
  }

  return ApplyReloc(pReloc, result, pRelocDesc, DiagEngine, options);
}

// R_X86_64_32S: 32-bit signed absolute relocation (sign-extension overflow checking)
Relocator::Result eld::relocAbs32S(Relocation &pReloc, x86_64Relocator &pParent,
                                   RelocationDescription &pRelocDesc) {
  DiagnosticEngine *DiagEngine = pParent.config().getDiagEngine();
  ResolveInfo *rsym = pReloc.symInfo();
  Relocator::Address S = pReloc.symValue(pParent.module());
  Relocator::DWord A = pReloc.addend();
  const GeneralOptions &options = pParent.config().options();
  int64_t result = static_cast<int64_t>(S + A);

  if (rsym && rsym->isWeakUndef() &&
      (pParent.config().codeGenType() == LinkerConfig::Exec)) {
    S = 0;
    result = static_cast<int64_t>(S + A);
  }

  // Check for 32-bit signed overflow (sign-extension)
  if (result < INT32_MIN || result > INT32_MAX) {
    DiagEngine->raise(Diag::reloc_truncated)
        << "R_X86_64_32S" << pReloc.symInfo()->name()
        << pReloc.getTargetPath(options) << pReloc.getSourcePath(options);
    return Relocator::Overflow;
  }

  if (!pReloc.targetRef()->getOutputELFSection()->isAlloc()) {
    return ApplyReloc(pReloc, static_cast<uint64_t>(result), pRelocDesc, DiagEngine, options);
  }

  return ApplyReloc(pReloc, static_cast<uint64_t>(result), pRelocDesc, DiagEngine, options);
}

// R_X86_64_16: 16-bit absolute relocation
Relocator::Result eld::relocAbs16(Relocation &pReloc, x86_64Relocator &pParent,
                                  RelocationDescription &pRelocDesc) {
  DiagnosticEngine *DiagEngine = pParent.config().getDiagEngine();
  ResolveInfo *rsym = pReloc.symInfo();
  Relocator::Address S = pReloc.symValue(pParent.module());
  Relocator::DWord A = pReloc.addend();
  const GeneralOptions &options = pParent.config().options();
  uint64_t result = S + A;

  if (rsym && rsym->isWeakUndef() &&
      (pParent.config().codeGenType() == LinkerConfig::Exec)) {
    S = 0;
    result = S + A;
  }

  // Check for 16-bit unsigned overflow
  if (result > UINT16_MAX) {
    DiagEngine->raise(Diag::reloc_truncated)
        << "R_X86_64_16" << pReloc.symInfo()->name()
        << pReloc.getTargetPath(options) << pReloc.getSourcePath(options);
    return Relocator::Overflow;
  }

  if (!pReloc.targetRef()->getOutputELFSection()->isAlloc()) {
    return ApplyReloc(pReloc, result, pRelocDesc, DiagEngine, options);
  }

  return ApplyReloc(pReloc, result, pRelocDesc, DiagEngine, options);
}

// R_X86_64_8: 8-bit absolute relocation
Relocator::Result eld::relocAbs8(Relocation &pReloc, x86_64Relocator &pParent,
                                 RelocationDescription &pRelocDesc) {
  DiagnosticEngine *DiagEngine = pParent.config().getDiagEngine();
  ResolveInfo *rsym = pReloc.symInfo();
  Relocator::Address S = pReloc.symValue(pParent.module());
  Relocator::DWord A = pReloc.addend();
  const GeneralOptions &options = pParent.config().options();
  uint64_t result = S + A;

  if (rsym && rsym->isWeakUndef() &&
      (pParent.config().codeGenType() == LinkerConfig::Exec)) {
    S = 0;
    result = S + A;
  }

  // Check for 8-bit unsigned overflow
  if (result > UINT8_MAX) {
    DiagEngine->raise(Diag::reloc_truncated)
        << "R_X86_64_8" << pReloc.symInfo()->name()
        << pReloc.getTargetPath(options) << pReloc.getSourcePath(options);
    return Relocator::Overflow;
  }

  if (!pReloc.targetRef()->getOutputELFSection()->isAlloc()) {
    return ApplyReloc(pReloc, result, pRelocDesc, DiagEngine, options);
  }

  return ApplyReloc(pReloc, result, pRelocDesc, DiagEngine, options);
}
