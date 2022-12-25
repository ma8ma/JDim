add_library(xml STATIC)

target_include_directories(xml PRIVATE ${OUT_SRC_DIR} ${SRC_DIR}
                                       ${GTKMM_INCLUDE_DIRS})

target_sources(xml PRIVATE document.cpp dom.cpp tools.cpp)
