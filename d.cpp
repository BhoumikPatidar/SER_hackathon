// Test for GOTPCREL relocation in static linking
// This test verifies that global variable access through GOT works correctly

int global_var = 42;

void _start() {
  // Access global variable through GOTPCREL relocation
  // This will generate R_X86_64_GOTPCREL relocation
  int *ptr;
  asm (
    "movq global_var@GOTPCREL(%%rip), %0\n"
    : "=r" (ptr)
    :
    : 
  );
  
  // Load the value from the GOT entry
  int value = *ptr;
  
  // Exit with the value from global_var (should be 42)
  asm (
    "movq $60, %%rax\n"
    "movq %0, %%rdi\n"
    "syscall\n"
    :
    : "r" ((long)value)
    : "%rax", "%rdi"
  );
}

#---GOTPCREL.test----------------- Executable --------------------#

BEGIN_COMMENT
# Test GOTPCREL relocation support in eld
# This test verifies that eld correctly handles R_X86_64_GOTPCREL relocations
# for global variable access through the Global Offset Table.
# The test uses a global variable accessed via GOTPCREL and verifies
# that the GOT entry is properly created and the relocation is resolved.
END_COMMENT

#START_TEST
RUN: %clang %clangopts -c %p/Inputs/gotpcrel.c -o %t.gotpcrel.o
RUN: %link %linkopts -o %t.gotpcrel.out %t.gotpcrel.o --image-base=0x400000
RUN: %t.gotpcrel.out; echo $? > %t.code
RUN: %filecheck --input-file=%t.code %s --check-prefix=EXITCODE
EXITCODE: 42
#END_TEST
