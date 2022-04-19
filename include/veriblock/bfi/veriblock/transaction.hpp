// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef BFI_VERIBLOCK_TRANSACTION_HPP
#define BFI_VERIBLOCK_TRANSACTION_HPP

#include <veriblock/pop/entities/vbktx.hpp>
#include <veriblock/pop/validation_state.hpp>

namespace altintegration {
namespace vbk {

struct ProtoOtput;
bool ConvertFromProto(const ProtoOtput& from,
                      Output& to,
                      ValidationState& state);

void ConvertToProto(const Output& from, ProtoOtput& to);

struct ProtoTransaction;
bool ConvertFromProto(const ProtoTransaction& from,
                      VbkTx& to,
                      ValidationState& state);

void ConvertToProto(const VbkTx& from, ProtoTransaction& to);

}  // namespace vbk
}  // namespace altintegration

#endif