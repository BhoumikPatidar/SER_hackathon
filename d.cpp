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


Test Guide for Your Machine

  Step 1: Basic Test Setup

  # Create test directory
  mkdir dynsym_test && cd dynsym_test

  # Create simple shared library
  cat > libtest.c << 'EOF'
  int add_numbers(int a, int b) {
      return a + b;
  }
  EOF

  # Create main program
  cat > main.c << 'EOF'
  extern int add_numbers(int a, int b);
  int main() {
      return add_numbers(5, 3);
  }
  EOF

  Step 2: Test Normal Behavior

  # Create normal shared library
  gcc -shared -fPIC -o libtest.so libtest.c

  # Check if .dynsym exists
  readelf --dyn-syms libtest.so | grep add_numbers

  # Test linking with both linkers
  gcc -o main_bfd main.c -L. -ltest                    # BFD linker
  gcc -fuse-ld=lld -o main_lld main.c -L. -ltest       # LLD linker (if available)

  Step 3: Critical Test - Remove .dynsym Section

  # Copy library and try to remove .dynsym
  cp libtest.so libtest_no_dynsym.so
  objcopy --remove-section=.dynsym libtest_no_dynsym.so

  # Check if removal worked
  readelf -S libtest_no_dynsym.so | grep dynsym
  readelf --dyn-syms libtest_no_dynsym.so

  Step 4: Test Linker Behavior with Missing .dynsym

  # THIS IS THE KEY TEST - try linking against library without .dynsym
  echo "Testing BFD linker:"
  gcc -o main_no_dynsym_bfd main.c -L. -ltest_no_dynsym 2>&1

  echo "Testing LLD linker:"
  gcc -fuse-ld=lld -o main_no_dynsym_lld main.c -L. -ltest_no_dynsym 2>&1

  Step 5: Alternative Approach - Use sstrip

  # If objcopy doesn't work, try sstrip (more aggressive)
  # Install sstrip if available or use alternative methods
  cp libtest.so libtest_sstrip.so
  sstrip libtest_sstrip.so  # If available

  # Test linking
  gcc -o main_sstrip main.c -L. -ltest_sstrip 2>&1

  Expected Results:

  If linkers ERROR or WARN when .dynsym is missing:
  - Your change to return true (continue) is WRONG
  - Should return false (error) like before

  If linkers handle it gracefully (no error/warning):
  - Your change to return true (continue) is CORRECT
  - Aligns with standard linker behavior

  What to Look For:

  1. Error messages like:
    - "no dynamic symbol table"
    - "cannot read symbols"
    - "malformed shared library"
  2. Successful linking even without .dynsym means it's optional
  3. Runtime behavior - even if linking succeeds, runtime loading might fail

  Quick Alternative Test:

  If the above doesn't work, try this simpler approach:

  # Create empty shared library
  echo 'void dummy() {}' > empty.c
  gcc -shared -fPIC -o libempty.so empty.c
  objcopy --remove-section=.dynsym libempty.so

  # Try to link against it
  gcc -o test_empty -lempty -L. /dev/null 2>&1
