add_executable(baloo_file_extractor_dummy extractor.cpp)
set_target_properties(baloo_file_extractor_dummy
    PROPERTIES OUTPUT_NAME baloo_file_extractor
)
if(WIN32)
    set_target_properties(baloo_file_extractor_dummy
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()

target_link_libraries(baloo_file_extractor_dummy
  Qt5::Core
)

add_definitions("-DBALOO_TEMP_PATH=\"${CMAKE_CURRENT_BINARY_DIR}\"")
ecm_add_test(fileindexingjobtest.cpp ../../fileindexingjob.cpp
    TEST_NAME "fileindexingjobtest"
    LINK_LIBRARIES Qt5::Test KF5::CoreAddons KF5::ConfigCore KF5::FileMetaData ${XAPIAN_LIBRARIES}
)
