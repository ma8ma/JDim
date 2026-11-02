// SPDX-License-Identifier: GPL-2.0-or-later
/** @file gtk_compat.h
 *
 * @details GTK3 と GTK4 の両方で動くコードを書くための互換性ライブラリ
 */
#ifndef JDLIB_GTK_COMPAT_H
#define JDLIB_GTK_COMPAT_H

#include <giomm/application.h>
#include <glibmm/binding.h>
#include <glibmm/fileutils.h>
#include <glibmm/iochannel.h>
#include <gtkmm/box.h>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/filechooser.h>
#include <gtkmm/gesture.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/textview.h>
#include <gtkmm/treeviewcolumn.h>
#include <pangomm/fontdescription.h>
#include <pangomm/layout.h>


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

/**
 * @brief Gdk::DragAction の alias
 */
namespace DragAction {
using type = Gdk::DragAction;
#ifdef USE_GTKMM4
using Gdk::DragAction::COPY;
using Gdk::DragAction::MOVE;
using Gdk::DragAction::LINK;
using Gdk::DragAction::ASK;
#else
constexpr type COPY = Gdk::ACTION_COPY;
constexpr type MOVE = Gdk::ACTION_MOVE;
constexpr type LINK = Gdk::ACTION_LINK;
constexpr type ASK = Gdk::ACTION_ASK;
#endif
} // DragAction

/**
 * @brief Gdk::AnchorHints の alias
 */
namespace AnchorHints {
using type = Gdk::AnchorHints;
#ifdef USE_GTKMM4
using Gdk::AnchorHints::FLIP_X;
using Gdk::AnchorHints::FLIP_Y;
using Gdk::AnchorHints::SLIDE_X;
using Gdk::AnchorHints::SLIDE_Y;
using Gdk::AnchorHints::RESIZE_X;
using Gdk::AnchorHints::RESIZE_Y;
using Gdk::AnchorHints::FLIP;
using Gdk::AnchorHints::SLIDE;
using Gdk::AnchorHints::RESIZE;
#else
constexpr type FLIP_X = Gdk::ANCHOR_FLIP_X;
constexpr type FLIP_Y = Gdk::ANCHOR_FLIP_Y;
constexpr type SLIDE_X = Gdk::ANCHOR_SLIDE_X;
constexpr type SLIDE_Y = Gdk::ANCHOR_SLIDE_Y;
constexpr type RESIZE_X = Gdk::ANCHOR_RESIZE_X;
constexpr type RESIZE_Y = Gdk::ANCHOR_RESIZE_Y;
constexpr type FLIP = Gdk::ANCHOR_FLIP;
constexpr type SLIDE = Gdk::ANCHOR_SLIDE;
constexpr type RESIZE = Gdk::ANCHOR_RESIZE;
#endif
} // AnchorHints

/**
 * @brief Gdk::Colorspace の alias
 */
namespace Colorspace {
using type = Gdk::Colorspace;
#ifdef USE_GTKMM4
using Gdk::Colorspace::RGB;
#else
constexpr type RGB = Gdk::COLORSPACE_RGB;
#endif
} // Colorspace

/**
 * @brief Gdk::Gravity の alias
 */
namespace Gravity {
using type = Gdk::Gravity;
#ifdef USE_GTKMM4
using Gdk::Gravity::NORTH_WEST;
using Gdk::Gravity::NORTH;
using Gdk::Gravity::NORTH_EAST;
using Gdk::Gravity::WEST;
using Gdk::Gravity::CENTER;
using Gdk::Gravity::EAST;
using Gdk::Gravity::SOUTH_WEST;
using Gdk::Gravity::SOUTH;
using Gdk::Gravity::SOUTH_EAST;
using Gdk::Gravity::STATIC;
#else
constexpr type NORTH_WEST = Gdk::GRAVITY_NORTH_WEST;
constexpr type NORTH = Gdk::GRAVITY_NORTH;
constexpr type NORTH_EAST = Gdk::GRAVITY_NORTH_EAST;
constexpr type WEST = Gdk::GRAVITY_WEST;
constexpr type CENTER = Gdk::GRAVITY_CENTER;
constexpr type EAST = Gdk::GRAVITY_EAST;
constexpr type SOUTH_WEST = Gdk::GRAVITY_SOUTH_WEST;
constexpr type SOUTH = Gdk::GRAVITY_SOUTH;
constexpr type SOUTH_EAST = Gdk::GRAVITY_SOUTH_EAST;
constexpr type STATIC = Gdk::GRAVITY_STATIC;
#endif
} // Gravity

/**
 * @brief Gdk::InterpType の alias
 */
namespace InterpType {
using type = Gdk::InterpType;
#ifdef USE_GTKMM4
using Gdk::InterpType::NEAREST;
using Gdk::InterpType::TILES;
using Gdk::InterpType::BILINEAR;
using Gdk::InterpType::HYPER;
#else
constexpr type NEAREST = Gdk::INTERP_NEAREST;
constexpr type TILES = Gdk::INTERP_TILES;
constexpr type BILINEAR = Gdk::INTERP_BILINEAR;
constexpr type HYPER = Gdk::INTERP_HYPER;
#endif
} // InterpType

