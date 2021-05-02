prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${KDE_INSTALL_BINDIR}
libdir=${KDE_INSTALL_LIBDIR}
includedir=${KDE_INSTALL_INCLUDEDIR_KF5}

Name: Baloo
Description: Baloo is a file indexing and searching framework for Linux
URL: https://www.kde.org
Requires: Qt5Core
Version: ${BALOO_VERSION}
Libs: -L${CMAKE_INSTALL_PREFIX}/${KDE_INSTALL_LIBDIR} -lKF5Baloo
Cflags: -I${CMAKE_INSTALL_PREFIX}/${KDE_INSTALL_INCLUDEDIR_KF5}/Baloo
