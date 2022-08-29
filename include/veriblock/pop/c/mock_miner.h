// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_MOCK_MINER_H
#define VERIBLOCK_POP_CPP_C_MOCK_MINER_H

#include "veriblock/pop/c/entities/atv.h"
#include "veriblock/pop/c/entities/btcblock.h"
#include "veriblock/pop/c/entities/publication_data.h"
#include "veriblock/pop/c/entities/vbkblock.h"
#include "veriblock/pop/c/entities/vtb.h"
#include "veriblock/pop/c/type_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

POP_DECLARE_ENTITY(mock_miner);

POP_ENTITY_NEW_FUNCTION(mock_miner);

/**
 * Mine new altintegration::BtcBlock on the top of the provided block.
 *
 * @param[in] self MockMiner.
 * @param[in] POP_ENTITY_NAME(btc_block) pointer to to the provided block.
 * @return POP_ENTITY_NAME(btc_block) pointer to the mined
 * altintegration::BtcBlock.
 */
POP_ENTITY_CUSTOM_FUNCTION(mock_miner,
                           POP_ENTITY_NAME(btc_block) *,
                           mineBtcBlock,
                           const POP_ENTITY_NAME(btc_block) * tip);

/**
 * Mine new altintegration::BtcBlock on the top of the current btctree.
 *
 * @param[in] self MockMiner.
 * @return POP_ENTITY_NAME(btc_block) pointer to the mined
 * altintegration::BtcBlock.
 */
POP_ENTITY_CUSTOM_FUNCTION(mock_miner,
                           POP_ENTITY_NAME(btc_block) *,
                           mineBtcBlockTip);

/**
 * Mine new altintegration::VbkBlock on the top of the provided block.
 *
 * @param[in] self MockMiner.
 * @param[in] POP_ENTITY_NAME(vbk_block) pointer to to the provided block.
 * @return POP_ENTITY_NAME(vbk_block) pointer to the mined
 * altintegration::VbkBlock.
 */
POP_ENTITY_CUSTOM_FUNCTION(mock_miner,
                           POP_ENTITY_NAME(vbk_block) *,
                           mineVbkBlock,
                           const POP_ENTITY_NAME(vbk_block) * tip);

/**
 * Mine new altintegration::VbkBlock on the top of the current btctree.
 *
 * @param[in] self MockMiner.
 * @return POP_ENTITY_NAME(vbk_block) pointer to the mined
 * altintegration::VbkBlock.
 */
POP_ENTITY_CUSTOM_FUNCTION(mock_miner,
                           POP_ENTITY_NAME(vbk_block) *,
                           mineVbkBlockTip);

/**
 * Mine new altintegration::ATV.
 *
 * @param[in] self MockMiner.
 * @param[in] pub_data POP_ENTITY_NAME(publication_data) pointer to the
 * altintegration::PublicationData
 * @return POP_ENTITY_NAME(atv) pointer to the mined
 * altintegration::ATV.
 */
POP_ENTITY_CUSTOM_FUNCTION(mock_miner,
                           POP_ENTITY_NAME(atv) *,
                           mineATV,
                           const POP_ENTITY_NAME(publication_data) * pub_data);

/**
 * Mine new altintegration::VTB.
 *
 * @param[in] self MockMiner.
 * @param[in] endorsed_block POP_ENTITY_NAME(vbk_block) pointer to the
 * altintegration::VbkBlock.
 * @param[in] last_known_btc_block POP_ENTITY_NAME(btc_block) pointer to the
 * altintegration::BtcBlock.
 * @return POP_ENTITY_NAME(vtb) pointer to the mined altintegration::VTB.
 */
POP_ENTITY_CUSTOM_FUNCTION(mock_miner,
                           POP_ENTITY_NAME(vtb) *,
                           mineVTB,
                           const POP_ENTITY_NAME(vbk_block) * endorsed_block,
                           const POP_ENTITY_NAME(btc_block) *
                               last_known_btc_block);

#ifdef __cplusplus
}  // end of extern "C"
#endif

#endif
