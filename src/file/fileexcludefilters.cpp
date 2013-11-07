/* This file is part of the KDE Project
   Copyright (c) 2008-2010 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "fileexcludefilters.h"

namespace 
{
const char* s_defaultFileExcludeFilters[] = {
    // tmp files
    "*~",
    "*.part",

    // temporary build files
    "*.o",
    "*.la",
    "*.lo",
    "*.loT",
    "*.moc",
    "moc_*.cpp",
    "cmake_install.cmake",
    "CMakeCache.txt",
    "CTestTestfile.cmake",
    "libtool",
    "config.status",
    "confdefs.h",
    "autom4te",
    "conftest",
    "confstat",
    "Makefile.am",

    // misc
    "*.csproj",
    "*.m4",
    "*.rej",
    "*.gmo",
    "*.pc",
    "*.omf",
    "*.aux",
    "*.tmp",
    "*.po",
    "*.vm*",
    "*.nvram",
    "*.rcore",
    "lzo",
    "litmain.sh",
    "*.orig",
    ".histfile.*",
    ".xsession-errors*",

    // Compiled files
    "*.class", // Java
    "*.pyc",   // Python
    "*.elc",   // Emacs Lisp

    // end of list
    0
};

const int s_defaultFileExcludeFiltersVersion = 2;

const char* s_defaultFolderExcludeFilters[] = {
    "po",

    // VCS
    "CVS",
    ".svn",
    ".git",
    "_darcs",
    ".bzr",
    ".hg",

    // development
    "CMakeFiles",
    "CMakeTmp",
    "CMakeTmpQmake",

    //misc
    "core-dumps",
    "lost+found",

    // end of list
    0
};

const int s_defaultFolderExcludeFiltersVersion = 1;
}


QStringList Nepomuk2::defaultExcludeFilterList()
{
    QStringList l;
    for (int i = 0; s_defaultFileExcludeFilters[i]; ++i)
        l << QLatin1String(s_defaultFileExcludeFilters[i]);
    for (int i = 0; s_defaultFolderExcludeFilters[i]; ++i)
        l << QLatin1String(s_defaultFolderExcludeFilters[i]);
    return l;
}

int Nepomuk2::defaultExcludeFilterListVersion()
{
    return qMax(s_defaultFileExcludeFiltersVersion, s_defaultFolderExcludeFiltersVersion);
}
