# SPDX-License-Identifier: GPL-2.0-or-later
#
# gnome.compile_resources() は meson 0.56 から C++ プロジェクトでcppソースファイルを
# 出力するようになった。 https://github.com/mesonbuild/meson/pull/7841
#
# 静的ライブラリに生成したソースファイルを含めると __attribute__((constructor)) gcc拡張が機能しない
# そのため実行ファイルのターゲット jdim_exe のソースに追加する

gnome = import('gnome')

jdim_ui_gresource_cpp = gnome.compile_resources(
  'jdim_ui',
  'jdim-ui.gresource.xml',
  source_dir: '.',
)
