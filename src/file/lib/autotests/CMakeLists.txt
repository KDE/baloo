#
# Fetch Job
#
set(filefetchjobtest_SRC filefetchjobtest.cpp db.cpp ../filefetchjob.cpp ../filemapping.cpp ../file.cpp)

kde4_add_unit_test(fetchjobtest NOGUI ${filefetchjobtest_SRC})

target_link_libraries(fetchjobtest
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${QT_QTSQL_LIBRARY}
  ${XAPIAN_LIBRARIES}
  ${QJSON_LIBRARIES}
  baloocore
)

#
# Modify Job
#
set(filemodifyjobtest_SRC filemodifyjobtest.cpp db.cpp ../filemodifyjob.cpp ../filemapping.cpp ../file.cpp)

kde4_add_unit_test(modifyjobtest NOGUI ${filemodifyjobtest_SRC})

target_link_libraries(modifyjobtest
  ${QT_QTTEST_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${QT_QTSQL_LIBRARY}
  ${XAPIAN_LIBRARIES}
  ${QJSON_LIBRARIES}
  baloocore
)
