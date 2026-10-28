// SPDX-License-Identifier: GPL-2.0-or-later
/** @file gtk_compat.h
 *
 * @details GTK3 と GTK4 の両方で動くコードを書くための互換性ライブラリ
 */
#ifndef JDLIB_GTK_COMPAT_H
#define JDLIB_GTK_COMPAT_H

#include <gtkmm/box.h>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/filechooser.h>
#include <gtkmm/gesture.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/textview.h>
#include <gtkmm/treeviewcolumn.h>


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
 * @brief Gtk::IconPosition の alias
 */
namespace Entry::IconPosition {

#ifdef USE_GTKMM4
using type = Gtk::Entry::IconPosition;
using Gtk::Entry::IconPosition::PRIMARY;
using Gtk::Entry::IconPosition::SECONDARY;
#else
using type = Gtk::EntryIconPosition;
constexpr type PRIMARY = Gtk::ENTRY_ICON_PRIMARY;
constexpr type SECONDARY = Gtk::ENTRY_ICON_SECONDARY;
#endif
} // IconPosition

/**
 * @brief Gtk::EventSequenceState の alias
 */
namespace EventSequenceState {
using type = Gtk::EventSequenceState;

#ifdef USE_GTKMM4
using Gtk::EventSequenceState::NONE;
using Gtk::EventSequenceState::CLAIMED;
using Gtk::EventSequenceState::DENIED;
#else
constexpr type NONE = Gtk::EVENT_SEQUENCE_NONE;
constexpr type CLAIMED = Gtk::EVENT_SEQUENCE_CLAIMED;
constexpr type DENIED = Gtk::EVENT_SEQUENCE_DENIED;
#endif
} // EventSequenceState

/**
 * @brief Gtk::FileChooser::Action の alias
 */
namespace FileChooser::Action {

#ifdef USE_GTKMM4
using type = Gtk::FileChooser::Action;
using Gtk::FileChooser::Action::OPEN;
using Gtk::FileChooser::Action::SAVE;
using Gtk::FileChooser::Action::SELECT_FOLDER;
#else
using type = Gtk::FileChooserAction;
constexpr type OPEN = Gtk::FILE_CHOOSER_ACTION_OPEN;
constexpr type SAVE = Gtk::FILE_CHOOSER_ACTION_SAVE;
constexpr type SELECT_FOLDER = Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER;
constexpr type CREATE_FOLDER = Gtk::FILE_CHOOSER_ACTION_CREATE_FOLDER;
#endif
} // FileChooser::Action

/**
 * @brief Gtk::InputPurpose の alias
 */
namespace InputPurpose {
using type = Gtk::InputPurpose;

#ifdef USE_GTKMM4
using Gtk::InputPurpose::FREE_FORM;
using Gtk::InputPurpose::ALPHA;
using Gtk::InputPurpose::DIGITS;
using Gtk::InputPurpose::NUMBER;
using Gtk::InputPurpose::PHONE;
using Gtk::InputPurpose::URL;
using Gtk::InputPurpose::EMAIL;
using Gtk::InputPurpose::NAME;
using Gtk::InputPurpose::PASSWORD;
using Gtk::InputPurpose::PIN;
using Gtk::InputPurpose::TERMINAL;
#else
constexpr type FREE_FORM = Gtk::INPUT_PURPOSE_FREE_FORM;
constexpr type ALPHA = Gtk::INPUT_PURPOSE_ALPHA;
constexpr type DIGITS = Gtk::INPUT_PURPOSE_DIGITS;
constexpr type NUMBER = Gtk::INPUT_PURPOSE_NUMBER;
constexpr type PHONE = Gtk::INPUT_PURPOSE_PHONE;
constexpr type URL = Gtk::INPUT_PURPOSE_URL;
constexpr type EMAIL = Gtk::INPUT_PURPOSE_EMAIL;
constexpr type NAME = Gtk::INPUT_PURPOSE_NAME;
constexpr type PASSWORD = Gtk::INPUT_PURPOSE_PASSWORD;
constexpr type PIN = Gtk::INPUT_PURPOSE_PIN;
#endif
} // InputPurpose

