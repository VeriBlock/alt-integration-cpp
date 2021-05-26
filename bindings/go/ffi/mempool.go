// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/mempool.h>
import "C"

func (v *PopContext2) MemPoolSubmitVbk(vbkBlock *VbkBlock) (int, error) {
	v.validate()
	vbkBlock.validate()

	state := NewValidationState2()
	defer state.Free()

	res := C.pop_pop_context_function_mempool_submit_vbk(v.ref, vbkBlock.ref, state.ref)

	return int(res), state.Error()
}

func (v *PopContext2) MemPoolSubmitVtb(vtb *Vtb) (int, error) {
	v.validate()
	vtb.validate()

	state := NewValidationState2()
	defer state.Free()

	res := C.pop_pop_context_function_mempool_submit_vtb(v.ref, vtb.ref, state.ref)

	return int(res), state.Error()
}

func (v *PopContext2) MemPoolSubmitAtv(atv *Atv) (int, error) {
	v.validate()
	atv.validate()

	state := NewValidationState2()
	defer state.Free()

	res := C.pop_pop_context_function_mempool_submit_atv(v.ref, atv.ref, state.ref)

	return int(res), state.Error()
}

func (v *PopContext2) MemPoolGeneratePopData() *PopData {
	v.validate()
	return createPopData(C.pop_pop_context_function_mempool_generate_pop_data(v.ref))
}

func (v *PopContext2) MemPoolGetAtv(id []byte) *Atv {
	v.validate()
	atv_c := C.pop_pop_context_function_mempool_get_atv(v.ref, createCBytes(id))
	atv := createAtv(atv_c)
	return atv
}

func (v *PopContext2) MemPoolGetVtb(id []byte) *Vtb {
	v.validate()
	vtb_c := C.pop_pop_context_function_mempool_get_vtb(v.ref, createCBytes(id))
	vtb := createVtb(vtb_c)
	return vtb
}

func (v *PopContext2) MemPoolGetVbkBlock(id []byte) *VbkBlock {
	v.validate()
	vbk_c := C.pop_pop_context_function_mempool_get_vbk_block(v.ref, createCBytes(id))
	vbk := createVbkBlock(vbk_c)
	return vbk
}

func (v *PopContext2) MemPoolGetAtvs() []*Atv {
	v.validate()
	array := C.pop_pop_context_function_mempool_get_atvs(v.ref)
	defer freeArrayAtv(&array)
	return createArrayAtv(&array)
}

func (v *PopContext2) MemPoolGetVtbs() []*Vtb {
	v.validate()
	array := C.pop_pop_context_function_mempool_get_vtbs(v.ref)
	defer freeArrayVtb(&array)
	return createArrayVtb(&array)
}

func (v *PopContext2) MemPoolGetVbkBlocks() []*VbkBlock {
	v.validate()
	array := C.pop_pop_context_function_mempool_get_vbk_blocks(v.ref)
	defer freeArrayVbkBlock(&array)
	return createArrayVbkBlock(&array)
}

func (v *PopContext2) MemPoolGetAtvsInFlight() []*Atv {
	v.validate()
	array := C.pop_pop_context_function_mempool_get_in_flight_atvs(v.ref)
	defer freeArrayAtv(&array)
	return createArrayAtv(&array)
}

func (v *PopContext2) MemPoolGetVtbsInFlight() []*Vtb {
	v.validate()
	array := C.pop_pop_context_function_mempool_get_in_flight_vtbs(v.ref)
	defer freeArrayVtb(&array)
	return createArrayVtb(&array)
}

func (v *PopContext2) MemPoolGetVbkBlocksInFlight() []*VbkBlock {
	v.validate()
	array := C.pop_pop_context_function_mempool_get_in_flight_vbk_blocks(v.ref)
	defer freeArrayVbkBlock(&array)
	return createArrayVbkBlock(&array)
}

func (v *PopContext2) MemPoolGetMissingBtcBlocks() [][]byte {
	v.validate()
	array := C.pop_pop_context_function_mempool_get_missing_btc_blocks(v.ref)
	defer freeArrayArrayU8(&array)
	return createArrayOfArraysU8(&array)
}

func (v *PopContext2) MemPoolRemoveAll(popData *PopData) {
	v.validate()
	if popData != nil {
		popData.validate()
		C.pop_pop_context_function_mempool_remove_all(v.ref, popData.ref)
	}
}

func (v *PopContext2) MemPoolCleanUp() {
	v.validate()
	C.pop_pop_context_function_mempool_clean_up(v.ref)
}

func (v *PopContext2) MemPoolClear() {
	v.validate()
	C.pop_pop_context_function_mempool_clear(v.ref)
}
