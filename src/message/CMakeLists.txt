add_library(message STATIC)

target_include_directories(message PRIVATE ${OUT_SRC_DIR} ${SRC_DIR}
                                           ${GTKMM_INCLUDE_DIRS})

target_sources(
  message
  PRIVATE confirmdiag.cpp
          logmanager.cpp
          messageadmin.cpp
          messageview.cpp
          messageviewbase.cpp
          messagewin.cpp
          post.cpp
          toolbar.cpp)
