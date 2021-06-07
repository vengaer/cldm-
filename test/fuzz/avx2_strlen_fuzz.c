#include "avx2_strlen_fuzz.h"

#include <cldm/cldm_config.h>

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern unsigned long long cldm_avx2_strlen(char const *string);

static inline char ascii_cvt(uint8_t byte) {
    return isprint(byte) ? (char)byte : (char)(((unsigned char)byte & ('~' - ' ')) + ' ');
}

static inline size_t alignment(void const *addr) {
    return (size_t)addr & -(size_t)addr;
}

static inline size_t pgdistance(void const *addr) {
    return CLDM_PGSIZE - ((size_t)addr & (CLDM_PGSIZE - 1));
}

static void report_error(char const *str, unsigned long long reported, size_t actual) {
    fputs("Error encountered\n", stderr);
    fprintf(stderr, "String: '%s'\n", str);
    fprintf(stderr, "Reported length: %llu\n", reported);
    fprintf(stderr, "Actual length: %zu\n", actual);
    fprintf(stderr, "Address: %p\n", (void const *)str);
    fprintf(stderr, "Alignment: %zu\n", alignment(str));
    fprintf(stderr, "Distance to page boundary: %zu\n", pgdistance((void const *)str));
}

int avx2_strlen_fuzz(uint8_t const *data, size_t size) {
#ifndef CLDM_HAS_AVX2
    fputs("cldm compiled without avx2 support\n", stderr);
    abort();
#endif
    char *str;
    bool crash;
    unsigned long long reported;
    size_t actual;

    if(!size) {
        return 0;
    }

    str = malloc(size);
    if(!str) {
        fputs("malloc failure\n", stderr);
        return 0;
    }
    for(unsigned i = 0; i < size - 1; i++) {
        str[i] = ascii_cvt(data[i]);
    }
    str[size - 1] = 0;

    crash = true;
    actual = strlen(str);

    for(unsigned i = 0; i < actual; i++) {
        reported = cldm_avx2_strlen(str + i);
        if(reported != actual - i) {
            report_error(str + i, reported, actual - i);
            goto epilogue;
        }
    }

    crash = false;
epilogue:
    free(str);
    if(crash) {
        abort();
    }
    return 0;
}