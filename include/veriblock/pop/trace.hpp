// Copyright (c) 2019-2021 Xenios SEZC
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
#define VBK_TRACE_ZONE_SCOPED_N(name) ZoneScopedN(name)
#else
#define VBK_TRACE_ZONE_SCOPED
#define VBK_TRACE_ZONE_SCOPED_N(name)
#endif

#endif  // VERIBLOCK_POP_CPP_TRACE_HPP
