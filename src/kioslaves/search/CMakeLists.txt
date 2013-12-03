project(kioslave_nepomuksearch)

include(SopranoAddOntology)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=300200)

include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${KDE4_KIO_INCLUDES}
  ${SOPRANO_INCLUDE_DIR}
  ${NEPOMUK_CORE_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${nepomuk_kio_slaves_SOURCE_DIR}/common
)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DDISABLE_NEPOMUK_LEGACY)

set(kio_nepomuksearch_PART_SRCS
  kio_nepomuksearch.cpp
  searchfolder.cpp
)

soprano_add_ontology(kio_nepomuksearch_PART_SRCS ${NEPOMUK_CORE_ONTOLOGY_DIR}/kde/kext.trig "KExt" "Nepomuk2::Vocabulary" "trig")

kde4_add_plugin(kio_nepomuksearch
  ${kio_nepomuksearch_PART_SRCS})

target_link_libraries(kio_nepomuksearch
  ${KDE4_KIO_LIBS}
  ${KDE4_SOLID_LIBS}
  ${NEPOMUK_CORE_LIBRARY}
  ${SOPRANO_LIBRARIES}
  )

install(
  TARGETS kio_nepomuksearch
  DESTINATION ${PLUGIN_INSTALL_DIR})
install(
  FILES nepomuksearch.protocol
  DESTINATION  ${SERVICES_INSTALL_DIR})

add_subdirectory(kdedmodule)
