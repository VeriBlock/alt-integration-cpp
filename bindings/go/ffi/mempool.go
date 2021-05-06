// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/mempool.h>
import "C"

func (v *PopContext2) MemPoolSubmitVbk(vbk_block *VbkBlock) (int, error) {
	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	state := NewValidationState2()
	defer state.Free()

	res := C.pop_pop_context_function_mempool_submit_vbk(v.ref, vbk_block.ref, state.ref)

	return int(res), state.Error()
}

func (v *PopContext2) MemPoolSubmitVtb(vtb *Vtb) (int, error) {
	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	state := NewValidationState2()
	defer state.Free()

	res := C.pop_pop_context_function_mempool_submit_vtb(v.ref, vtb.ref, state.ref)

	return int(res), state.Error()
}

func (v *PopContext2) MemPoolSubmitAtv(atv *Atv) (int, error) {
	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	state := NewValidationState2()
	defer state.Free()

	res := C.pop_pop_context_function_mempool_submit_atv(v.ref, atv.ref, state.ref)

	return int(res), state.Error()
}

func (v *PopContext2) MemPoolGetAtvs() []*Atv {
	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	array := C.pop_pop_context_function_mempool_get_atvs(v.ref)
	defer freeArrayAtv(&array)
	return createArrayAtv(&array)
}

func (v *PopContext2) MemPoolGetVtbs() []*Vtb {
	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	array := C.pop_pop_context_function_mempool_get_vtbs(v.ref)
	defer freeArrayVtb(&array)
	return createArrayVtb(&array)
}

func (v *PopContext2) MemPoolGetVbkBlocks() []*VbkBlock {
	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	array := C.pop_pop_context_function_mempool_get_vbk_blocks(v.ref)
	defer freeArrayVbkBlock(&array)
	return createArrayVbkBlock(&array)
}

func (v *PopContext2) MemPoolGetAtvsInFlight() []*Atv {
	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	array := C.pop_pop_context_function_mempool_get_in_flight_atvs(v.ref)
	defer freeArrayAtv(&array)
	return createArrayAtv(&array)
}

func (v *PopContext2) MemPoolGetVtbsInFlight() []*Vtb {
	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	array := C.pop_pop_context_function_mempool_get_in_flight_vtbs(v.ref)
	defer freeArrayVtb(&array)
	return createArrayVtb(&array)
}

func (v *PopContext2) MemPoolGetVbkBlocksInFlight() []*VbkBlock {
	if v.ref == nil {
		panic("PopContext does not initialized")
	}
	array := C.pop_pop_context_function_mempool_get_in_flight_vbk_blocks(v.ref)
	defer freeArrayVbkBlock(&array)
	return createArrayVbkBlock(&array)
}
