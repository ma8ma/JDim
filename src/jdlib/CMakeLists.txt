add_library(jdlib STATIC)

target_include_directories(
  jdlib
  PRIVATE ${OUT_SRC_DIR}
          ${SRC_DIR}
          ${GTKMM_INCLUDE_DIRS}
          ${REGEXP_INCLUDE_DIRS}
          ${TLS_INCLUDE_DIRS}
          ${X11_INCLUDE_DIRS}
          ${ZLIB_INCLUDE_DIRS})

target_sources(
  jdlib
  PRIVATE confloader.cpp
          cookiemanager.cpp
          heap.cpp
          imgloader.cpp
          jdiconv.cpp
          jdmigemo.cpp
          jdregex.cpp
          jdthread.cpp
          loader.cpp
          loaderdata.cpp
          misccharcode.cpp
          miscgtk.cpp
          miscmsg.cpp
          misctime.cpp
          misctrip.cpp
          miscutil.cpp
          miscx.cpp
          ssl.cpp
          tfidf.cpp
          timeout.cpp)
