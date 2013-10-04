#ifndef OOKCONTRIB_STACK_IO_H
#define OOKCONTRIB_STACK_IO_H
/* Stack is an IO interface for a stack of images.  Give it a directory and it
 * will assume each of the files in the directory is a single 2D slice of a 3D
 * dataset.  The ordering of the slices is lexographic based on filename. */

#include "io-interface.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct io StackIO;

#ifdef __cplusplus
}
#endif
#endif
