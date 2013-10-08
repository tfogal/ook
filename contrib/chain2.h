#ifndef OOKCONTRIB_CHAIN2_IO_H
#define OOKCONTRIB_CHAIN2_IO_H
/* Chain2 is an IO interface that chains two other IO interfaces together.
 * For every function call, the two interfaces are called in series.  However,
 * if the first fails then the second will not run. */
#include "io-interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/** create a chain of one IO interface after the other.  Caller must free the
 * returned memory. */
struct io* chain2(struct io first, struct io second);

#ifdef __cplusplus
}
#endif
#endif