/**
 * @brief Gtk::Justification の alias
 */
namespace Justification {
using type = Gtk::Justification;

#ifdef USE_GTKMM4
using Gtk::Justification::LEFT;
using Gtk::Justification::RIGHT;
using Gtk::Justification::CENTER;
using Gtk::Justification::FILL;
#else
constexpr type LEFT = Gtk::JUSTIFY_LEFT;
constexpr type RIGHT = Gtk::JUSTIFY_RIGHT;
constexpr type CENTER = Gtk::JUSTIFY_CENTER;
constexpr type FILL = Gtk::JUSTIFY_FILL;
#endif
} // Justification

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
 * @brief Gtk::Orientation の alias
 *
 * @note 未使用またはコメントアウトされている enum 値は alias を作成していません。
 */
namespace Orientation {
using type = Gtk::Orientation;

#ifdef USE_GTKMM4
using Gtk::Orientation::HORIZONTAL;
using Gtk::Orientation::VERTICAL;
#else
constexpr type HORIZONTAL = Gtk::ORIENTATION_HORIZONTAL;
constexpr type VERTICAL = Gtk::ORIENTATION_VERTICAL;
#endif
} // Orientation

/**
 * @brief Gtk::PolicyType の alias
 */
namespace PolicyType {
using type = Gtk::PolicyType;

#ifdef USE_GTKMM4
using Gtk::PolicyType::ALWAYS;
using Gtk::PolicyType::AUTOMATIC;
using Gtk::PolicyType::NEVER;
using Gtk::PolicyType::EXTERNAL;
#else
constexpr type ALWAYS = Gtk::POLICY_ALWAYS;
constexpr type AUTOMATIC = Gtk::POLICY_AUTOMATIC;
constexpr type NEVER = Gtk::POLICY_NEVER;
constexpr type EXTERNAL = Gtk::POLICY_EXTERNAL;
#endif
} // PolicyType

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

/**
 * @brief Gtk::SelectionMode の alias
 */
namespace SelectionMode {
using type = Gtk::SelectionMode;

#ifdef USE_GTKMM4
using Gtk::SelectionMode::NONE;
using Gtk::SelectionMode::SINGLE;
using Gtk::SelectionMode::BROWSE;
using Gtk::SelectionMode::MULTIPLE;
#else
constexpr type NONE = Gtk::SELECTION_NONE;
constexpr type SINGLE = Gtk::SELECTION_SINGLE;
constexpr type BROWSE = Gtk::SELECTION_BROWSE;
constexpr type MULTIPLE = Gtk::SELECTION_MULTIPLE;
#endif
} // SelectionMode

/**
 * @brief Gtk::SortType の alias
 */
namespace SortType {
using type = Gtk::SortType;

#ifdef USE_GTKMM4
using Gtk::SortType::ASCENDING;
using Gtk::SortType::DESCENDING;
#else
constexpr type ASCENDING = Gtk::SORT_ASCENDING;
constexpr type DESCENDING = Gtk::SORT_DESCENDING;
#endif
} // SortType

/**
 * @brief Gtk::StateFlags の alias
 */
