#ifndef OOKCONTRIB_CARR_H
#define OOKCONTRIB_CARR_H
/* A simple growable C array.  Does *not* own the memory you give it. */

#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct carray array;

array* carray(size_t);
array* ca_append(array*, void*);
size_t ca_size(const array*);
void* ca_elem(const array*, size_t);
void ca_destroy(array*);

#ifdef __cplusplus
}
#endif
#endif
