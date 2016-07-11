![IonDB](https://cdn.rawgit.com/iondbproject/iondb/development/documentation/doxygen/iondb_logo_final.svg)
=========

**Development:** [![Build Status](http://ci.iondb.org/buildStatus/icon?job=iondb-pc-full)](http://162.243.92.39/job/iondb-pc-full/)
**Embedded Device:** [![Build Status](http://ci.iondb.org/buildStatus/icon?job=iondb-device)](http://162.243.92.39/job/iondb-device/)

You might also be interested in our sister project, [LittleD - A relational database using 1kB of RAM or less](https://github.com/graemedouglas/LittleD).

# "What is this?"

Currently in the Arduino world, there doesn't exist an associative array or map implementation that is both easy to use *and* performance competitive. There also is little support for disk based storage options that don't involve writing it yourself. IonDB is fast, functional, and offers disk based storage out of the box.

In general, IonDB supports:

* Storing arbitrary values associated to a key
* Duplicate key support
* Range and Equality queries
* Disk based persistent data storage

IonDB has a paper that was published at IEEE (CCECE) 2015, which can be found [here.](http://ieeexplore.ieee.org/xpl/articleDetails.jsp?reload=true&tp=&arnumber=7129178)

# Code Examples

Check out [this example file](src/examples/iondb_example.ino) for code examples on how to use IonDB.

# Further Reading

IonDB's Homepage: [link](https://iondb.org)

User focused information pages can be found on our [wiki](https://github.com/iondbproject/iondb/wiki).

For extensive information on IonDB, refer to our [documentation pages](https://iondb.org/) for an API level reference.

# License

IonDB is licensed under the Apache License. For more information, please refer to [the license file](LICENSE.md).

# Collaboration

Pull requests are very much welcome. Note that IonDB has a strict integration process that must fully pass before changes will be merged. Please see our page on [contributing](https://github.com/iondbproject/iondb/wiki/Contributing-and-Development-on-IonDB) for more details.

