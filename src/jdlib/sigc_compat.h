// SPDX-License-Identifier: GPL-2.0-or-later
/** @file sigc_compat.h
 *
 * @details GTK3 と GTK4 の両方で動くコードを書くための sigc 互換性ライブラリ
 */
#ifndef JDLIB_SIGC_COMPAT_H
#define JDLIB_SIGC_COMPAT_H

#include <sigc++/functors/slot.h>
#include <sigc++/signal.h>


namespace JDLIB::compat {

#ifdef USE_GTKMM4

/**
 * @brief sigc::slot の互換レイヤー
 */
template<typename Signature>
using slot = sigc::slot<Signature>;

/**
 * @brief sigc::signal の互換レイヤー
 */
template<typename Signature>
using signal = sigc::signal<Signature>;

#else

namespace detail {

template<typename Signature>
struct slot_impl;

template<typename Signature>
struct signal_impl;

template<typename R, typename... Args>
struct slot_impl<R(Args...)>
{
    using type = sigc::slot<R, Args...>;
};

template<typename R, typename... Args>
struct signal_impl<R(Args...)>
{
    using type = sigc::signal<R, Args...>;
};

} // detail

/**
 * @brief sigc::slot の互換レイヤー
 */
template<typename Signature>
using slot = typename detail::slot_impl<Signature>::type;

/**
 * @brief sigc::signal の互換レイヤー
 */
template<typename Signature>
using signal = typename detail::signal_impl<Signature>::type;

#endif // USE_GTKMM4

} // JDLIB::compat

#endif // JDLIB_SIGC_COMPAT_H
