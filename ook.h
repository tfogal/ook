#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdio.h>

bool ook_init();
size_t ook_bricks(FILE*);
void ook_max_brick_size(FILE*, size_t[3]);
size_t ook_width(FILE*);
size_t ook_components(FILE*);
bool ook_signed(FILE*);

enum OOKTYPE { OOK_I8,OOK_U8, OOK_I16,OOK_U16, OOK_I32,OOK_U32,
               OOK_I64,OOK_U64, OOK_FLOAT, OOK_DOUBLE };

enum OOKTYPE ook_type(FILE*);

void ook_brick(FILE*, size_t id, void* data);
void ook_dimensions(FILE*, uint64_t[3]);

struct OokFile;
struct OokFile*
ook_new(const char* filename, const size_t bsize[3], const uint64_t dims[3],
        enum OOKTYPE, size_t components);

void ook_brick_size(FILE*, size_t id, size_t bsize[3]);
void ook_brick_out(struct OokFile*, size_t id, void*, size_t n);

#ifdef __cplusplus
}
#endif
