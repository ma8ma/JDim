set(icon_files
    bkmark.png
    bkmark_broken_subject.png
    bkmark_thread.png
    bkmark_update.png
    board.png
    board_update.png
    board_updated.png
    broken_subject.png
    check.png
    dir.png
    down.png
    favorite.png
    hist.png
    hist_board.png
    hist_close.png
    hist_closeboard.png
    hist_closeimg.png
    image.png
    info.png
    jd16.png
    jd32.png
    jd48.png
    jd96.png
    link.png
    loading.png
    loading_stop.png
    newthread.png
    newthread_hour.png
    post.png
    post_refer.png
    thread.png
    thread_old.png
    thread_update.png
    thread_updated.png
    update.png
    write.png)

find_program(gdk_pixbuf_csource gdk-pixbuf-csource)

# Generate CXX header files from icon png images.
foreach(f IN ITEMS ${icon_files})
  # Get filename without extension.
  string(REGEX REPLACE "^(.*)\\.[^.]*$" "\\1" basename "${f}")
  add_custom_command(
    OUTPUT "${basename}.h"
    COMMAND ${gdk_pixbuf_csource} --raw --name="icon_${basename}"
            "${CMAKE_CURRENT_SOURCE_DIR}/${f}" >"${basename}.h")
  list(APPEND icon_headers "${basename}.h")
endforeach()

add_library(icon STATIC)

target_include_directories(
  icon PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${OUT_SRC_DIR} ${SRC_DIR}
               ${GTKMM_INCLUDE_DIRS})

target_sources(icon PRIVATE iconmanager.cpp ${icon_headers})
