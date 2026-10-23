// ライセンス: GPL2

// 画像つきツールボタン

#ifndef _IMGTOOLBUTTON_H
#define _IMGTOOLBUTTON_H

#include <gtkmm.h>
#include <string>
#include <type_traits>

#include "control/controlutil.h"
#include "icons/iconmanager.h"

namespace SKELETON
{
    template<typename Base>
        requires std::is_base_of_v<Gtk::Button, Base>
    class ToolButtonExtension : public Base
    {
    public:
        explicit ToolButtonExtension( const int iconid )
            : Base()
        {
            Base::set_image( *Gtk::make_managed<Gtk::Image>( ICON::get_icon( iconid ), Gtk::ICON_SIZE_SMALL_TOOLBAR ) );
        }

        ~ToolButtonExtension() noexcept override = default;
    };

    using ImgToolButton = ToolButtonExtension<Gtk::Button>;
    using ImgToggleToolButton = ToolButtonExtension<Gtk::ToggleButton>;
}

#endif
