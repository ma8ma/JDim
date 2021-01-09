# JDim用 CMakeLists.txt
# NOTE: 実験的なサポートのため変更または廃止の可能性がある

# cmakeを使ってJDimをビルドする方法
#
# Fedora
#   ディストロのパッケージをインストールする
#   ```
#   dnf install git libtool cmake
#   dnf install gnutls-devel gtkmm30-devel libSM-devel
#   ```
#
# Debian, Ubuntu
#   ディストロのパッケージをインストールする
#   ```
#   sudo apt install build-essential git libtool cmake
#   sudo apt install libgnutls28-dev libgtkmm-3.0-dev libltdl-dev
#   ```
#
# ビルドの手順
#   ```
#   git clone -b master --depth 1 https://github.com/JDimproved/JDim.git jdim
#   cd jdim
#   cmake -B build
#   cmake --build build
#   ```
# Tips
#   - ビルドオプションは `cmake -B build -Dregex=glib` のように指定する
#   - 生成された実行ファイルの場所は build/src/jdim

cmake_minimum_required(VERSION 3.7)

project(
  jdim
  VERSION 0.4.0
  LANGUAGES CXX)
# LICENSE : GPL-2.0
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 追加コンパイルオプション
add_compile_options(-DHAVE_CONFIG_H=1)
add_compile_options(-DGTK_DOMAIN=\"gtk30\")
# -Wextraで有効になる-Wunused-parameterは修正方法の検討が必要なので暫定的に無効
add_compile_options(-Wall -Wextra -Wno-unused-parameter -Wpedantic)

# オプションの表示にはconfigureスタイル('--with-foo')が必要なので注意
set(configure_args "")

# cmake モジュール
include(CheckCXXCompilerFlag)
include(CheckIncludeFileCXX)
include(FindPkgConfig)

#
# OSを判定してOS別の設定
#
if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
  set(USE_MKTIME 1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
  set(USE_TIMEGM 1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(USE_MKTIME 1)
endif()

#
# 必須パッケージのチェック
#
pkg_check_modules(GTKMM gtkmm-3.0>=3.22.0)
if(NOT GTKMM_FOUND)
  message(
    WARNING "JDim will require gtkmm-3.0 >= 3.22.0 in the future release.")
  pkg_check_modules(GTKMM REQUIRED gtkmm-3.0>=3.18.0)
endif()
find_package(Threads)
pkg_check_modules(X11 REQUIRED x11)
pkg_check_modules(ZLIB REQUIRED zlib>=1.2.0)

# crypt
find_library(CRYPT crypt)
if(EXISTS "${CRYPT}")
  set(CRYPT_LINK_LIBRARIES "${CRYPT}")
else()
  pkg_check_modules(CRYPT libcrypt)
  if(NOT CRYPT_FOUND)
    pkg_check_modules(CRYPT REQUIRED libxcrypt)
  endif()
endif()
check_include_file_cxx(crypt.h HAVE_CRYPT_H)

# socket
find_library(SOCKET socket)
if(EXISTS "${SOCKET}")
  set(SOCKET_LINK_LIBRARIES "${SOCKET}")
endif()

# utsname
check_include_file_cxx(sys/utsname.h HAVE_SYS_UTSNAME_H)

#
# オプションのパッケージ
#

# セッション管理
if(NOT DEFINED sessionlib)
  set(sessionlib "xsmp")
endif()
if(sessionlib STREQUAL "xsmp")
  pkg_check_modules(SM REQUIRED sm>=1.2.0)
  pkg_check_modules(ICE REQUIRED ice>=1.0.0)
  set(USE_XSMP 1)
elseif(sessionlib STREQUAL "no")
  list(APPEND configure_args "'--with-sessionlib=no'")
else()
  message(
    FATAL_ERROR
      "Value \"${sessionlib}\" for sessionlib option is not one of the choices. "
      "Possible choices are: \"xsmp\", \"no\".")
endif()

# 正規表現ライブラリ
if(NOT DEFINED regex)
  set(regex "glib")
endif()
if(regex STREQUAL "posix")
  set(HAVE_REGEX_H 1)
  list(APPEND configure_args "'--with-regex=posix'")
  message(WARNING "-Dregex=posix is deprecated. Use -Dregex=glib instead.")
elseif(regex STREQUAL "oniguruma")
  pkg_check_modules(REGEXP REQUIRED oniguruma)
  check_include_file_cxx(onigposix.h HAVE_ONIGPOSIX_H)
  if(NOT HAVE_ONIGPOSIX_H)
    message(FATAL_ERROR "onigposix.h not found")
  endif()
  list(APPEND configure_args "'--with-regex=oniguruma'")
elseif(NOT regex STREQUAL "glib")
  message(
    FATAL_ERROR
      "Value \"${regex}\" for sessionlib option is not one of the choices. "
      "Possible choices are: \"posix\", \"oniguruma\", \"glib\".")
endif()

# TLS
if(NOT DEFINED tls)
  set(tls "gnutls")
endif()
if(tls STREQUAL "gnutls")
  pkg_check_modules(TLS REQUIRED gnutls>=3.4.10)
  set(USE_GNUTLS 1)
elseif(tls STREQUAL "openssl")
  pkg_check_modules(TLS REQUIRED openssl>=0.9.0)
  set(USE_OPENSSL 1)
  list(APPEND configure_args "'--with-tls=openssl'")
else()
  message(
    FATAL_ERROR
      "Value \"${tls}\" for sessionlib option is not one of the choices. "
      "Possible choices are: \"gnutls\", \"openssl\".")
endif()

# migemo
option(migemo "Use text search by romaji" OFF)
if(migemo)
  find_library(MIGEMO migemo)
  check_include_file_cxx(migemo.h HAVE_MIGEMO_H)
  if(MIGEMO AND HAVE_MIGEMO_H)
    set(MIGEMO_LINK_LIBRARIES "${MIGEMO}")
  else()
    message(FATAL_ERROR "migemo option is turned ON, but not found")
  endif()
endif()
if(DEFINED migemodict)
  set(MIGEMODICT "${migemodict}")
  list(APPEND configure_args "'--with-migemodict=${migemodict}'")
  message(STATUS "Default path for migemo dictionary file: ${migemodict}")
endif()

# alsa
option(alsa "Use sound effects" OFF)
if(alsa)
  pkg_check_modules(ALSA REQUIRED alsa>=1.0.0)
  check_include_file_cxx(alsa/asoundlib.h HAVE_ALSA_ASOUNDLIB_H)
  set(USE_ALSA 1)
  list(APPEND configure_args "'--with-alsa'")
endif()

# googletestが見つからない場合はテストはしない
option(build_tests "Build test program if gtest found" ON)
if(build_tests)
  pkg_check_modules(GTEST_MAIN gtest_main)
  if(NOT GTEST_MAIN_FOUND)
    set(build_tests OFF)
  endif()
endif()

#
# オプションの機能
#

# Use PangoLayout instead of PangoGlyphString
option(pangolayout "Render text by PangoLayout" OFF)
if(pangolayout)
  set(USE_PANGOLAYOUT 1)
  list(APPEND configure_args "'--with-pangolayout'")
  message(STATUS "Render text by PangoLayout: YES")
endif()

# compatible cache directory
option(compat_cache_dir "Use compatible cache directory" ON)
if(compat_cache_dir)
  set(ENABLE_COMPAT_CACHE_DIR 1)
  message(STATUS "Use compatible cache directory: YES")
else()
  list(APPEND configure_args "'--disable-compat-cache-dir'")
  message(STATUS "Use compatible cache directory: NO")
endif()

#
# コンパイラーの追加オプション
#

# gprof support
option(gprof "Output profile information for gprof" OFF)
if(gprof)
  check_cxx_compiler_flag(-pg USE_GPROF)
  if(USE_GPROF)
    add_compile_options(-pg)
    list(APPEND configure_args "'--enable-gprof'")
    message(STATUS "Output profile information for gprof : YES")
  else()
    message(FATAL_ERROR "compiler does not support -pg option")
  endif()
endif()

# CPUの最適化オプション
option(native "Optimize to your machine" OFF)
if(native)
  check_cxx_compiler_flag(-march=native USE_NATIVE)
  if(USE_NATIVE)
    add_compile_options(-march=native)
    list(APPEND configure_args "'--with-native'")
    message(STATUS "Optimize to your machine: YES")
  else()
    message(FATAL_ERROR "compiler does not support -march=native option")
  endif()
endif()

#
# ビルドの情報
#

message("\n*** Configuration ***")
message("alsa = ${alsa}")
message("build_tests = ${build_tests}")
message("compat_cache_dir = ${compat_cache_dir}")
message("gprof = ${gprof}")
message("migemo = ${migemo}")
message("migemodict = ${migemodict}")
message("native = ${native}")
message("pangolayout = ${pangolayout}")
message("regex = ${regex}")
message("sessionlib = ${sessionlib}")
message("tls = ${tls}")
message("*********************\n")

# ビルドオプションの文字列を空白で連結する
string(REPLACE ";" " " CONFIGURE_ARGS "${configure_args}")

add_subdirectory(src)
if(build_tests)
  add_subdirectory(test)
endif()

#
# プログラムと一緒にインストールするアイコンや設定など
#
install(FILES jdim.png DESTINATION "share/icons/hicolor/48x48/apps")
install(FILES jdim.svg DESTINATION "share/icons/hicolor/scalable/apps")

install(FILES jdim.desktop DESTINATION "share/applications")
install(FILES jdim.metainfo.xml DESTINATION "share/metainfo")
