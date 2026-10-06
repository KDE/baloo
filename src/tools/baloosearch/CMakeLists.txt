add_definitions(-DTRANSLATION_DOMAIN=\"baloosearch6\")

set(BALOO_SEARCH_SRCS main.cpp)

add_executable(baloosearch6 ${BALOO_SEARCH_SRCS})
ecm_mark_nongui_executable(baloosearch6)
target_compile_definitions(baloosearch6 PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(baloosearch6
  KF6::I18n
  KF6::CoreAddons
  KF6::Baloo
)

install(TARGETS baloosearch6 DESTINATION ${KDE_INSTALL_BINDIR})

# Text if the --list... commands work
# We check if the output matches using a regular expression

add_test(NAME search_list_properties COMMAND baloosearch6 "--listProperties")
set_tests_properties(search_list_properties PROPERTIES
    PASS_REGULAR_EXPRESSION "albumArtist"
)

add_test(NAME search_list_types COMMAND baloosearch6 "--listTypes")
set_tests_properties(search_list_types PROPERTIES
    PASS_REGULAR_EXPRESSION "Document"
)
