Introduction
============

This is ook, a simple library for providing bricked versions of a dataset.

`Bricking' is the term we use in some visualization subfields to refer
to the process by which a large data set is carved into smaller pieces.
The major reason one does this is for memory usage reasons.  Typically,
a large data set exceeds the amount of physical memory available on
a machine.  One can load up a brick of the data, perform any needed
processing, and then throw the brick away.  By iterating over this
process, one can process the entire data set while only needing enough
memory for a single brick.

Ook essentially provides the illusion that your data set is already
stored as bricks.

What Ook is Not
---------------

Ook *only* provides a contiguous, bricked view of a data set.  This
means it may lack some features you desire.  You'll have to provide
them yourself.

Here are some related ideas which are out of scope for Ook:

 - multiresolution.
 - file format abstraction.  Ook does not know about file formats.
 - memory handling.  Ook provides the tool; wield it how you will.
 - metadata handling
 - high-dimensional data.  Ook deals with 3D data only, though you can
   hack lower dimensions with a ``1``-sized dimension.

Usage
=====

Ook is exported mostly as a set of function calls.  You'll need to
``#include`` the header ``ook.h`` for the functions to be available.
All Ook functions begin with ``ook``, are always spelled with lowercase
characters, and never contain underscores.

The important ones functions are:

    bool ookinit(struct io);

``ookinit`` is an initialization function for the library.  There is no
corresponding deinitialization function; just make sure all your files
are closed.  The argument tells Ook which IO interface it should use.
For simple raw data, you can pass it ``StdCIO``.

    struct ookfile;

``ookfile`` is an opaque type which represents a file opened by Ook.
Note that you may have a file open in your own code and opened a second
time through Ook.  With a "header + data" type of file format, you
might do this just to read the header and then open it again via Ook
for accessing the data.

There is a limit to the number of open ``struct ookfile``s a process
may have at any one time.  This limit is system-defined, but you may
safely assume it to be no less than 100.

    enum OOKTYPE;

``OOKTYPE`` is an enumeration which describes the underlying type of
the data.  The options are:

    OOK_I8,OOK_U8, OOK_I16,OOK_U16, OOK_I32,OOK_U32, OOK_I64,OOK_U64,
    OOK_FLOAT, OOK_DOUBLE

The data type of a file cannot be changed once it is opened.

    struct ookfile* ookread(const char*, const uint64_t voxels[3],
                            const size_t bricksize[3], const enum OOKTYPE,
                            const size_t components);

``ookread`` gives back a file used in basically all other Ook function
calls.  It only allows read access to a file.

In order, the arguments are: a filename to open, the full dimensions of
the file, the desired brick size, the underlying type of the data, and
the number of components in the data.

    size_t ookbricks(const struct ookfile*);

``ookbricks`` returns the total number of bricks in the file.  You
might use it in code like this, for example:

    for(size_t i=0; i < ookbricks(f); ++i) {
      /* process brick `i` */
    }

You could technically calculate this yourself based on the volume and
brick size you gave Ook when you opened the file.

    void ookmaxbricksize(const struct ookfile*, size_t[3]);

``ookmaxbricksize`` simply returns the brick size you gave Ook when
you opened the file.  This is just for convenience; the intent is that
you can pass the ``struct ookfile`` around all over the place without
always duplicating its metadata.

    void ookbricksize(struct ookfile*, const size_t id, size_t bsize[3]);

``ookbricksize`` gives the brick size for a specific brick (``id``)
in the data set.  This can be smaller than what is given by
``ookmaxbricksize`` in the case that the bricking size does not evenly
divide the domain.  In that case, bricks on the edge of the domain may
be smaller than other bricks.

    void ookbrick(struct ookfile*, size_t id, void* data);

``ookbrick`` is probably the most important function of the library.
It reads the data for a given brick into the provided pointer.  No
allocation is performed; Ook assumes ``data`` has enough memory to
perform the copy.

If you get segfaults in ``ookbrick``, the most likely explanation is
that you are passing in an invalid pointer or a memory block which is
not large enough for the given brick.  Remember to calculate enough
space for multicomponent data and the width of each datum.

    void ookdimensions(const struct ookfile*, uint64_t[3]);

Like ``ookmaxbricksize``, this is a convenience function so that you
do not need to communicate file metadata all over your program.  It
returns the volume size information you gave to Ook when you opened the
file.

    struct ookfile*
    ookcreate(const char* filename, const uint64_t dims[3],
              const size_t bsize[3], enum OOKTYPE, size_t components);

``ookcreate`` is the "writing" analogy to ``ookread``.  The arguments
are the same, but this gives write access to the underlying file
instead of read access.

Note the given file is truncated!

Ook will not detect if you open the same file multiple times with
any mixture of readers and writers.  The serialization of individual
readers/writers in this situation is undefined.

    void ookwrite(struct ookfile*, const size_t id, const void*);

``ookwrite`` is the opposite of ``ookbrick``: it writes the given data
to the file at the correct positions for the brick ID.

    int ookclose(struct ookfile*);

``ookclose`` closes a file which was opened by ``ookread`` or
``ookcreate``.  Any Ook calls which accept a ``struct ookfile`` give
undefined results after an ``ookclose`` operation on the same file.

*All Ook files must be closed*.

Note that a close operation may be expensive.

A common error is to ignore the return value of ``ookclose``.  Write
errors, in particular, are reported during the close.  Code which does
not check for errors on close should be assumed to write invalid output
files.