/**
 * @brief Gdk::InputSource の alias
 */
namespace InputSource {
using type = Gdk::InputSource;
#ifdef USE_GTKMM4
using Gdk::InputSource::MOUSE;
using Gdk::InputSource::PEN;
using Gdk::InputSource::KEYBOARD;
using Gdk::InputSource::TOUCHSCREEN;
using Gdk::InputSource::TOUCHPAD;
using Gdk::InputSource::TRACKPOINT;
using Gdk::InputSource::TABLET_PAD;
#else
constexpr type MOUSE = Gdk::SOURCE_MOUSE;
constexpr type PEN = Gdk::SOURCE_PEN;
constexpr type KEYBOARD = Gdk::SOURCE_KEYBOARD;
constexpr type TOUCHSCREEN = Gdk::SOURCE_TOUCHSCREEN;
constexpr type TOUCHPAD = Gdk::SOURCE_TOUCHPAD;
constexpr type TRACKPOINT = Gdk::SOURCE_TRACKPOINT;
constexpr type TABLET_PAD = Gdk::SOURCE_TABLET_PAD;
#endif
} // InputSource

/**
 * @brief Glib::Binding::Flags の alias
 */
namespace Binding::Flags {
#ifdef USE_GTKMM4
using type = Glib::Binding::Flags;
using Glib::Binding::Flags::DEFAULT;
using Glib::Binding::Flags::BIDIRECTIONAL;
using Glib::Binding::Flags::SYNC_CREATE;
using Glib::Binding::Flags::INVERT_BOOLEAN;
#else
using type = Glib::BindingFlags;
constexpr type DEFAULT = Glib::BINDING_DEFAULT;
constexpr type BIDIRECTIONAL = Glib::BINDING_BIDIRECTIONAL;
constexpr type SYNC_CREATE = Glib::BINDING_SYNC_CREATE;
constexpr type INVERT_BOOLEAN = Glib::BINDING_INVERT_BOOLEAN;
#endif
} // Binding::Flags

/**
 * @brief Glib::FileTest の alias
 */
namespace FileTest {
using type = Glib::FileTest;
#ifdef USE_GTKMM4
using Glib::FileTest::IS_REGULAR;
using Glib::FileTest::IS_SYMLINK;
using Glib::FileTest::IS_DIR;
using Glib::FileTest::IS_EXECUTABLE;
using Glib::FileTest::EXISTS;
#else
constexpr type IS_REGULAR = Glib::FILE_TEST_IS_REGULAR;
constexpr type IS_SYMLINK = Glib::FILE_TEST_IS_SYMLINK;
constexpr type IS_DIR = Glib::FILE_TEST_IS_DIR;
constexpr type IS_EXECUTABLE = Glib::FILE_TEST_IS_EXECUTABLE;
constexpr type EXISTS = Glib::FILE_TEST_EXISTS;
#endif
} // FileTest

/**
 * @brief Glib::IOCondition の alias
 */
namespace IOCondition {
using type = Glib::IOCondition;
#ifdef USE_GTKMM4
using Glib::IOCondition::IO_IN;
using Glib::IOCondition::IO_OUT;
using Glib::IOCondition::IO_PRI;
using Glib::IOCondition::IO_ERR;
using Glib::IOCondition::IO_HUP;
using Glib::IOCondition::IO_NVAL;
#else
constexpr type IO_IN = Glib::IO_IN;
constexpr type IO_OUT = Glib::IO_OUT;
constexpr type IO_PRI = Glib::IO_PRI;
constexpr type IO_ERR = Glib::IO_ERR;
constexpr type IO_HUP = Glib::IO_HUP;
constexpr type IO_NVAL = Glib::IO_NVAL;
#endif
} // IOCondition

/**
 * @brief Glib::IOStatus の alias
 */
namespace IOStatus {
using type = Glib::IOStatus;
#ifdef USE_GTKMM4
using Glib::IOStatus::ERROR;
using Glib::IOStatus::NORMAL;
using Glib::IOStatus::ENDOFFILE;
using Glib::IOStatus::AGAIN;
#else
constexpr type ERROR = Glib::IO_STATUS_ERROR;
constexpr type NORMAL = Glib::IO_STATUS_NORMAL;
constexpr type ENDOFFILE = Glib::IO_STATUS_EOF;
constexpr type AGAIN = Glib::IO_STATUS_AGAIN;
#endif
} // IOStatus

/**
 * @brief Gio::Application::Flags の alias
 */
