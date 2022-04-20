// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef BFI_VERIBLOCK_TRANSACTION_HPP
#define BFI_VERIBLOCK_TRANSACTION_HPP

#include <veriblock/pop/entities/vbkpoptx.hpp>
#include <veriblock/pop/entities/vbktx.hpp>
#include <veriblock/pop/validation_state.hpp>

namespace core {
// forward declaration
class RpcOutput;
class RpcSignedTransaction;
class RpcSignedTransaction;
}  // namespace core

namespace altintegration {
namespace vbk {

bool ConvertFromProto(const core::RpcOutput& from,
                      Output& to,
                      ValidationState& state);

bool ConvertFromProto(const core::RpcSignedTransaction& from,
                      VbkTx& to,
                      ValidationState& state);
bool ConvertFromProto(const core::RpcSignedTransaction& from,
                      VbkPopTx& to,
                      ValidationState& state);

}  // namespace vbk
}  // namespace altintegration

#endif