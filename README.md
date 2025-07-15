# check_riscv_simd
C program to check if on RISCV SIMD is available from CPU and inside program

# Compile

`gcc -march=rv64gcv -O2 -o check_riscv_simd check_riscv_simd.c`

# Run

```
$ ./check_riscv_simd
Running on RISC-V architecture.
Vector extension 'v' IS listed in ISA in /proc/cpuinfo.
Vector instructions are available at runtime.
```

