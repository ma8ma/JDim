add_library(control STATIC)

target_include_directories(control PRIVATE ${OUT_SRC_DIR} ${SRC_DIR}
                                           ${GTKMM_INCLUDE_DIRS})

target_sources(
  control
  PRIVATE buttonconfig.cpp
          buttonpref.cpp
          control.cpp
          controlutil.cpp
          keyconfig.cpp
          keypref.cpp
          mouseconfig.cpp
          mousekeyconf.cpp
          mousekeypref.cpp
          mousepref.cpp)
