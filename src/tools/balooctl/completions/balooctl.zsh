#compdef balooctl6

# SPDX-FileCopyrightText: 2023 Natalie Clarius <natalie.clarius@kde.org>
#
# SPDX-License-Identifier: GPL-2.0-or-later

_arguments -s \
  '(-f --format)'{-f,--format}'[Output format]:format:(multiline json simple)' \
  '(- *)'{-v,--version}'[Displays version information]' \
  '(- *)'{-h,--help}'[Displays help on commandline options]' \
  '--help-all[Displays help including Qt specific options]' \
  'command[The command to execute]' \
  'status[Print the status of the indexer]' \
  'enable[Enable the file indexer]' \
  'disable[Disable the file indexer]' \
  'purge[Remove the index database]' \
  'suspend[Suspend the file indexer]' \
  'resume[Resume the file indexer]' \
  'check[Check for any unindexed files and index them]' \
  'index[Index the specified files]' \
  'clear[Forget the specified files]' \
  'config[Modify the Baloo configuration]' \
  'monitor[Monitor the file indexer]' \
  'indexSize[Display the disk space used by index]' \
  'failed[Display files which could not be indexed]' 
