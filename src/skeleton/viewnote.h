// ライセンス: GPL2
//
// DragableNoteBookを構成するview表示用の Notebook
//

#ifndef _VIEWNOTE_H
#define _VIEWNOTE_H

#include <gtkmm.h>

namespace SKELETON
{
    class DragableNoteBook;

    class ViewNotebook : public Gtk::Notebook
    {
        DragableNoteBook* m_parent;

      public:

        ViewNotebook( DragableNoteBook* parent );

        void redraw_scrollbar();

#if !GTKMM_CHECK_VERSION(3,0,0)
      protected:

        virtual bool on_expose_event( GdkEventExpose* event );
#endif
    };
}

#endif
