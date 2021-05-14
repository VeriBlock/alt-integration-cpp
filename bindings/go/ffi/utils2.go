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
	v.mutex.AssertMutexLocked("PopContext is not locked")
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
	v.mutex.AssertMutexLocked("PopContext is not locked")
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

func (v *PopContext2) SaveAllTrees() error {
	v.mutex.AssertMutexLocked("PopContext is not locked")
	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	state := NewValidationState2()
	defer state.Free()

	C.pop_pop_context_function_save_all_trees(v.ref, state.ref)
	return state.Error()
}

func (v *PopContext2) LoadAllTrees() error {
	v.mutex.AssertMutexLocked("PopContext is not locked")
	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	state := NewValidationState2()
	defer state.Free()

	C.pop_pop_context_function_load_all_trees(v.ref, state.ref)
	return state.Error()
}

func (v *PopContext2) CheckAtv(atv *Atv) error {
	v.mutex.AssertMutexLocked("PopContext is not locked")
	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	if atv.ref == nil {
		panic("Atv does not initialized")
	}
	state := NewValidationState2()
	defer state.Free()

	C.pop_pop_context_function_check_atv(v.ref, atv.ref, state.ref)
	return state.Error()
}

func (v *PopContext2) CheckVtb(vtb *Vtb) error {
	v.mutex.AssertMutexLocked("PopContext is not locked")
	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	if vtb.ref == nil {
		panic("Vtb does not initialized")
	}
	state := NewValidationState2()
	defer state.Free()

	C.pop_pop_context_function_check_vtb(v.ref, vtb.ref, state.ref)
	return state.Error()
}

func (v *PopContext2) CheckVbkBlock(vbkBlock *VbkBlock) error {
	v.mutex.AssertMutexLocked("PopContext is not locked")
	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	if vbkBlock.ref == nil {
		panic("VbkBlock does not initialized")
	}
	state := NewValidationState2()
	defer state.Free()

	C.pop_pop_context_function_check_vbk_block(v.ref, vbkBlock.ref, state.ref)
	return state.Error()
}

func (v *PopContext2) CheckPopData(popData *PopData) error {
	v.mutex.AssertMutexLocked("PopContext is not locked")
	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	if popData.ref == nil {
		panic("PopData does not initialized")
	}
	state := NewValidationState2()
	defer state.Free()

	C.pop_pop_context_function_check_pop_data(v.ref, popData.ref, state.ref)
	return state.Error()
}
