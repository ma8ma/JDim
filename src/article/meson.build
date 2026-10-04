# SPDX-License-Identifier: GPL-2.0-or-later

sources = [
  'articleadmin.cpp',
  'articleview.cpp',
  'articleviewbase.cpp',
  'articleviewetc.cpp',
  'articleviewinfo.cpp',
  'articleviewpopup.cpp',
  'articleviewpreview.cpp',
  'articleviewsearch.cpp',
  'drawareabase.cpp',
  'drawareainfo.cpp',
  'drawareamain.cpp',
  'drawareapopup.cpp',
  'embeddedimage.cpp',
  'font.cpp',
  'layouttree.cpp',
  'preference.cpp',
  'toolbar.cpp',
  'toolbarsearch.cpp',
  'toolbarsimple.cpp',
]

if use_gtkmm4
  sources += [
    'articleviewbase_menu_gtk4.cpp',
  ]
else
  sources += [
    'articleviewbase_menu_gtk3.cpp',
  ]
endif

article_lib = static_library(
  'article', sources,
  dependencies : [config_h_dep, gtkmm_dep],
  include_directories : include_directories('..'),
)
