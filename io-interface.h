#ifndef OOK_IO_INTERFACE_H
#define OOK_IO_INTERFACE_H

#ifdef _MSC_VER
# include <stdint.h>
#else
# include <inttypes.h>
#endif
#include <sys/types.h>

/** IO Interfaces used in Ook.  These are abstractions over an idea such as
 * reading or writing data.
 * @note These interfaces are byte-oriented.  Ook will internally handle all the
 * conversions to ensure the number of components and field width is taken
 * care of appropriately. */

/** The opener interface provides 'open' semantics.
 * It opens the given file in the given mode.  The identifier returned is
 * opaque; it will be given to 'read' interface calls verbatim, and the read
 * implementation knows/assumes what 'open' returned.
 * @return NULL on failure, identifier on success. */
/**@{*/
enum OOKMODE { OOK_RDONLY, OOK_RDWR };
typedef void* (opener)(const char* fn, const enum OOKMODE, const void*);
/**@}*/

/** The reader interface wraps basic 'read' operations.
 * It needs a 'descriptor', offset and length.  It reads into the given buffer.
 * @param fd is the descriptor returned from the 'open' interface.
 * @param offset *byte* offset into the stream.
 * @param len number of *bytes* to read.
 * @note ook's reader interface is defined to be atomic: it cannot return a
 * partial read (implementations should instead return an error).
 * @returns 0 on success, an error code on error. */
typedef int (reader)(void* fd, const off_t offset, const size_t len,
                     void* buf);

/** The writer interface wraps basic 'write' operations.
 * It needs a 'descriptor', an offset, and length.  It writes from the given
 * buffer.
 * @param fd a descriptor returned from the 'open' interface.
 * @note ook's writer interface is atomic: partial writes are not possible.  if
 *       only a portion of the data are successfully written, an error is
 *       returned (with no way to identify which portion).
 * @returns 0 on success, an error code on error. */
typedef int (writer)(void* fd, const off_t offset, const size_t len,
                     const void* buf);

/** The closer interface simply closes something opened by an opener.
 * @returns 0 on success, an error code on error. */
typedef int (closer)(void* fd);

/** A prealloc function preallocates the given amount of space in the file.
 * This is useful for keeping file data contiguous, even when writes are
 * scattered.
 * @note This function is optional; set it to NULL if your interface cannot
 *       support it. */
typedef void (prealloc)(void*, off_t len);

extern reader* stdc_reader;

struct io {
  opener* open;
  reader* read;
  writer* write;
  closer* close;
  prealloc* preallocate;
  const void* state;
};
extern struct io StdCIO;

#endif
