# Baloo

Baloo provides file search and indexing.

## File Searching

Baloo allows you to search for local files based on a query language. This language
mostly consists of (key: value) pairs which can be ANDed or ORed together to create
complex queries.

The Query interface provides a convenient API for clients to write queries which
form a tree like syntax of terms which can be ANDed and ORed together. Each term
comprises of a property name, a value and a comparison operator.

The property names which are used in the terms can either be strings or properties
from KFileMetaData::Properties.

## Baloo File Indexing

Baloo ships with a daemon `baloo_file` which is responsible for maintaining an
index of the users files. It uses the KFileMetaData framework in order to extract
metadata and text from the files.

This process utilizes inotify in order to track file changes. It performs the indexing in 2 phases.

- Phase 1 - Index the filename, fast mimetype and mtime of the file
- Phase 2 - Index the file contents. This is performed in a separate process,
as the metadata extraction plugins, provided by KFileMetaData, can potentially
crash.
