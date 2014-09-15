Protocol basics
===============
The protocol is command based with one command per line. Commands are given on
stdin and responses appear on stdout.

BNF:
    <command> ::= <char><args><EOL>
    <char> ::= [a..zA..Z]
    <args> ::= <literal> | <literal> " " <args> | ""

Malformed commands result in immediate termination of the process without
storage or other output being generated before exit.

In (to extractor on its stdin)
==============================
Command         Args            Meaning
-------         ----------      -----------------------
b               <+|->           binary out: + == results are sent to stdout, - == no stdout
d               <+|->           + == results are saved to the db, - == no db storage
i               <path>          index the file at path
s               <path>          path to store the database at
q                               exit process
z               <+|->           + == debug on; - == debug off

Out (from extractor on its stdout)
==================================
Command         Args            Meaning
-------         ----------      -----------------------
b               <data>          binary output of results from last indexing
i               <path>          indexed the file at path
s               <path>          saved all pending file data up to <path>


Default settings
================
b-
d+
z-
s${default_data_path}/baloo/file/
