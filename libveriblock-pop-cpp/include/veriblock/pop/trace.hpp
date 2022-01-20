// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_TRACE_HPP
#define VERIBLOCK_POP_CPP_TRACE_HPP

#ifdef TRACY_ENABLE
#ifdef WIN32
#pragma warning(push, 0)
#endif
#include "third_party/Tracy.hpp"
#ifdef WIN32
#pragma warning(pop)
#endif
#define VBK_TRACE_ZONE_SCOPED ZoneScoped
// suffix S = also collects stack information
#define VBK_TRACE_ZONE_SCOPED_S(x) ZoneScopedS(x)
#define VBK_TRACE_ZONE_SCOPED_N(name) ZoneScopedN(name)
#define VBK_TRACE_FRAME_START(name) FrameMarkStart(name)
#define VBK_TRACE_FRAME_END(name) FrameMarkEnd(name)
#define VBK_TRACE_LOCKABLE(type, var) TracyLockable(type, var)
#define VBK_TRACE_LOCKABLE_BASE(type) LockableBase(type)
#else
#define VBK_TRACE_ZONE_SCOPED
#define VBK_TRACE_ZONE_SCOPED_S(x)
#define VBK_TRACE_ZONE_SCOPED_N(name)
#define VBK_TRACE_FRAME_START(name)
#define VBK_TRACE_FRAME_END(name)
#define VBK_TRACE_LOCKABLE(type, var) type var
#define VBK_TRACE_LOCKABLE_BASE(type) type
#endif

#endif  // VERIBLOCK_POP_CPP_TRACE_HPP
