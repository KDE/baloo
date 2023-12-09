#compdef balooshow6

# SPDX-FileCopyrightText: 2023 Natalie Clarius <natalie.clarius@kde.org>
#
# SPDX-License-Identifier: GPL-2.0-or-later

_arguments -s \
  '(- *)'{-h,--help}'[Displays help on commandline options]' \
  '--help-all[Displays help including Qt specific options]' \
  '-x[Print internal info]' \
  '-i[Arguments are interpreted as inode numbers]' \
  '-d[Device id for the files]:device id::_numbers' \
  '*:: :_urls'
