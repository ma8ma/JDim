add_library(config STATIC)

target_include_directories(config PRIVATE ${OUT_SRC_DIR} ${SRC_DIR}
                                          ${GTKMM_INCLUDE_DIRS})

target_sources(config PRIVATE aboutconfig.cpp aboutconfigdiag.cpp
                              configitems.cpp globalconf.cpp)
