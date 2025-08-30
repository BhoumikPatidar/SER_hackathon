name: Nightly builder for testing eld against musl-test-suite for x86 

on: 
  schedule:
    - cron: '0 2 * * *'
  workflow_dispatch:
  push:
    branches: [main]
  pull_request:
    paths:
      - '.github/workflows/nightly-musl-builder.yml'

jobs:
  build-and-test:
    runs-on: ubuntu-latest

    steps:
      - name: Set up Clang 20
        uses: egor-tensin/setup-clang@v1
        with:
          version: "20"
          platform: x64

      - name: Install Dependencies
        run: |
          sudo apt update
          sudo apt install -y clang libc++-dev libc++abi-dev ccache libclang-rt-20-dev cmake ninja-build make

      - name: Checkout ELD
        uses: actions/checkout@v4

      - name: Checkout LLVM Project
        uses: actions/checkout@v4
        with:
          repository: llvm/llvm-project
          path: llvm-project
          ref: main

      - name: Setup ELD in LLVM Tree
        run: |
          cp -r . llvm-project/llvm/tools/eld/

      - name: Configure ELD Build
        run: |
          mkdir -p obj
          cd obj
          
          cmake -G Ninja \
            -DCMAKE_BUILD_TYPE=Release \
            -DLLVM_ENABLE_PROJECTS="llvm;clang;clang-tools-extra" \
            -DCMAKE_C_COMPILER=`which clang` \
            -DCMAKE_CXX_COMPILER=`which clang++` \
            -DCMAKE_CXX_FLAGS="-stdlib=libc++" \
            -DLLVM_TARGETS_TO_BUILD="X86" \
            -DLLVM_CCACHE_BUILD=ON \
            -DLLVM_CCACHE_DIR=$PWD/ccache/ \
            -DELD_TARGETS_TO_BUILD='x86_64' \
            -DLLVM_ENABLE_SPHINX=OFF \
            ../llvm-project/llvm

      - name: Build ELD
        run: |
          cd obj
          ninja -j$(nproc) ld.eld

      - name: Setup ELD Environment
        run: |          
          mkdir -p bin lib
          cp obj/bin/ld.eld bin/
          cp obj/lib/libLW.so.4 lib/
          chmod +x bin/ld.eld lib/libLW.so.4
          
          export LD_LIBRARY_PATH=$PWD/lib:$LD_LIBRARY_PATH
          echo "LD_LIBRARY_PATH=$PWD/lib:$LD_LIBRARY_PATH" >> $GITHUB_ENV

      - name: Test ELD Basic Functionality
        run: |
          cat > test.c << 'EOF'
          void _start() {
            long u = 99;
            asm (
              "movq $60, %%rax\n"
              "movq %0, %%rdi\n"
              "syscall\n"
              :
              : "r" (u)
              : "%rax", "%rdi"
            );
          }
          EOF
          
          clang -c test.c -o test.o
          ./bin/ld.eld -m elf_x86_64 -o test.out test.o
          
          set +e  
          ./test.out
          EXIT_CODE=$?
          set -e 
          
          if [ $EXIT_CODE -eq 99 ]; then
            echo "✓ ELD basic linking works"
          else
            echo "ELD basic test failed (expected 99, got $EXIT_CODE)"
            exit 1
          fi

      - name: Download and Build musl
        run: |
          git clone https://github.com/bminor/musl.git
          cd musl
          CC=clang CXX=clang++ ./configure --prefix=$PWD/../musl-install
          make -j$(nproc)
          make install

      - name: Fix musl-clang Wrapper for ELD
        run: |
          sed -i 's/exec \$(\$cc -print-prog-name=ld)/exec ld.$LD/' musl-install/bin/ld.musl-clang

      - name: Setup Complete Environment
        run: |
          export PATH="$PWD/musl-install/bin:$PWD/bin:$PATH"
          export CC="$PWD/musl-install/bin/musl-clang"
          export CFLAGS="-Wno-unused-command-line-argument"
          
          echo "PATH=$PWD/musl-install/bin:$PWD/bin:$PATH" >> $GITHUB_ENV
          echo "CC=$PWD/musl-install/bin/musl-clang" >> $GITHUB_ENV
          echo "CFLAGS=-Wno-unused-command-line-argument" >> $GITHUB_ENV

      - name: Test ELD + musl Integration
        run: |
          cat > eld_musl_test.c << 'EOF'
          #include <stdio.h>
          #include <stdlib.h>
          int main() {
              printf("Hello from ELD + musl integration!\n");
              return 77;
          }
          EOF
          
          if LD=eld $CC $CFLAGS -o eld_musl_test.out eld_musl_test.c --static; then
            set +e
            ./eld_musl_test.out
            EXIT_CODE=$?
            set -e
            
            if [ $EXIT_CODE -eq 77 ]; then
              echo "✓ ELD + musl integration works"
            else
              echo "ELD + musl integration failed (expected 77, got $EXIT_CODE)"
              exit 1
            fi
          else
            echo "ELD + musl linking failed"
            exit 1
          fi

      - name: Download and Setup libc-test
        run: |
          git clone https://repo.or.cz/libc-test.git
          cd libc-test
          cp config.mak.def config.mak
          sed -i 's/LDFLAGS += -g/LDFLAGS += -g -Wl,--image-base=0x400000 -static/' config.mak

      - name: Build and Run libc-test with ELD + musl
        run: |          
          cd libc-test
          export LD=eld
          
          if make -j$(nproc) 2>&1 | tee build.log; then
            echo "libc-test build completed"
          else
            echo "libc-test build had errors, continuing with available tests"
          fi
          
          echo "Built test programs: $(find . -name "*.exe" | wc -l)"

      - name: Check for Unexpected Test Failures
        run: |  
          chmod +x .github/scripts/check-test-results.sh
          .github/scripts/check-test-results.sh libc-test
