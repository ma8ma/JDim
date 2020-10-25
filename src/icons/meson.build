# meson 組み込みの [gnome] モジュールを使うと .c ファイルが出力されるため
# コマンドを明示的に指定して .cpp ファイルを生成し C++ コンパイラで処理する
#
# 静的ライブラリに GResource オブジェクトを含めると __attribute__((constructor)) gcc拡張が機能しない
# そのため実行ファイルのターゲット jdim_exe のソースに入れる
#
# [gnome]: https://mesonbuild.com/Gnome-module.html

glib_compile_resources = find_program('glib-compile-resources')
icons_gresource_gen = generator(glib_compile_resources,
                                output : '@BASENAME@.cpp',
                                depfile : '@BASENAME@.cpp.d',
                                arguments : ['@INPUT@', '--sourcedir', '@CURRENT_SOURCE_DIR@',
                                             '--internal', '--generate', '--target', '@OUTPUT@',
                                             '--dependency-file', '@DEPFILE@'])
icons_gresource_cpp = icons_gresource_gen.process('icons.gresource.xml')


sources = [
  'iconmanager.cpp',
]


icon_lib = static_library(
  'icon', [sources],
  dependencies : gtkmm_dep,
  include_directories : include_directories('..'),
)
