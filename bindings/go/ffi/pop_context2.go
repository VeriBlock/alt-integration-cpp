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
	ref *C.pop_pop_context_t
}

func NewPopContext2(config *Config2, storage *Storage2, log_lvl string) *PopContext2 {
	if config == nil {
		panic("Config not provided")
	}
	if storage == nil {
		panic("Storage not provided")
	}

	context := &PopContext2{
		ref: C.pop_pop_context_new(config.ref, storage.ref, createCString(log_lvl)),
	}
	runtime.SetFinalizer(context, func(v *PopContext2) {
		v.Free()
	})
	return context
}

func (v *PopContext2) Free() {
	if v.ref != nil {
		C.pop_pop_context_free(v.ref)
		v.ref = nil
	}
}

func (v *PopContext2) AltBlockTreeAcceptBlockHeader(block *AltBlock) (bool, error) {
	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	state := NewValidationState2()
	defer state.Free()

	res := C.pop_pop_context_function_accept_block_header(v.ref, block.ref, state.ref)

	return bool(res), state.Error()
}

func (v *PopContext2) MemPoolSubmitVbk(vbk_block *VbkBlock) (int, error) {
	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	state := NewValidationState2()
	defer state.Free()

	res := C.pop_pop_context_function_submit_vbk(v.ref, vbk_block.ref, state.ref)

	return int(res), state.Error()
}

func (v *PopContext2) MemPoolSubmitVtb(vtb *Vtb) (int, error) {
	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	state := NewValidationState2()
	defer state.Free()

	res := C.pop_pop_context_function_submit_vtb(v.ref, vtb.ref, state.ref)

	return int(res), state.Error()
}

func (v *PopContext2) MemPoolSubmitAtv(atv *Atv) (int, error) {
	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	state := NewValidationState2()
	defer state.Free()

	res := C.pop_pop_context_function_submit_atv(v.ref, atv.ref, state.ref)

	return int(res), state.Error()
}
