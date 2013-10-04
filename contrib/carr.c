#include <stdlib.h>

/** A simple growable C array.  Stores pointers to "something".  Does not own
 * memory. */
struct carray {
  size_t i, n; /* i is where we are now; n is how much space we have total. */
  void** data;
};

#include "carr.h"

const size_t GROWTH_FACTOR = 2;

array*
carray(size_t n)
{
  struct carray* arr = calloc(1, sizeof(struct carray));
  arr->n = n;
  arr->i = 0;
  arr->data = malloc(sizeof(void*) * n);
  return (void*)arr;
}

struct carray*
ca_append(struct carray* arr, void* element)
{
  if(arr->i == arr->n) { /* we need to grow. */
    arr->data = realloc(arr->data, sizeof(void*)*arr->n*GROWTH_FACTOR);
    if(arr->data == NULL) {
      free(arr);
      return NULL;
    }
    arr->n *= GROWTH_FACTOR;
  }
  arr->data[arr->i] = element;
  arr->i++;
  return (void*)arr;
}

size_t
ca_size(const struct carray* arr)
{
  return arr->i;
}

void*
ca_elem(const struct carray* arr, size_t idx)
{
  return arr->data[idx];
}

void
ca_destroy(struct carray* arr)
{
  free(arr->data);
  free(arr);
}
