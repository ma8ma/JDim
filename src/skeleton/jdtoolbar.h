// ライセンス: GPL2
//
// カスタマイズした Gtk::Toolbar ( 背景を描画しない )
//
// (注) DragableNoteBookにappendするのは SKELETON::ToolBar
//

#ifndef JDTOOLBAR_H
#define JDTOOLBAR_H

#include <gtkmm.h>

namespace SKELETON
{
    class JDToolbar : public Gtk::Toolbar
    {
    public:
        JDToolbar(){}

        // GTK+3ではデフォルトの描画処理に任せる
#if !GTKMM_CHECK_VERSION(3,0,0)
    protected:
        virtual bool on_expose_event( GdkEventExpose* event );
#endif
    };
}

#endif
