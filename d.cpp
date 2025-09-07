Target Program Details
  Our target program consists of two source files that demonstrate basic dynamic linking between an object file and a shared library:
  File 1: 1.c (Shared Library Source)
  long foo(){return 3;}
  Purpose:
  - Defines a simple function foo() that returns the value 3
  - Will be compiled into a shared library lib1.so
  - Exports the foo symbol for dynamic linking
  File 2: 4.1.c (Main Program Source)
  long foo();
  void _start(){
      long u = foo();
      asm (
          "movq $60, %%rax\n"
          "movq %0, %%rdi\n"
          "syscall\n"
          :
          : "r" (u)
          : "%rax", "%rdi"
      );
  }
  Purpose:
  - Declares foo() as an external function (undefined symbol)
  - _start() is the entry point (no standard C library)
  - Calls foo() and stores result in variable u
  - Uses inline assembly to perform exit(u) system call directly:
    - movq $60, %rax - Load system call number 60 (sys_exit)
    - movq %0, %rdi - Load return value from foo() into first argument register
    - syscall - Invoke system call to exit with the value returned by foo()
  Build Process
  # Compile source files to object files
  musl-clang -o 1.o 1.c -c -ffunction-sections -fPIC
  musl-clang -o 4.1.o 4.1.c -c -ffunction-sections
  # Create shared library from 1.o
  musl-clang -shared -o lib1.so 1.o
ld.lld -o 4.1.lld.out 4.1.o lib1.so -nostdlib -dynamic-linker=/lib64/ld-linux-x86-64.so.2
