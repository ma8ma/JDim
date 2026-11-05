// SPDX-License-Identifier: GPL-2.0-or-later
/** @file gtk_compat.h
 *
 * @details GTK3 と GTK4 の両方で動くコードを書くための互換性ライブラリ
 */
#ifndef JDLIB_GTK_COMPAT_H
#define JDLIB_GTK_COMPAT_H

#include <gtkmm/box.h>

#include <memory>

#include "enum_compat.h"

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
 * @brief Glib::RefPtr を dynamic_pointer_cast する
 *
 * GTKMM3 と GTKMM4 の RefPtr に対する
 * dynamic_pointer_cast の互換処理を行います。
 *
 * @tparam T キャスト先の型
 * @tparam U キャスト元の型
 * @param[in] src キャストする RefPtr
 * @return キャストした RefPtr。
 *         キャストに失敗した場合は空の RefPtr を返します。
 */
template<typename T, typename U>
inline Glib::RefPtr<T> dynamic_pointer_cast( const Glib::RefPtr<U>& src )
{
#ifdef USE_GTKMM4
    return std::dynamic_pointer_cast<T>( src );
#else
    return Glib::RefPtr<T>::cast_dynamic( src );
#endif
}

/**
 * @brief Glib::RefPtr を const_pointer_cast する
 *
 * GTKMM3 と GTKMM4 の RefPtr に対する
 * const_pointer_cast の互換処理を行います。
 *
 * @tparam T キャスト先の型
 * @tparam U キャスト元の型
 * @param[in] src キャストする RefPtr
 * @return const_pointer_cast した RefPtr
 */
template<typename T, typename U>
inline Glib::RefPtr<T> const_pointer_cast( const Glib::RefPtr<U>& src )
{
#ifdef USE_GTKMM4
    return std::const_pointer_cast<T>( src );
#else
    return Glib::RefPtr<T>::cast_const( src );
#endif
}

} // JDLIB::compat

#endif // JDLIB_GTK_COMPAT_H
