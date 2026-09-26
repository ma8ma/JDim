# SPDX-License-Identifier: GPL-2.0-or-later

sources = [
  'imageadmin.cpp',
  'imagearea.cpp',
  'imageareabase.cpp',
  'imageareaicon.cpp',
  'imageareapopup.cpp',
  'imageview.cpp',
  'imageviewbase.cpp',
  'imageviewicon.cpp',
  'imageviewpopup.cpp',
  'imagewin.cpp',
  'preference.cpp',
]

if use_gtkmm4
  sources += [
    'imageviewbase_menu_gtk4.cpp',
  ]
else
  sources += [
    'imageviewbase_menu_gtk3.cpp',
  ]
endif


image_lib = static_library(
  'image', sources,
  dependencies : gtkmm_dep,
  include_directories : include_directories('..'),
)
