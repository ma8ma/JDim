add_library(article STATIC)

target_include_directories(article PRIVATE ${OUT_SRC_DIR} ${SRC_DIR}
                                           ${GTKMM_INCLUDE_DIRS})

target_sources(
  article
  PRIVATE articleadmin.cpp
          articleview.cpp
          articleviewbase.cpp
          articleviewetc.cpp
          articleviewinfo.cpp
          articleviewpopup.cpp
          articleviewpreview.cpp
          articleviewsearch.cpp
          drawareabase.cpp
          drawareainfo.cpp
          drawareamain.cpp
          drawareapopup.cpp
          embeddedimage.cpp
          font.cpp
          layouttree.cpp
          preference.cpp
          toolbar.cpp
          toolbarsearch.cpp
          toolbarsimple.cpp)