namespace StateFlags {
using type = Gtk::StateFlags;

#ifdef USE_GTKMM4
using Gtk::StateFlags::NORMAL;
using Gtk::StateFlags::ACTIVE;
using Gtk::StateFlags::PRELIGHT;
using Gtk::StateFlags::SELECTED;
using Gtk::StateFlags::INSENSITIVE;
using Gtk::StateFlags::INCONSISTENT;
using Gtk::StateFlags::FOCUSED;
using Gtk::StateFlags::BACKDROP;
using Gtk::StateFlags::DIR_LTR;
using Gtk::StateFlags::DIR_RTL;
using Gtk::StateFlags::LINK;
using Gtk::StateFlags::VISITED;
using Gtk::StateFlags::CHECKED;
using Gtk::StateFlags::DROP_ACTIVE;
using Gtk::StateFlags::FOCUS_VISIBLE;
using Gtk::StateFlags::FOCUS_WITHIN;
#else
constexpr type NORMAL = Gtk::STATE_FLAG_NORMAL;
constexpr type ACTIVE = Gtk::STATE_FLAG_ACTIVE;
constexpr type PRELIGHT = Gtk::STATE_FLAG_PRELIGHT;
constexpr type SELECTED = Gtk::STATE_FLAG_SELECTED;
constexpr type INSENSITIVE = Gtk::STATE_FLAG_INSENSITIVE;
constexpr type INCONSISTENT = Gtk::STATE_FLAG_INCONSISTENT;
constexpr type FOCUSED = Gtk::STATE_FLAG_FOCUSED;
constexpr type BACKDROP = Gtk::STATE_FLAG_BACKDROP;
constexpr type DIR_LTR = Gtk::STATE_FLAG_DIR_LTR;
constexpr type DIR_RTL = Gtk::STATE_FLAG_DIR_RTL;
constexpr type LINK = Gtk::STATE_FLAG_LINK;
constexpr type VISITED = Gtk::STATE_FLAG_VISITED;
constexpr type CHECKED = Gtk::STATE_FLAG_CHECKED;
constexpr type DROP_ACTIVE = Gtk::STATE_FLAG_DROP_ACTIVE;
#endif
} // StateFlags

/**
 * @brief Gtk::TextWindowType の alias
 */
namespace TextWindowType {
using type = Gtk::TextWindowType;

#ifdef USE_GTKMM4
using Gtk::TextWindowType::WIDGET;
using Gtk::TextWindowType::TEXT;
using Gtk::TextWindowType::LEFT;
using Gtk::TextWindowType::RIGHT;
using Gtk::TextWindowType::TOP;
using Gtk::TextWindowType::BOTTOM;
#else
constexpr type PRIVATE = Gtk::TEXT_WINDOW_PRIVATE;
constexpr type WIDGET = Gtk::TEXT_WINDOW_WIDGET;
constexpr type TEXT = Gtk::TEXT_WINDOW_TEXT;
constexpr type LEFT = Gtk::TEXT_WINDOW_LEFT;
constexpr type RIGHT = Gtk::TEXT_WINDOW_RIGHT;
constexpr type TOP = Gtk::TEXT_WINDOW_TOP;
constexpr type BOTTOM = Gtk::TEXT_WINDOW_BOTTOM;
#endif
} // TextWindowType

/**
 * @brief Gtk::TreeViewColumn::Sizing の alias
 */
namespace TreeViewColumn::Sizing {

#ifdef USE_GTKMM4
using type = Gtk::TreeViewColumn::Sizing;
using Gtk::TreeViewColumn::Sizing::GROW_ONLY;
using Gtk::TreeViewColumn::Sizing::AUTOSIZE;
using Gtk::TreeViewColumn::Sizing::FIXED;
#else
using type = Gtk::TreeViewColumnSizing;
constexpr type GROW_ONLY = Gtk::TREE_VIEW_COLUMN_GROW_ONLY;
constexpr type AUTOSIZE =  Gtk::TREE_VIEW_COLUMN_AUTOSIZE;
constexpr type FIXED =  Gtk::TREE_VIEW_COLUMN_FIXED;
#endif
} // TreeViewColumn::Sizing

/**
 * @brief Gtk::WrapMode の alias
 */
namespace WrapMode {
using type = Gtk::WrapMode;

#ifdef USE_GTKMM4
using Gtk::WrapMode::NONE;
using Gtk::WrapMode::CHAR;
using Gtk::WrapMode::WORD;
using Gtk::WrapMode::WORD_CHAR;
#else
constexpr type NONE = Gtk::WRAP_NONE;
constexpr type CHAR = Gtk::WRAP_CHAR;
constexpr type WORD = Gtk::WRAP_WORD;
constexpr type WORD_CHAR = Gtk::WRAP_WORD_CHAR;
#endif
} // WrapMode

} // JDLIB::compat

#endif // JDLIB_GTK_COMPAT_H
