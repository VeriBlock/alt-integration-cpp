// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/entities/block_index.h>
import "C"
import "runtime"

type AltBlockIndex struct {
	ref *C.pop_alt_block_index_t
}

type VbkBlockIndex struct {
	ref *C.pop_vbk_block_index_t
}

type BtcBlockIndex struct {
	ref *C.pop_btc_block_index_t
}

func createAltBlockIndex(ref *C.pop_alt_block_index_t) *AltBlockIndex {
	val := &AltBlockIndex{ref: ref}
	runtime.SetFinalizer(val, func(v *AltBlockIndex) {
		v.Free()
	})
	return val
}

func createVbkBlockIndex(ref *C.pop_vbk_block_index_t) *VbkBlockIndex {
	val := &VbkBlockIndex{ref: ref}
	runtime.SetFinalizer(val, func(v *VbkBlockIndex) {
		v.Free()
	})
	return val
}

func createBtcBlockIndex(ref *C.pop_btc_block_index_t) *BtcBlockIndex {
	val := &BtcBlockIndex{ref: ref}
	runtime.SetFinalizer(val, func(v *BtcBlockIndex) {
		v.Free()
	})
	return val
}

func (v *AltBlockIndex) Free() {
	if v.ref != nil {
		C.pop_alt_block_index_free(v.ref)
		v.ref = nil
	}
}

func (v *VbkBlockIndex) Free() {
	if v.ref != nil {
		C.pop_vbk_block_index_free(v.ref)
		v.ref = nil
	}
}

func (v *BtcBlockIndex) Free() {
	if v.ref != nil {
		C.pop_btc_block_index_free(v.ref)
		v.ref = nil
	}
}

func (v *AltBlockIndex) GetHeader() *AltBlock {
	if v.ref == nil {
		panic("AltBlockIndex does not initialized")
	}
	return createAltBlock(C.pop_alt_block_index_get_header(v.ref))
}

func (v *VbkBlockIndex) GetHeader() *VbkBlock {
	if v.ref == nil {
		panic("VbkBlockIndex does not initialized")
	}
	return createVbkBlock(C.pop_vbk_block_index_get_header(v.ref))
}

func (v *BtcBlockIndex) GetHeader() *BtcBlock {
	if v.ref == nil {
		panic("BtcBlockIndex does not initialized")
	}
	return createBtcBlock(C.pop_btc_block_index_get_header(v.ref))
}

func (v *AltBlockIndex) GetStatus() uint32 {
	if v.ref == nil {
		panic("AltBlockIndex does not initialized")
	}
	return uint32(C.pop_alt_block_index_get_status(v.ref))
}

func (v *VbkBlockIndex) GetStatus() uint32 {
	if v.ref == nil {
		panic("VbkBlockIndex does not initialized")
	}
	return uint32(C.pop_vbk_block_index_get_status(v.ref))
}

func (v *BtcBlockIndex) GetStatus() uint32 {
	if v.ref == nil {
		panic("BtcBlockIndex does not initialized")
	}
	return uint32(C.pop_btc_block_index_get_status(v.ref))
}
