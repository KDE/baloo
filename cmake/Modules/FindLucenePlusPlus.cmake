# - Try to find Lucene++
# Once done, this will define
#
#  LucenePlusPlus_FOUND - system has Lucene++
#  LucenePlusPlus_INCLUDE_DIRS - the Lucene++ include directories
#  LucenePlusPlus_LIBRARIES - link these to use Lucene++
include(LibFindMacros)

# Use pkg-config to get hints about paths
#libfind_pkg_check_modules(LucenePlusPlus_PKGCONF lucene++)

find_path(LucenePlusPlus_INCLUDE_DIR
  NAMES lucene++/LuceneHeaders.h
  PATHS ${LPP_ROOT}/include ${LucenePlusPlus_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(LucenePlusPlus_LIBRARY
  NAMES lucene++
  PATHS ${LPP_ROOT}/lib ${LucenePlusPlus_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(LucenePlusPlus_PROCESS_INCLUDES LucenePlusPlus_INCLUDE_DIR LucenePlusPlus_INCLUDE_DIRS)
set(LucenePlusPlus_PROCESS_LIBS LucenePlusPlus_LIBRARY LucenePlusPlus_LIBRARIES)

libfind_process(LucenePlusPlus)