namespace Application::Flags {
#ifdef USE_GTKMM4
using type = Gio::Application::Flags;
using Gio::Application::Flags::NONE;
using Gio::Application::Flags::IS_SERVICE;
using Gio::Application::Flags::IS_LAUNCHER;
using Gio::Application::Flags::HANDLES_OPEN;
using Gio::Application::Flags::HANDLES_COMMAND_LINE;
using Gio::Application::Flags::SEND_ENVIRONMENT;
using Gio::Application::Flags::NON_UNIQUE;
using Gio::Application::Flags::CAN_OVERRIDE_APP_ID;
using Gio::Application::Flags::ALLOW_REPLACEMENT;
using Gio::Application::Flags::REPLACE;
#else
using type = Gio::ApplicationFlags;
constexpr type NONE = Gio::APPLICATION_FLAGS_NONE;
constexpr type IS_SERVICE = Gio::APPLICATION_IS_SERVICE;
constexpr type IS_LAUNCHER = Gio::APPLICATION_IS_LAUNCHER;
constexpr type HANDLES_OPEN = Gio::APPLICATION_HANDLES_OPEN;
constexpr type HANDLES_COMMAND_LINE = Gio::APPLICATION_HANDLES_COMMAND_LINE;
constexpr type SEND_ENVIRONMENT = Gio::APPLICATION_SEND_ENVIRONMENT;
constexpr type NON_UNIQUE = Gio::APPLICATION_NON_UNIQUE;
constexpr type CAN_OVERRIDE_APP_ID = Gio::APPLICATION_CAN_OVERRIDE_APP_ID;
constexpr type ALLOW_REPLACEMENT = Gio::APPLICATION_ALLOW_REPLACEMENT;
constexpr type REPLACE = Gio::APPLICATION_REPLACE;
#endif
} // Application::Flags

/**
 * @brief Pango::EllipsizeMode の alias
 */
namespace EllipsizeMode {
using type = Pango::EllipsizeMode;
#ifdef USE_GTKMM4
using Pango::EllipsizeMode::NONE;
using Pango::EllipsizeMode::START;
using Pango::EllipsizeMode::MIDDLE;
using Pango::EllipsizeMode::END;
#else
constexpr type NONE = Pango::ELLIPSIZE_NONE;
constexpr type START = Pango::ELLIPSIZE_START;
constexpr type MIDDLE = Pango::ELLIPSIZE_MIDDLE;
constexpr type END = Pango::ELLIPSIZE_END;
#endif
} // EllipsizeMode

/**
 * @brief Pango::Underline の alias
 */
namespace Underline {
using type = Pango::Underline;
#ifdef USE_GTKMM4
using Pango::Underline::NONE;
using Pango::Underline::SINGLE;
using Pango::Underline::DOUBLE;
using Pango::Underline::LOW;
using Pango::Underline::ERROR;
using Pango::Underline::SINGLE_LINE;
using Pango::Underline::DOUBLE_LINE;
using Pango::Underline::ERROR_LINE;
#else
constexpr type NONE = Pango::UNDERLINE_NONE;
constexpr type SINGLE = Pango::UNDERLINE_SINGLE;
constexpr type DOUBLE = Pango::UNDERLINE_DOUBLE;
constexpr type LOW = Pango::UNDERLINE_LOW;
constexpr type ERROR = Pango::UNDERLINE_ERROR;
constexpr type SINGLE_LINE = Pango::UNDERLINE_SINGLE_LINE;
constexpr type DOUBLE_LINE = Pango::UNDERLINE_DOUBLE_LINE;
constexpr type ERROR_LINE = Pango::UNDERLINE_ERROR_LINE;
#endif
} // Underline

/**
 * @brief Pango::Weight の alias
 */
namespace Weight {
using type = Pango::Weight;
#ifdef USE_GTKMM4
using Pango::Weight::THIN;
using Pango::Weight::ULTRALIGHT;
using Pango::Weight::LIGHT;
using Pango::Weight::SEMILIGHT;
using Pango::Weight::BOOK;
using Pango::Weight::NORMAL;
using Pango::Weight::MEDIUM;
using Pango::Weight::SEMIBOLD;
using Pango::Weight::BOLD;
using Pango::Weight::ULTRABOLD;
using Pango::Weight::HEAVY;
using Pango::Weight::ULTRAHEAVY;
#else
constexpr type THIN = Pango::WEIGHT_THIN;
constexpr type ULTRALIGHT = Pango::WEIGHT_ULTRALIGHT;
constexpr type LIGHT = Pango::WEIGHT_LIGHT;
constexpr type SEMILIGHT = Pango::WEIGHT_SEMILIGHT;
constexpr type BOOK = Pango::WEIGHT_BOOK;
constexpr type NORMAL = Pango::WEIGHT_NORMAL;
constexpr type MEDIUM = Pango::WEIGHT_MEDIUM;
constexpr type SEMIBOLD = Pango::WEIGHT_SEMIBOLD;
constexpr type BOLD = Pango::WEIGHT_BOLD;
constexpr type ULTRABOLD = Pango::WEIGHT_ULTRABOLD;
constexpr type HEAVY = Pango::WEIGHT_HEAVY;
constexpr type ULTRAHEAVY = Pango::WEIGHT_ULTRAHEAVY;
#endif
} // Weight

} // JDLIB::compat

#endif // JDLIB_GTK_COMPAT_H
