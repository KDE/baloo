# Baloo

## Introduction

Baloo is the file indexing and file search framework for KDE Plasma. It focuses 
on speed and a very small memory footprint. It maintains an index of your files 
and optionally their contents which [you can search](./docs/user/searching.md).

## Contributing

Baloo is part of the KDE umbrella and relies on the KDE infrastructure.

**Mailing List:** kde-devel@kde.org ([info page](https://mail.kde.org/mailman/listinfo/kde-devel))
**Bug Tracker:** http://bugs.kde.org  ([new bug](https://bugs.kde.org/enter_bug.cgi?product=frameworks-baloo))
**IRC Channel:** #kde-baloo on Libera Chat

The recommended way of contributing patches is via KDE's [GitLab](https://invent.kde.org/frameworks/baloo) instance.

## Documentation

### Users
* [Searching](./docs/user/searching.md)
* [The Baloo pages on KDE Community Wiki](https://community.kde.org/Baloo) have information on Baloo's command-line tools and how to monitor its operation.


### Developers
[![Build Status](https://build.kde.org/job/Frameworks/job/baloo/job/kf5-qt5%20SUSEQt5.15/badge/icon?subject=SUSE%20Qt5.15)](https://build.kde.org/job/Frameworks/job/baloo/job/kf5-qt5%20SUSEQt5.15/)
[![Build Status](https://build.kde.org/job/Frameworks/job/baloo/job/kf5-qt5%20FreeBSDQt5.15/badge/icon?subject=FreeBSD%20Qt5.15)](https://build.kde.org/job/Frameworks/job/baloo/job/kf5-qt5%20FreeBSDQt5.15/)
* [Build Instructions](@ref build-instructions)
* Baloo follows the [KDE Frameworks coding style](https://community.kde.org/Policies/Frameworks_Coding_Style).

### Distributions
Baloo is developed and tested exclusively for Linux. While it may run on other
unix based systems. It is not recommended, and certainly not tested.

We do not recommend to package Baloo for Windows or OSX as both these operating
systems offer their own file searching solutions which better integrate with
the native system than Baloo ever will.

Baloo may run on 32-bit systems, but it has not been tested and may not work
correctly. Please test and let us know by [filing a bug](https://bugs.kde.org/enter_bug.cgi?product=frameworks-baloo).

**Supported Kernels:** Linux
**Supported Architectures:** x86_64, aarch64
**Supported Filesystems:** ext3/4, Btrfs, XFS

###### File Indexing Plugins

Baloo relies on [KFileMetaData](https://api.kde.org/frameworks/kfilemetadata/html/index.html) to extract content from the files. KFileMetadata
ships with a number of plugins which can be enabled or disabled. We recommend
shipping all KFileMetaData plugins. Specially ffmpeg by default. Without the indexers, 
Baloo cannot function to its full potential.
