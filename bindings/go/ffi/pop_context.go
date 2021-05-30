// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/pop_context.h>
import "C"
import "runtime"

type PopContext struct {
	ref   *C.pop_pop_context_t
	mutex *SafeMutex
}

func (v *PopContext) validate() {
	v.mutex.AssertMutexLocked("PopContext is not locked")
	if v.ref == nil {
		panic("PopContext does not initialized")
	}
}

func NewPopContext(config *Config, storage *Storage, log_lvl string) *PopContext {
	config.validate()
	storage.validate()
	context := &PopContext{
		ref:   C.pop_pop_context_new(config.ref, storage.ref, createCString(log_lvl)),
		mutex: NewSafeMutex(),
	}
	runtime.SetFinalizer(context, func(v *PopContext) {
		defer v.Lock()()
		v.Free()
	})
	return context
}

func (v *PopContext) Free() {
	v.mutex.AssertMutexLocked("PopContext is not locked")
	if v.ref != nil {
		C.pop_pop_context_free(v.ref)
		v.ref = nil
	}
}

func (v *PopContext) AcceptBlockHeader(block *AltBlock) error {
	v.validate()
	block.validate()
	state := NewValidationState()
	defer state.Free()
	C.pop_pop_context_function_accept_block_header(v.ref, block.ref, state.ref)
	return state.Error()
}

func (v *PopContext) AcceptBlock(hash []byte, popData *PopData) {
	v.validate()
	if popData == nil {
		popData = CreatePopData()
	}
	C.pop_pop_context_function_accept_block(v.ref, createCBytes(hash), popData.ref)
}

func (v *PopContext) SetState(hash []byte) error {
	v.validate()
	state := NewValidationState()
	defer state.Free()
	C.pop_pop_context_function_set_state(v.ref, createCBytes(hash), state.ref)
	return state.Error()
}

func (v *PopContext) ComparePopScore(A_hash []byte, B_hash []byte) int {
	v.validate()
	return int(C.pop_pop_context_function_compare_pop_score(v.ref, createCBytes(A_hash), createCBytes(B_hash)))
}

func (v *PopContext) GetPopPayouts(hash []byte) []*PopPayout {
	v.validate()
	array := C.pop_pop_context_function_get_pop_payouts(v.ref, createCBytes(hash))
	defer freeArrayPopPayout(&array)
	return createArrayPopPayout(&array)
}

func (v *PopContext) RemoveSubtree(hash []byte) {
	v.validate()
	C.pop_pop_context_function_remove_subtree(v.ref, createCBytes(hash))
}

func (v *PopContext) AltGetBlockIndex(hash []byte) *AltBlockIndex {
	v.validate()
	if res := C.pop_pop_context_function_alt_get_block_index(v.ref, createCBytes(hash)); res != nil {
		return createAltBlockIndex(res)
	}
	return nil
}

func (v *PopContext) VbkGetBlockIndex(hash []byte) *VbkBlockIndex {
	v.validate()
	if res := C.pop_pop_context_function_vbk_get_block_index(v.ref, createCBytes(hash)); res != nil {
		return createVbkBlockIndex(res)
	}
	return nil
}

func (v *PopContext) BtcGetBlockIndex(hash []byte) *BtcBlockIndex {
	v.validate()
	if res := C.pop_pop_context_function_btc_get_block_index(v.ref, createCBytes(hash)); res != nil {
		return createBtcBlockIndex(res)
	}
	return nil
}

func (v *PopContext) AltGetBestBlock() *AltBlockIndex {
	v.validate()
	return createAltBlockIndex(C.pop_pop_context_function_alt_get_best_block(v.ref))
}

func (v *PopContext) VbkGetBestBlock() *VbkBlockIndex {
	v.validate()
	return createVbkBlockIndex(C.pop_pop_context_function_vbk_get_best_block(v.ref))
}

func (v *PopContext) BtcGetBestBlock() *BtcBlockIndex {
	v.validate()
	return createBtcBlockIndex(C.pop_pop_context_function_btc_get_best_block(v.ref))
}

func (v *PopContext) AltGetBootstrapBlock() *AltBlockIndex {
	v.validate()
	return createAltBlockIndex(C.pop_pop_context_function_alt_get_bootstrap_block(v.ref))
}

func (v *PopContext) VbkGetBootstrapBlock() *VbkBlockIndex {
	v.validate()
	return createVbkBlockIndex(C.pop_pop_context_function_vbk_get_bootstrap_block(v.ref))
}

func (v *PopContext) BtcGetBootstrapBlock() *BtcBlockIndex {
	v.validate()
	return createBtcBlockIndex(C.pop_pop_context_function_btc_get_bootstrap_block(v.ref))
}

func (v *PopContext) AltGetBlockAtActiveChainByHeight(height uint32) *AltBlockIndex {
	v.validate()
	if res := C.pop_pop_context_function_alt_get_block_at_active_chain(v.ref, C.uint32_t(height)); res != nil {
		createAltBlockIndex(res)
	}
	return nil
}

func (v *PopContext) VbkGetBlockAtActiveChainByHeight(height uint32) *VbkBlockIndex {
	v.validate()
	if res := C.pop_pop_context_function_vbk_get_block_at_active_chain(v.ref, C.uint32_t(height)); res != nil {
		createVbkBlockIndex(res)
	}
	return nil
}

func (v *PopContext) BtcGetBlockAtActiveChainByHeight(height uint32) *BtcBlockIndex {
	v.validate()
	if res := C.pop_pop_context_function_btc_get_block_at_active_chain(v.ref, C.uint32_t(height)); res != nil {
		createBtcBlockIndex(res)
	}
	return nil
}

func (v *PopContext) GetPayloadContainingBlocks(id []byte) [][]byte {
	v.validate()
	array := C.pop_pop_context_function_get_payload_containing_blocks(v.ref, createCBytes(id))
	defer freeArrayArrayU8(&array)
	return createArrayOfArraysU8(&array)
}

func (v *PopContext) Lock() (unlock func()) {
	v.mutex.Lock()
	return func() {
		v.mutex.Unlock()
	}
}
