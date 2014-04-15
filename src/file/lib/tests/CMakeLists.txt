add_executable(xattrdetectortest
               xattrdetectortest.cpp
               ../xattrdetector.cpp
)

target_link_libraries(xattrdetectortest
  Qt5::Core
  KF5::Solid
)
