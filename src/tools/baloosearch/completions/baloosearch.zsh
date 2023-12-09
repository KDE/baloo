#compdef baloosearch

# SPDX-FileCopyrightText: 2023 Natalie Clarius <natalie.clarius@kde.org>
#
# SPDX-License-Identifier: GPL-2.0-or-later

local ret=1

_arguments -C \
  '(- *)'{-v,--version}'[Displays version information]' \
  '(- *)'{-h,--help}'[Displays help on commandline options]' \
  '--help-all[Displays help including Qt specific options]' \
  '(-l --limit)'{-l,--limit}'[The maximum number of results]' \
  '(-o --offset)'{-o,--ofset}'[Offset from which to start the search]' \
  '(-t --type)'{-t,--type]'[Type of data to be searched]' \
  '(-d --directory)'{-d,--directory}'[Limit search to specified directory]' \
  '(-i --id)'{-i,--id}'[Show document IDs]' \
  '(-s --sort)'{-s,--sort}'[Sorting criteria]:criteria:(auto|time|none)' \
  '*:::' \
  && ret=0

return $ret
