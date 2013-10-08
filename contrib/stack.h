#ifndef OOKCONTRIB_STACK_IO_H
#define OOKCONTRIB_STACK_IO_H
/* Stack is an IO interface for a stack of images.  Give it a directory and it
 * will assume each of the files in the directory is a single 2D slice of a 3D
 * dataset.  The ordering of the slices is lexographic based on filename. */

#include "io-interface.h"

#ifdef __cplusplus
extern "C" {
#endif

struct metadata {
  uint64_t voxels[3];
  size_t components;
  size_t width; /* in bytes */
};

/* the 'state' should be set to a pointer to metadata.
 *  struct metadata md;
 *  memcpy(md.voxels, voxels, sizeof(uint64_t)*3);
 *  md.components = components;
 *  md.width = sizeof(...);
 *  struct io mystack = StackIO;
 *  mystack.state = &md;
 *  struct ookfile* of = ookread(mystack, "file", voxels, ...); */
extern struct io StackIO;

#ifdef __cplusplus
}
#endif
#endif
