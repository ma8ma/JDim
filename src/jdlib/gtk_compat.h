// SPDX-License-Identifier: GPL-2.0-or-later
/** @file gtk_compat.h
 *
 * @details GTK3 と GTK4 の両方で動くコードを書くための互換性ライブラリ
 */
#ifndef JDLIB_GTK_COMPAT_H
#define JDLIB_GTK_COMPAT_H

#include <gtkmm/box.h>
#include <gtkmm/dialog.h>
#include <gtkmm/messagedialog.h>


namespace JDLIB::compat {

#ifdef USE_GTKMM4
/** @brief GTK3の `pack_start()` によるパッキングをGTK4移行しやすくする関数
 *
 * @param[out] box    widget を追加する Gtk::Box
 * @param[in] widget  box に追加するウィジェット
 * @param[in] expand  true のときは Gtk::Box の方向に沿って、余分な領域を子ウィジェットに割り当てる
 * @param[in] fill    true のときは割り当てられた領域を子ウィジェットが埋める
 * @param[in] padding ウィジェットの左右または上下に追加するパッディングのサイズ
 */
inline void box_append_packed( Gtk::Box& box, Gtk::Widget& widget,
                               bool expand, bool fill, guint padding )
{
    const bool horizontal = ( box.get_orientation() == Gtk::Orientation::HORIZONTAL );

    if( horizontal ) {
        widget.set_hexpand( expand );
        widget.set_halign( fill ? Gtk::Align::FILL : Gtk::Align::CENTER );
        if( padding != 0 ) {
            widget.set_margin_start( static_cast<int>( padding ) );
            widget.set_margin_end( static_cast<int>( padding ) );
        }
    }
    else {
        widget.set_vexpand( expand );
        widget.set_valign( fill ? Gtk::Align::FILL : Gtk::Align::CENTER );
        if( padding != 0 ) {
            widget.set_margin_top( static_cast<int>( padding ) );
            widget.set_margin_bottom( static_cast<int>( padding ) );
        }
    }
    box.append( widget );
}
#endif

/** @brief GTK3の `pack_start()` による非拡張パッキングをGTK4移行しやすくする関数
 *
 * @param[out] box    widget を追加する Gtk::Box
 * @param[in] widget  box に追加するウィジェット
 * @param[in] padding ウィジェットの左右または上下に追加するパッディングのサイズ
 */
inline void box_append_shrink( Gtk::Box& box, Gtk::Widget& widget, guint padding = 0 )
{
#ifdef USE_GTKMM4
    box_append_packed( box, widget, false, false, padding );
#else
    box.pack_start( widget, false, false, padding );
#endif
}

/** @brief GTK3の `pack_start()` による拡張パッキングをGTK4移行しやすくする関数
 *
 * @param[out] box    widget を追加する Gtk::Box
 * @param[in] widget  box に追加するウィジェット
 * @param[in] padding ウィジェットの左右または上下に追加するパッディングのサイズ
 */
inline void box_append_expand( Gtk::Box& box, Gtk::Widget& widget, guint padding = 0 )
{
#ifdef USE_GTKMM4
    box_append_packed( box, widget, true, true, padding );
#else
    box.pack_start( widget, true, true, padding );
#endif
}

/** @brief GTK3の `pack_start()` による領域自体は拡張（expand）するが、
 *  ウィジェット自体は拡大（fill）せず配置するパッキングをGTK4移行しやすくする関数
 *
 * @param[out] box    widget を追加する Gtk::Box
 * @param[in] widget  box に追加するウィジェット
 * @param[in] padding ウィジェットの左右または上下に追加するパッディングのサイズ
 */
inline void box_append_expand_nofill( Gtk::Box& box, Gtk::Widget& widget, guint padding = 0 )
{
#ifdef USE_GTKMM4
    box_append_packed( box, widget, true, false, padding );
#else
    box.pack_start( widget, true, false, padding );
#endif
}

/**
 * @brief Gtk::Align の alias
 *
 * @note 未使用またはコメントアウトされている enum 値は alias を作成していません。
 */
namespace Align {
using type = Gtk::Align;

#ifdef USE_GTKMM4
using Gtk::Align::FILL;
using Gtk::Align::START;
using Gtk::Align::END;
using Gtk::Align::CENTER;
#else
constexpr type FILL = Gtk::ALIGN_FILL;
constexpr type START = Gtk::ALIGN_START;
constexpr type END = Gtk::ALIGN_END;
constexpr type CENTER = Gtk::ALIGN_CENTER;
#endif
} // Align

/**
 * @brief Gtk::ButtonsType の alias
 *
 * @note 未使用またはコメントアウトされている enum 値は alias を作成していません。
 */
namespace ButtonsType {
using type = Gtk::ButtonsType;

#ifdef USE_GTKMM4
using Gtk::ButtonsType::NONE;
using Gtk::ButtonsType::OK;
using Gtk::ButtonsType::CANCEL;
using Gtk::ButtonsType::YES_NO;
using Gtk::ButtonsType::OK_CANCEL;
#else
constexpr type NONE = Gtk::BUTTONS_NONE;
constexpr type OK = Gtk::BUTTONS_OK;
constexpr type CANCEL = Gtk::BUTTONS_CANCEL;
constexpr type YES_NO = Gtk::BUTTONS_YES_NO;
constexpr type OK_CANCEL = Gtk::BUTTONS_OK_CANCEL;
#endif
} // ButtonsType

/**
 * @brief Gtk::MessageType の alias
 *
 * @note 未使用またはコメントアウトされている enum 値は alias を作成していません。
 */
namespace MessageType {
using type = Gtk::MessageType;

#ifdef USE_GTKMM4
using Gtk::MessageType::INFO;
using Gtk::MessageType::WARNING;
using Gtk::MessageType::QUESTION;
using Gtk::MessageType::ERROR;
#else
constexpr type INFO = Gtk::MESSAGE_INFO;
constexpr type WARNING = Gtk::MESSAGE_WARNING;
constexpr type QUESTION = Gtk::MESSAGE_QUESTION;
constexpr type ERROR = Gtk::MESSAGE_ERROR;
#endif
} // MessageType

/**
 * @brief Gtk::ResponseType の alias
 *
 * @note 未使用またはコメントアウトされている enum 値は alias を作成していません。
 */
namespace ResponseType {
using type = Gtk::ResponseType;

#ifdef USE_GTKMM4
using Gtk::ResponseType::ACCEPT;
using Gtk::ResponseType::OK;
using Gtk::ResponseType::CANCEL;
using Gtk::ResponseType::CLOSE;
using Gtk::ResponseType::YES;
using Gtk::ResponseType::NO;
#else
constexpr type ACCEPT = Gtk::RESPONSE_ACCEPT;
constexpr type OK = Gtk::RESPONSE_OK;
constexpr type CANCEL = Gtk::RESPONSE_CANCEL;
constexpr type CLOSE = Gtk::RESPONSE_CLOSE;
constexpr type YES = Gtk::RESPONSE_YES;
constexpr type NO = Gtk::RESPONSE_NO;
#endif
} // ResponseType

} // JDLIB::compat

#endif // JDLIB_GTK_COMPAT_H
