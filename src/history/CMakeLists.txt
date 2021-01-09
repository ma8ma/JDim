add_library(history STATIC)

target_include_directories(history PRIVATE ${OUT_SRC_DIR} ${SRC_DIR}
                                           ${GTKMM_INCLUDE_DIRS})

target_sources(history PRIVATE historymanager.cpp historymenu.cpp
                               historysubmenu.cpp viewhistory.cpp)
