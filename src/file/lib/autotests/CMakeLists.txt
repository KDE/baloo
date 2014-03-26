#
# Fetch Job
#
set(filefetchjobtest_SRC filefetchjobtest.cpp
                         db.cpp
                         ../filefetchjob.cpp
                         ../filemapping.cpp
                         ../file.cpp
                         ../xattrdetector.cpp
                         ../filecustommetadata.cpp)

kde4_add_unit_test(fetchjobtest NOGUI ${filefetchjobtest_SRC})

target_link_libraries(fetchjobtest
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_SOLID_LIBS}
  ${QT_QTSQL_LIBRARY}
  ${XAPIAN_LIBRARIES}
  ${QJSON_LIBRARIES}
  baloocore
)

#
# Modify Job
#
set(filemodifyjobtest_SRC filemodifyjobtest.cpp
                          db.cpp
                          ../filemodifyjob.cpp
                          ../filemapping.cpp
                          ../file.cpp
                          ../xattrdetector.cpp
                          ../filecustommetadata.cpp)

kde4_add_unit_test(modifyjobtest NOGUI ${filemodifyjobtest_SRC})

target_link_libraries(modifyjobtest
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_SOLID_LIBS}
  ${QT_QTSQL_LIBRARY}
  ${XAPIAN_LIBRARIES}
  ${QJSON_LIBRARIES}
  baloocore
  balooxapian
)

# Pass CMAKE_CURRENT_BINARY_DIR so tests can use it to create some of the
# temporary files they need (as XattrDetector blacklists /tmp by default).
set_target_properties(fetchjobtest PROPERTIES COMPILE_FLAGS -DBUILDDIR="\\"${CMAKE_CURRENT_BINARY_DIR}/\\"")
set_target_properties(modifyjobtest PROPERTIES COMPILE_FLAGS -DBUILDDIR="\\"${CMAKE_CURRENT_BINARY_DIR}/\\"")
