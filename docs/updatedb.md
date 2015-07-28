Baloo vs UpdateDB
-----------------

UpdateDB only searches through the filepath.

Baloo searches through -
    * MTime
    * Mimetype
    * Filename
    * File Content

Additionally, Baloo is optimized for searching and operating with very low latency
when searching. It does however consume more resources while writing and creating
the index.

