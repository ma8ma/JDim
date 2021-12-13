# 作業ディレクトリの状態を反映するためにビルド毎に更新する
add_custom_target(
  buildinfo_h ALL COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/buildinfo.h.sh
                          >${CMAKE_CURRENT_BINARY_DIR}/buildinfo.h)

# Generate config.h
string(
  CONCAT CONFIG_H_PRE_STR
         "#pragma once\n"
         "#cmakedefine CONFIGURE_ARGS \"@CONFIGURE_ARGS@\"\n"
         "#cmakedefine ENABLE_COMPAT_CACHE_DIR @ENABLE_COMPAT_CACHE_DIR@\n"
         "#cmakedefine HAVE_CRYPT_R @HAVE_CRYPT_R@\n"
         "#cmakedefine HAVE_MIGEMO_H @HAVE_MIGEMO_H@\n"
         "#cmakedefine HAVE_ONIGPOSIX_H @HAVE_ONIGPOSIX_H@\n"
         "#cmakedefine HAVE_REGEX_H @HAVE_REGEX_H@\n"
         "#cmakedefine MIGEMODICT \"@MIGEMODICT@\"\n"
         "#cmakedefine NO_TIMEGM @NO_TIMEGM@\n"
         "#cmakedefine USE_ALSA @USE_ALSA@\n"
         "#cmakedefine USE_GNUTLS @USE_GNUTLS@\n"
         "#cmakedefine USE_OPENSSL @USE_OPENSSL@\n"
         "#cmakedefine USE_PANGOLAYOUT @USE_PANGOLAYOUT@\n"
         "#cmakedefine USE_XSMP @USE_XSMP@\n"
         "#define HAVE_BUILDINFO_H 1\n")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/config.h.pre "${CONFIG_H_PRE_STR}")
configure_file(${CMAKE_CURRENT_BINARY_DIR}/config.h.pre config.h)

# 実行ファイルにリンクするライブラリの構成
set(OUT_SRC_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
add_subdirectory(article)
add_subdirectory(bbslist)
add_subdirectory(board)
add_subdirectory(config)
add_subdirectory(control)
add_subdirectory(dbimg)
add_subdirectory(dbtree)
add_subdirectory(history)
add_subdirectory(icons)
add_subdirectory(image)
add_subdirectory(jdlib)
add_subdirectory(message)
add_subdirectory(skeleton)
add_subdirectory(sound)
add_subdirectory(xml)

# 実行ファイルとテストプログラムに使うソースはObject libraryにする
add_library(core_objects OBJECT)
add_dependencies(core_objects buildinfo_h)

target_include_directories(
  core_objects PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
                       ${GTKMM_INCLUDE_DIRS})

target_sources(
  core_objects
  PRIVATE aamanager.cpp
          articleitemmenupref.cpp
          articleitempref.cpp
          boarditemmenupref.cpp
          boarditempref.cpp
          browsers.cpp
          cache.cpp
          command.cpp
          compmanager.cpp
          core.cpp
          cssmanager.cpp
          dispatchmanager.cpp
          dndmanager.cpp
          environment.cpp
          fontcolorpref.cpp
          iomonitor.cpp
          linkfiltermanager.cpp
          linkfilterpref.cpp
          livepref.cpp
          login2ch.cpp
          loginbe.cpp
          mainitempref.cpp
          maintoolbar.cpp
          menuslots.cpp
          msgitempref.cpp
          openurldiag.cpp
          prefdiagfactory.cpp
          replacestrmanager.cpp
          replacestrpref.cpp
          searchitempref.cpp
          searchloader.cpp
          searchmanager.cpp
          session.cpp
          setupwizard.cpp
          sharedbuffer.cpp
          sidebaritempref.cpp
          updatemanager.cpp
          urlreplacemanager.cpp
          usrcmdmanager.cpp
          usrcmdpref.cpp
          viewfactory.cpp
          winmain.cpp)

# 実行ファイルの構成
add_executable(jdim $<TARGET_OBJECTS:core_objects>)
install(TARGETS jdim DESTINATION bin)

target_include_directories(
  jdim PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
               ${GTKMM_INCLUDE_DIRS} ${ICE_INCLUDE_DIRS} ${SM_INCLUDE_DIRS})

target_link_libraries(
  jdim
  PRIVATE article
          bbslist
          board
          config
          control
          dbimg
          dbtree
          history
          icon
          image
          message # raise link error if put message behind jdlib
          jdlib
          skeleton
          sound
          xml
          ${ALSA_LINK_LIBRARIES}
          ${CMAKE_THREAD_LIBS_INIT}
          ${CRYPT_LINK_LIBRARIES}
          ${GTKMM_LINK_LIBRARIES}
          ${ICE_LINK_LIBRARIES}
          ${MIGEMO_LINK_LIBRARIES}
          ${REGEXP_LINK_LIBRARIES}
          ${SM_LINK_LIBRARIES}
          ${SOCKET_LINK_LIBRARIES}
          ${TLS_LINK_LIBRARIES}
          ${X11_LINK_LIBRARIES}
          ${ZLIB_LINK_LIBRARIES})

target_sources(jdim PRIVATE main.cpp)
