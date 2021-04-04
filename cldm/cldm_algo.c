#include "cldm_algo.h"
#include "cldm_log.h"

#include <stdlib.h>
#include <string.h>

bool cldm_is_prime(size_t num) {
    if(num == 2 || num == 3) {
        return true;
    }
    if((!num % 2) || !(num % 3)) {
        return false;
    }

    for(size_t div = 6u; div * div - 2 * div < num; div += 6u) {
        if(!(num % (div - 1)) || !(num % div + 1)) {
            return false;
        }
    }

    return true;
}

ssize_t cldm_stable_partition(void *data, size_t elemsize, size_t nelems, bool(*predicate)(void const *)) {
    unsigned char *arr;
    unsigned char *elem;
    unsigned st = 0u;
    unsigned ust = 0u;

    arr = malloc(2 * nelems * elemsize);

    if(!arr) {
        cldm_err("Could not allocate memory for partitioning");
        return -1;
    }

    for(unsigned i = 0; i < nelems; i++) {
        elem = (unsigned char *)data + i * elemsize;
        if(predicate(elem)) {
            memcpy(arr + elemsize * st++, elem, elemsize);
        }
        else {
            memcpy(arr + (nelems + ust++) * elemsize, elem, elemsize);
        }
    }

    for(unsigned i = 0; i < st; i++) {
        memcpy((unsigned char *)data + i * elemsize, arr + i * elemsize, elemsize);
    }

    for(unsigned d = st, i = 0; i < ust; i++, d++) {
        memcpy((unsigned char *)data + d * elemsize, arr + (i + nelems) * elemsize, elemsize);
    }

    free(arr);

    return st;
}
