add_library(image STATIC)

target_include_directories(image PRIVATE ${OUT_SRC_DIR} ${SRC_DIR}
                                         ${GTKMM_INCLUDE_DIRS})

target_sources(
  image
  PRIVATE imageadmin.cpp
          imagearea.cpp
          imageareabase.cpp
          imageareaicon.cpp
          imageareapopup.cpp
          imageview.cpp
          imageviewbase.cpp
          imageviewicon.cpp
          imageviewpopup.cpp
          imagewin.cpp
          preference.cpp)
