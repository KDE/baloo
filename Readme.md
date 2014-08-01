# Baloo

Baloo provides the searching and indexing infrastructure with an emphasis on
files.

## Baloo Core Library

The core library provides a
- Query Interface
- Search Stores

### Query Interface

The Query interface provides a convenient API for clients to write queries which
form a tree like syntax of terms which can be ANDed and ORed together. Each term
comprises of a property name, a value and a comparison operator.

The property names which are used in the terms are just strings. The properties
supported depend on the Search Store which is being queried.

Queries additionally can also stored date filters, a global search string, an 
offset, a limit and a sorting mechanism for the results.

Each Query can also be converted into a byte array where the query is
represented in a simple JSON format.

### Search Store

A Search Store is a plugin which implements the parsing of a Query and returns
the results. Each search store registers a set of types, represented as strings,
whose results it can provide. The client, when constructing a query provides a
list of types. Based on the type, the correct Search Store Plugin is run.

Each Search Store can interpret the query in the manner they see fit and return
results accordingly. This is typically done by maintaining an index for faster
search, but it is not necessary.

## Baloo File

Baloo has an emphasis on file searching. It ships with a daemon `baloo_file`
which is responsible for maintaining an index of the users files. It uses the
KFileMetaData framework in order to extract metadata and text from the files.
It also ships via a searching plugin.

### File Search Store

In order to facilitate searching, Baloo provides a search store plugin which
can be used to query for files which specific properties. In the most general
case the `Query::setQueryString` should be used to do a global search over
all properties in all the files.

Supported Properties: Lookup the properties via KFileMetaData property list
Supported Types: Supports the "File" type along with further specialized types
Lookup the list of types via the KFileMetaData type list.

### File Indexing

Baloo ships with an daemon `baloo_file` which is responsible for maintaining an
index of the users files. This process utilizes inotify in order to track file
changes. It performs the indexing in 2 phases.

* Phase 1 - Index the filename, fast mimetype and mtime of the file
* Phase 2 - Index the file contents. This is performed in a separate process,
as the metadata extraction plugins, provided by KFileMetaData, can potentially
crash.

