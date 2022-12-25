add_library(dbimg STATIC)

target_include_directories(dbimg PRIVATE ${OUT_SRC_DIR} ${SRC_DIR}
                                         ${GTKMM_INCLUDE_DIRS})

target_sources(dbimg PRIVATE delimgcachediag.cpp img.cpp imginterface.cpp
                             imgroot.cpp)
