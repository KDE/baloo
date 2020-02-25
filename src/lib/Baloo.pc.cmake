prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${BIN_INSTALL_DIR}
libdir=${LIB_INSTALL_DIR}
includedir=${KF5_INCLUDE_INSTALL_DIR}

Name: Baloo
Description: Baloo is a file indexing and searching framework for Linux
URL: https://www.kde.org
Requires: Qt5Core
Version: ${BALOO_VERSION_STRING}
Libs: -L${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR} -lKF5Baloo
Cflags: -I${CMAKE_INSTALL_PREFIX}/${KF5_INCLUDE_INSTALL_DIR}/Baloo
