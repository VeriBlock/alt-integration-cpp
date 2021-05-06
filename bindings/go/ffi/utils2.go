// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/utils2.h>
import "C"
import "errors"

func (v *PopContext2) GeneratePublicationData(endorsedBlockHeader []byte, txRootHash []byte, payoutInfo []byte, popData *PopData) (*PublicationData, error) {
	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	if popData.ref == nil {
		panic("PopData does not initialized")
	}

	res := C.pop_pop_context_function_generate_publication_data(v.ref, createCBytes(endorsedBlockHeader), createCBytes(txRootHash), createCBytes(payoutInfo), popData.ref)
	if res == nil {
		return nil, errors.New("cannot generate PublicationData")
	}

	return createPublicationData(res), nil
}

func (v *PopContext2) CalculateTopLevelMerkleRoot(txRoot []byte, prevBlockHash []byte, popData *PopData) []byte {
	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	if popData.ref == nil {
		panic("PopData does not initialized")
	}

	res := C.pop_pop_context_function_calculate_top_level_merkle_root(v.ref, createCBytes(txRoot), createCBytes(prevBlockHash), popData.ref)
	defer freeArrayU8(&res)
	return createBytes(&res)
}
