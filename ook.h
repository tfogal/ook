#ifndef OOK_OOK_H
#define OOK_OOK_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
# include <stdint.h>
#else
# include <inttypes.h>
#endif
#include <stdbool.h>
#include <stdio.h>
#include "io-interface.h"

struct ookfile;
enum OOKTYPE { OOK_I8,OOK_U8, OOK_I16,OOK_U16, OOK_I32,OOK_U32,
               OOK_I64,OOK_U64, OOK_FLOAT, OOK_DOUBLE };

bool ookinit(struct io);
struct ookfile* ookread(const char*, const uint64_t voxels[3],
                        const size_t bricksize[3], const enum OOKTYPE,
                        const size_t components);

size_t ookbricks(const struct ookfile*);
void ookmaxbricksize(const struct ookfile*, size_t[3]);

void ookbrick(struct ookfile*, size_t id, void* data);
void ookdimensions(const struct ookfile*, uint64_t[3]);

struct ookfile*
ookcreate(const char* filename, const uint64_t dims[3], const size_t bsize[3],
          enum OOKTYPE, size_t components);

void ookbricksize(struct ookfile*, const size_t id, size_t bsize[3]);
void ookwrite(struct ookfile*, const size_t id, const void*);

int ookclose(struct ookfile*);

#ifdef __cplusplus
}
#endif
#endif
