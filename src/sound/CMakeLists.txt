add_library(sound STATIC)

target_include_directories(
  sound PRIVATE ${OUT_SRC_DIR} ${SRC_DIR} ${ALSA_INCLUDE_DIRS}
                ${GTKMM_INCLUDE_DIRS})

target_sources(sound PRIVATE playsound.cpp soundmanager.cpp)
