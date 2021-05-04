// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/utils2.h>
import "C"

func (v *PopContext2) GeneratePublicationData(endorsedBlockHeader []byte, txRootHash []byte, payoutInfo []byte, popData *PopData) (*PublicationData, error) {
	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	if popData.ref == nil {
		panic("PopData does not initialized")
	}

	// C.pop_pop_context_function_generate_publication_data(createCBytes(endorsedBlockHeader), createCBytes(txRootHash), createCBytes(payoutInfo), popData.ref)

	return nil, nil
}
