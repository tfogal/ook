Introduction
============

[![Build Status](https://travis-ci.org/tfogal/ook.png)](https://travis-ci.org/tfogal/ook)

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

More info: http://tfogal.github.io/ook/
