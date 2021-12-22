prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${KDE_INSTALL_BINDIR}
libdir=${KDE_INSTALL_LIBDIR}
includedir=${KDE_INSTALL_INCLUDEDIR_KF}

Name: Baloo
Description: Baloo is a file indexing and searching framework for Linux
URL: https://www.kde.org
Requires: Qt${QT_MAJOR_VERSION}Core
Version: ${BALOO_VERSION}
Libs: -L${CMAKE_INSTALL_PREFIX}/${KDE_INSTALL_LIBDIR} -lKF5Baloo
Cflags: -I${CMAKE_INSTALL_PREFIX}/${KDE_INSTALL_INCLUDEDIR_KF}/Baloo
