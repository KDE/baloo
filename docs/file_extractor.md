Design Overview
===============

The file extractor (baloo_file_extractor_worker) is a small binary that drives
the process of indexing files from disk. It can store the results in an on-disk
database, return the results as a QVariantMap immediately or both. It also has
settings useful for debugging.

It is intended to be used as a (relatively) long running process: when indexing
should start, and instance of the indexer should be created and commands sent to
it to direct the indexing process.

This provides robustness to the system since if a file being indexed causes the
indexer to crash, the worker process dies but the process directing the indexing
will still be running and able to continue indexing after restarting the worker.

A class is provided in libbaloofile called ExtractorClient which encapsulates
the entire process of launching the extractor and interacting with it. This class
should be used rather than manually starting the worker binary and sending it
commands as that mechanism may change in future.

A command line client, baloo_file_exractor, is also provided that exposes the
functionality via command line options. In part, this is offered to preserve
backwards compatibility with previous versions of Baloo, but also in part to
make it easier to test and generally use the extractor from the command line.

The design goals for the worker inclue:

* robustness
* performance
* easy of use

Protocol
========
Once started, the extractor process can be directed by commands sent to it over
stdin. This allows one to use it easily from the command line by redirecting
batch files of commands to it (e.g. "baloo_file_extactor_worker < mycommands")
as well as ensures performance when used from another program directly.

The extractor responds to commands and lets the client know of progress by
outputing messages to its stdout. Again, this makes it easy to use from either
the command line or programmatically.

The protocol is command based with one command per line. Commands are given on
stdin and responses appear on stdout.

BNF:
    <command> ::= <char><args><EOL>
    <char> ::= [a..zA..Z]
    <args> ::= <literal> | <literal> " " <args> | ""

Malformed commands result in immediate termination of the process without
storage or other output being generated before exit. Multiple commands may be
sent at a time without waiting for response.

In (to extractor on its stdin)
==============================
Boolean settings take a + for true and - for false

Command         Args            Meaning
-------         ----------      -----------------------
b               <+|->           send serialized QVariantMap of results after indexing
c               <+|->           follow config to determinine indexability
d               <+|->           store data in the database
f                               indexing is finished for now (commits data to database)
i               <path>          index the file at path
s               <path>          path to store the database files
q                               quit process
z               <+|->           enable debugging

Out (from extractor on its stdout)
==================================
Command         Args            Meaning
-------         ----------      -----------------------
b               <size><data>    size followed by binary output of results from last indexing
i               <path>          indexed the file at path
s               <path>          saved all pending file data up to <path>


Default settings
================
b-
d+
z-
s${default_data_path}/baloo/file/
