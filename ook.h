#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdio.h>

struct ookfile;

/* The reader interface wraps basic 'read' operations.
 * It needs an ookfile, offset and length.  ook's reader interface is defined
 * to be atomic: it never returns a partial read.
 * @returns 0 on success, an error code (l.t. 0) on error. */
typedef int (reader)(struct ookfile*, const off_t offset, const size_t len);

extern reader* stdc_reader;

bool ookinit(reader*);
struct ookfile* ookread(const char*, const uint64_t voxels[3],
                        const size_t bricksize[3]);

size_t ookbricks(const struct ookfile*);
void ookmaxbricksize(const struct ookfile*, size_t[3]);

enum OOKTYPE { OOK_I8,OOK_U8, OOK_I16,OOK_U16, OOK_I32,OOK_U32,
               OOK_I64,OOK_U64, OOK_FLOAT, OOK_DOUBLE };

void ookbrick(struct ookfile*, size_t id, void* data);
void ookdimensions(struct ookfile*, uint64_t[3]);

struct ookfile*
ookcreate(const char* filename, const uint64_t dims[3], const size_t bsize[3],
          enum OOKTYPE, size_t components);

void ookbricksize(struct ookfile*, size_t id, size_t bsize[3]);
void ookwrite(struct ookfile*, size_t id, void*, size_t n);

int ookclose(struct ookfile*);

#ifdef __cplusplus
}
#endif
