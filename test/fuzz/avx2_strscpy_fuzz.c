#include "avx2_strscpy_fuzz.h"

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum { VECSIZE = 32 };
enum { STRINGSIZE = 8192 };

extern long long cldm_avx2_strscpy(char *restrict dst, char const *restrict src, size_t dstsize);

static size_t compute_alignment(void const *adstr) {
    size_t boundary = 1ull << (sizeof(size_t) * CHAR_BIT - 1);
    while(((size_t)adstr % boundary)) {
        boundary >>= 1u;
    }

    return boundary;
}

static inline int rand_in_range(int low, int high) {
    return low + (double)rand() / RAND_MAX * (high - low);
}

static char ascii_cvt(uint8_t src) {
    if(src > '~' || src < ' ') {
        return (char)(((unsigned char)src % ('~' - ' ')) + ' ');
    }
    return (char)src;
}

static void report_error(char const *restrict type, char const *restrict src, char const *restrict dst, size_t srcsize, size_t dstsize) {
    fprintf(stderr, "Error encountered: %s\n", type);
    fprintf(stderr, "src: %s\n", src);
    fprintf(stderr, "  size: %zu\n", srcsize);
    fprintf(stderr, "  length: %zu\n", strlen(src));
    fprintf(stderr, "  address: %p\n", (void const *)src);
    fprintf(stderr, "  alignment: %zu\n", compute_alignment(src));
    fprintf(stderr, "dst: %s\n", dst);
    fprintf(stderr, "  size: %zu\n", dstsize);
    fprintf(stderr, "  length %zu\n", strlen(dst));
    fprintf(stderr, "  address: %p\n", (void const *)dst);
    fprintf(stderr, "  alignment: %zu\n", compute_alignment(dst));
}

int avx2_strscpy_fuzz(uint8_t const *data, size_t size) {
    size_t dstsize;
    long long res;
    bool crash;
    char *src;
    char *dst;

    crash = true;

    if(size < VECSIZE + 1) {
        return 0;
    }

    src = malloc(STRINGSIZE);
    if(!src) {
        return 0;
    }

    dst = malloc(STRINGSIZE);
    if(!dst) {
        goto epilogue;
    }
    size = size > STRINGSIZE ? STRINGSIZE : size;

    srand(time(0));

    for(unsigned i = 0; i < size; i++) {
        src[i] = ascii_cvt(data[i]);
    }
    src[size - 1] = 0;

    dstsize = rand_in_range(0, STRINGSIZE);

    for(unsigned i = 0; i < VECSIZE; i++) {
        res = cldm_avx2_strscpy(dst, &src[i], dstsize);
        if(res == -7) {
            if(size - i <= dstsize) {
                report_error("erroneous E2BIG", &src[i], dst, size - i, dstsize);
                goto epilogue;
            }
            else if(dstsize && dst[dstsize - 1]) {
                report_error("missing null terminator on E2BIG", &src[i], dst, size - i, dstsize);
                goto epilogue;
            }
        }
        else if(res != (long long)strlen(&src[i])) {
            report_error("incorrect return value", &src[i], dst, size - i, dstsize);
            goto epilogue;
        }
        else if(strncmp(dst, &src[i], res)) {
            report_error("incorrect string written", &src[i], dst, size - i, dstsize);
            goto epilogue;
        }
        else if(dst[res]) {
            report_error("missing null terminator", &src[i], dst, size - i, dstsize);
            goto epilogue;
        }
    }

    crash = false;

epilogue:
    if(src) {
        free(src);
    }
    if(dst) {
        free(dst);
    }

    if(crash) {
        abort();
    }

    return 0;
}
