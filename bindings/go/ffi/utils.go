// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/utils.h>
import "C"
import "errors"

func (v *PopContext) GeneratePublicationData(endorsedBlockHeader []byte, txRootHash []byte, payoutInfo []byte, popData *PopData) (*PublicationData, error) {
	v.validate()
	if popData == nil {
		popData = CreatePopData()
	}

	res := C.pop_pop_context_function_generate_publication_data(v.ref, createCBytes(endorsedBlockHeader), createCBytes(txRootHash), createCBytes(payoutInfo), popData.ref)
	if res == nil {
		return nil, errors.New("cannot generate PublicationData")
	}

	return createPublicationData(res), nil
}

func (v *PopContext) CalculateTopLevelMerkleRoot(txRoot []byte, prevBlockHash []byte, popData *PopData) []byte {
	v.validate()
	popData.validate()
	res := C.pop_pop_context_function_calculate_top_level_merkle_root(v.ref, createCBytes(txRoot), createCBytes(prevBlockHash), popData.ref)
	defer freeArrayU8(&res)
	return createBytes(&res)
}

func (v *PopContext) SaveAllTrees() error {
	v.validate()
	state := NewValidationState()
	defer state.Free()

	C.pop_pop_context_function_save_all_trees(v.ref, state.ref)
	return state.Error()
}

func (v *PopContext) LoadAllTrees() error {
	v.validate()
	state := NewValidationState()
	defer state.Free()

	C.pop_pop_context_function_load_all_trees(v.ref, state.ref)
	return state.Error()
}

func (v *PopContext) CheckAtv(atv *Atv) error {
	v.validate()
	if atv == nil {
		return nil
	}
	atv.validate()
	state := NewValidationState()
	defer state.Free()

	C.pop_pop_context_function_check_atv(v.ref, atv.ref, state.ref)
	return state.Error()
}

func (v *PopContext) CheckVtb(vtb *Vtb) error {
	v.validate()
	if vtb == nil {
		return nil
	}
	vtb.validate()
	state := NewValidationState()
	defer state.Free()

	C.pop_pop_context_function_check_vtb(v.ref, vtb.ref, state.ref)
	return state.Error()
}

func (v *PopContext) CheckVbkBlock(vbkBlock *VbkBlock) error {
	v.validate()
	if vbkBlock == nil {
		return nil
	}
	vbkBlock.validate()
	state := NewValidationState()
	defer state.Free()

	C.pop_pop_context_function_check_vbk_block(v.ref, vbkBlock.ref, state.ref)
	return state.Error()
}

func (v *PopContext) CheckPopData(popData *PopData) error {
	v.validate()
	if popData == nil {
		return nil
	}
	popData.validate()
	state := NewValidationState()
	defer state.Free()

	C.pop_pop_context_function_check_pop_data(v.ref, popData.ref, state.ref)
	return state.Error()
}
