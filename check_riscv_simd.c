#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>

static sigjmp_buf jmpbuf;

void sigill_handler(int signo) {
    siglongjmp(jmpbuf, 1);
}

// Check /proc/cpuinfo for 'v' extension
int check_cpuinfo_for_vector() {
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (!fp) {
        perror("fopen");
        return -1;
    }
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "ISA") || strstr(line, "isa")) {
            if (strstr(line, "v")) {
                fclose(fp);
                return 1;  // Vector extension present
            }
        }
    }
    fclose(fp);
    return 0; // No vector extension found
}

// Try to execute a vector instruction to test runtime availability
int try_vector_instruction() {
    struct sigaction sa_old, sa_new;
    sa_new.sa_handler = sigill_handler;
    sigemptyset(&sa_new.sa_mask);
    sa_new.sa_flags = 0;
    sigaction(SIGILL, &sa_new, &sa_old);

    if (sigsetjmp(jmpbuf, 1) == 0) {
        // Inline assembly to set VL to 4 elements of 8 bits each
        asm volatile(
            "li t0, 4\n\t"
            "vsetvli t1, t0, e8, m1\n\t"
            :
            :
            : "t0", "t1"
        );
        sigaction(SIGILL, &sa_old, NULL);
        return 1; // Success
    } else {
        sigaction(SIGILL, &sa_old, NULL);
        return 0; // SIGILL caught
    }
}

int main() {
#ifdef __riscv
    printf("Running on RISC-V architecture.\n");

    int cpuinfo_vector = check_cpuinfo_for_vector();
    if (cpuinfo_vector == -1) {
        printf("Could not read /proc/cpuinfo.\n");
        return 1;
    } else if (cpuinfo_vector == 0) {
        printf("Vector extension 'v' is NOT listed in ISA in /proc/cpuinfo.\n");
    } else {
        printf("Vector extension 'v' IS listed in ISA in /proc/cpuinfo.\n");
    }

    int runtime_vector = try_vector_instruction();
    if (runtime_vector) {
        printf("Vector instructions are available at runtime.\n");
    } else {
        printf("Vector instructions are NOT available at runtime (SIGILL caught).\n");
    }

#else
    printf("Not running on RISC-V. This program is for RISC-V only.\n");
#endif

    return 0;
}
