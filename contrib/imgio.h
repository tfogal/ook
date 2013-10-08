#ifndef OOKCONTRIB_IMAGE_H
#define OOKCONTRIB_IMAGE_H
/* ImageIO is an io-interface which pretends an image is a volume. */

#include "io-interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* an array of the voxel counts in each dimension should be given as the state
 * of the ImageIO interface.  For example:
 *   const uint64_t voxels[3] = { 128, 128, 96 };
 *   struct io iio = ImageIO;
 *   iio.state = voxels;
 *   struct ookfile* of = ookread(iio, "file", voxels, ...); */
extern struct io ImageIO;

#ifdef __cplusplus
}
#endif
#endif
