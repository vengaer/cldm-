#ifndef CLDM_ALGO_H
#define CLDM_ALGO_H

#include <stdbool.h>
#include <stddef.h>

#include <sys/types.h>

ssize_t cldm_stable_partition(void *data, size_t elemsize, size_t nelems, bool(*predicate)(void const *));

#endif /* CLDM_ALGO_H */
