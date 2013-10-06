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
