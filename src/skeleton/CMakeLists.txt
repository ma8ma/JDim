add_library(skeleton STATIC)

target_include_directories(skeleton PRIVATE ${OUT_SRC_DIR} ${SRC_DIR}
                                            ${GTKMM_INCLUDE_DIRS})

target_sources(
  skeleton
  PRIVATE aamenu.cpp
          aboutdiag.cpp
          admin.cpp
          backforwardbutton.cpp
          compentry.cpp
          detaildiag.cpp
          dispatchable.cpp
          dragnote.cpp
          dragtreeview.cpp
          editcolumns.cpp
          edittreeview.cpp
          editview.cpp
          entry.cpp
          hpaned.cpp
          jdtoolbar.cpp
          label_entry.cpp
          loadable.cpp
          login.cpp
          menubutton.cpp
          msgdiag.cpp
          notebook.cpp
          panecontrol.cpp
          popupwin.cpp
          popupwinbase.cpp
          prefdiag.cpp
          selectitempref.cpp
          tablabel.cpp
          tabnote.cpp
          tabswitchbutton.cpp
          tabswitchmenu.cpp
          textloader.cpp
          toolbar.cpp
          toolbarnote.cpp
          toolmenubutton.cpp
          treeviewbase.cpp
          undobuffer.cpp
          vbox.cpp
          view.cpp
          viewnote.cpp
          vpaned.cpp
          window.cpp)
