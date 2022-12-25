add_library(bbslist STATIC)

target_include_directories(bbslist PRIVATE ${OUT_SRC_DIR} ${SRC_DIR}
                                           ${GTKMM_INCLUDE_DIRS})

target_sources(
  bbslist
  PRIVATE addetcdialog.cpp
          bbslistadmin.cpp
          bbslistview.cpp
          bbslistviewbase.cpp
          columns.cpp
          editlistwin.cpp
          favoriteview.cpp
          historyview.cpp
          selectdialog.cpp
          selectlistview.cpp
          toolbar.cpp)
