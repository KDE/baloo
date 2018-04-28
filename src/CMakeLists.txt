add_subdirectory(lib)
add_subdirectory(engine)
add_subdirectory(codecs)
add_subdirectory(qml)

if (BUILD_KINOTIFY)
    add_subdirectory(file)
    add_subdirectory(kioslaves)
    add_subdirectory(tools)
endif()

add_subdirectory(dbus)

