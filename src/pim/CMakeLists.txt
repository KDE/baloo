if (KDEPIM_SUPPORT_BUILD)
  set(KF5AkonadiServer_MIN_VERSION "5.0.40")
  find_package(KF5AkonadiServer ${KF5AkonadiServer_MIN_VERSION} CONFIG REQUIRED)


  set(KDEPIMLIBS_LIB_VERSION "4.70.0")

  find_package(KF5KDELibs4Support ${KF5_VERSION} CONFIG REQUIRED)

  find_package(KF5Abc ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
  find_package(KF5Mime ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
  find_package(KF5PimUtils ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
  find_package(KF5AkonadiMime ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)
  find_package(KF5CalendarCore ${KDEPIMLIBS_LIB_VERSION} CONFIG REQUIRED)


  add_subdirectory(agent)
  add_subdirectory(lib)
  add_subdirectory(akonadiplugin)
endif()
add_subdirectory(search)
