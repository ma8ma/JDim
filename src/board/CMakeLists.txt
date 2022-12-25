add_library(board STATIC)

target_include_directories(board PRIVATE ${OUT_SRC_DIR} ${SRC_DIR}
                                         ${GTKMM_INCLUDE_DIRS})

target_sources(
  board
  PRIVATE boardadmin.cpp
          boardview.cpp
          boardviewbase.cpp
          boardviewlog.cpp
          boardviewnext.cpp
          boardviewsidebar.cpp
          preference.cpp
          toolbar.cpp)
