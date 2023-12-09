#compdef baloodb6

# SPDX-FileCopyrightText: 2023 Natalie Clarius <natalie.clarius@kde.org>
#
# SPDX-License-Identifier: GPL-2.0-or-later

_arguments -s \
  'list[List database contents]:regular expression pattern:_normal' \
  'devices[List devices]' \
  'clean[Remove stale database entries]:regular expression pattern:_normal' \
  '(-i --device-id)'{-i,--device-id}'[Filter by device id]:device id::_numbers' \
  '(-m --missing-only)'{-m,--missing-only}'[List only inaccessible entries]' \
  '(-u --mounted-only)'{-u,--mounted-only}'[Act only on item on mounted devices]' \
  '(-D --dry-run)'{-d,--dry-run}'[Print results of a cleaning operation, but do not change anything]' \
  '*:: :_urls'
