// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/pop_context2.h>
import "C"
import "runtime"

type PopContext2 struct {
	ref   *C.pop_pop_context_t
	mutex *SafeMutex
}

func NewPopContext2(config *Config2, storage *Storage2, log_lvl string) *PopContext2 {
	if config == nil {
		panic("Config not provided")
	}
	if storage == nil {
		panic("Storage not provided")
	}

	context := &PopContext2{
		ref:   C.pop_pop_context_new(config.ref, storage.ref, createCString(log_lvl)),
		mutex: NewSafeMutex(),
	}
	runtime.SetFinalizer(context, func(v *PopContext2) {
		defer v.Lock()()
		v.Free()
	})
	return context
}

func (v *PopContext2) Free() {
	v.mutex.AssertMutexLocked("PopContext is not locked")

	if v.ref != nil {
		C.pop_pop_context_free(v.ref)
		v.ref = nil
	}
}

func (v *PopContext2) AcceptBlockHeader(block *AltBlock) error {
	v.mutex.AssertMutexLocked("PopContext is not locked")

	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	state := NewValidationState2()
	defer state.Free()

	C.pop_pop_context_function_accept_block_header(v.ref, block.ref, state.ref)
	return state.Error()
}

func (v *PopContext2) AcceptBlock(hash []byte, popData *PopData) {
	v.mutex.AssertMutexLocked("PopContext is not locked")

	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	if popData.ref == nil {
		panic("PopData does not initialized")
	}
	C.pop_pop_context_function_accept_block(v.ref, createCBytes(hash), popData.ref)
}

func (v *PopContext2) SetState(hash []byte) error {
	v.mutex.AssertMutexLocked("PopContext is not locked")

	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	state := NewValidationState2()
	defer state.Free()

	C.pop_pop_context_function_set_state(v.ref, createCBytes(hash), state.ref)
	return state.Error()
}

func (v *PopContext2) ComparePopScore(A_hash []byte, B_hash []byte) int {
	v.mutex.AssertMutexLocked("PopContext is not locked")

	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	return int(C.pop_pop_context_function_compare_pop_score(v.ref, createCBytes(A_hash), createCBytes(B_hash)))
}

func (v *PopContext2) GetPopPayouts(hash []byte) []*PopPayout {
	v.mutex.AssertMutexLocked("PopContext is not locked")

	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	array := C.pop_pop_context_function_get_pop_payouts(v.ref, createCBytes(hash))
	defer freeArrayPopPayout(&array)
	return createArrayPopPayout(&array)
}

func (v *PopContext2) RemoveSubtree(hash []byte) {
	v.mutex.AssertMutexLocked("PopContext is not locked")

	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	C.pop_pop_context_function_remove_subtree(v.ref, createCBytes(hash))
}

func (v *PopContext2) AltGetBlockIndex(hash []byte) *AltBlockIndex {
	v.mutex.AssertMutexLocked("PopContext is not locked")

	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	if res := C.pop_pop_context_function_alt_get_block_index(v.ref, createCBytes(hash)); res != nil {
		createAltBlockIndex(res)
	}
	return nil
}

func (v *PopContext2) VbkGetBlockIndex(hash []byte) *VbkBlockIndex {
	v.mutex.AssertMutexLocked("PopContext is not locked")

	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	if res := C.pop_pop_context_function_vbk_get_block_index(v.ref, createCBytes(hash)); res != nil {
		createVbkBlockIndex(res)
	}
	return nil
}

func (v *PopContext2) BtcGetBlockIndex(hash []byte) *BtcBlockIndex {
	v.mutex.AssertMutexLocked("PopContext is not locked")

	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	if res := C.pop_pop_context_function_btc_get_block_index(v.ref, createCBytes(hash)); res != nil {
		createBtcBlockIndex(res)
	}
	return nil
}

func (v *PopContext2) AltGetBestBlock() *AltBlockIndex {
	v.mutex.AssertMutexLocked("PopContext is not locked")

	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	return createAltBlockIndex(C.pop_pop_context_function_alt_get_best_block(v.ref))
}

func (v *PopContext2) VbkGetBestBlock() *VbkBlockIndex {
	v.mutex.AssertMutexLocked("PopContext is not locked")

	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	return createVbkBlockIndex(C.pop_pop_context_function_vbk_get_best_block(v.ref))
}

func (v *PopContext2) BtcGetBestBlock() *BtcBlockIndex {
	v.mutex.AssertMutexLocked("PopContext is not locked")

	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	return createBtcBlockIndex(C.pop_pop_context_function_btc_get_best_block(v.ref))
}

func (v *PopContext2) AltGetBootstrapBlock() *AltBlockIndex {
	v.mutex.AssertMutexLocked("PopContext is not locked")

	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	return createAltBlockIndex(C.pop_pop_context_function_alt_get_bootstrap_block(v.ref))
}

func (v *PopContext2) VbkGetBootstrapBlock() *VbkBlockIndex {
	v.mutex.AssertMutexLocked("PopContext is not locked")

	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	return createVbkBlockIndex(C.pop_pop_context_function_vbk_get_bootstrap_block(v.ref))
}

func (v *PopContext2) BtcGetBootstrapBlock() *BtcBlockIndex {
	v.mutex.AssertMutexLocked("PopContext is not locked")

	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	return createBtcBlockIndex(C.pop_pop_context_function_btc_get_bootstrap_block(v.ref))
}

func (v *PopContext2) AltGetBlockAtActiveChainByHeight(height uint32) *AltBlockIndex {
	v.mutex.AssertMutexLocked("PopContext is not locked")

	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	if res := C.pop_pop_context_function_alt_get_block_at_active_chain(v.ref, C.uint32_t(height)); res != nil {
		createAltBlockIndex(res)
	}
	return nil
}

func (v *PopContext2) VbkGetBlockAtActiveChainByHeight(height uint32) *VbkBlockIndex {
	v.mutex.AssertMutexLocked("PopContext is not locked")

	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	if res := C.pop_pop_context_function_vbk_get_block_at_active_chain(v.ref, C.uint32_t(height)); res != nil {
		createVbkBlockIndex(res)
	}
	return nil
}

func (v *PopContext2) BtcGetBlockAtActiveChainByHeight(height uint32) *BtcBlockIndex {
	v.mutex.AssertMutexLocked("PopContext is not locked")

	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	if res := C.pop_pop_context_function_btc_get_block_at_active_chain(v.ref, C.uint32_t(height)); res != nil {
		createBtcBlockIndex(res)
	}
	return nil
}

func (v *PopContext2) Lock() (unlock func()) {
	v.mutex.Lock()
	return func() {
		v.mutex.Unlock()
	}
}
