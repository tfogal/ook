#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdio.h>

struct OokFile;

bool ookinit();
struct OokFile* ookread(const char*);

size_t ookbricks(const struct OokFile*);
void ookmaxbricksize(const struct OokFile*, size_t[3]);
size_t ookwidth(const struct OokFile*);
size_t ookcomponents(const struct OokFile*);
bool ooksigned(const struct OokFile*);

enum OOKTYPE { OOK_I8,OOK_U8, OOK_I16,OOK_U16, OOK_I32,OOK_U32,
               OOK_I64,OOK_U64, OOK_FLOAT, OOK_DOUBLE };

enum OOKTYPE ooktype(const struct OokFile*);

void ookbrick(struct OokFile*, size_t id, void* data);
void ookdimensions(struct OokFile*, uint64_t[3]);

struct OokFile*
ookcreate(const char* filename, const size_t bsize[3], const uint64_t dims[3],
          enum OOKTYPE, size_t components);

void ookbricksize(struct OokFile*, size_t id, size_t bsize[3]);
void ookbrickout(struct OokFile*, size_t id, void*, size_t n);

int ookclose(struct OokFile*);

#ifdef __cplusplus
}
#endif
