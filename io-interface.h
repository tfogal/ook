#ifndef OOK_IO_INTERFACE_H
#define OOK_IO_INTERFACE_H

/* IO Interfaces used in Ook.  These are abstractions over an idea such as
 * reading or writing data. */

/** The opener interface provides 'open' semantics.
 * It opens the given file in the given mode.  The identifier returned is
 * opaque; it will be given to 'read' interface calls verbatim, and the read
 * interface assumes what 'open' did.
 * @return NULL on failure, identifier on success. */
/**@{*/
enum OOKMODE { OOK_RDONLY, OOK_RDWR };
typedef void* (opener)(const char* fn, const enum OOKMODE);
/**@}*/

/** The reader interface wraps basic 'read' operations.
 * It needs a 'descriptor', offset and length.  It reads into the given buffer.
 * @param fd is the descriptor returned from the 'open' interface.
 * @note ook's reader interface is defined to be atomic: it cannot return a
 * partial read (implementations should instead return an error).
 * @returns 0 on success, an error code (l.t. 0) on error. */
typedef int (reader)(void* fd, const off_t offset, const size_t len,
                     void* buf);

/** The closer interface simply closes something opened by an opener.
 * @returns 0 on success, an error code < 0 on error. */
typedef int (closer)(void* fd);

/** A prealloc function preallocates the given amount of space in the file.
 * This is useful for keeping file data contiguous, even when writes are
 * scattered. */
typedef void (prealloc)(void*, off_t len);

extern reader* stdc_reader;

struct io {
  opener* open;
  reader* read;
  closer* close;
  prealloc* preallocate;
};
extern struct io StdCIO;
extern struct io MappedIO;

#endif
