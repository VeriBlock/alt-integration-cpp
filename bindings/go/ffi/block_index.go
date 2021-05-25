// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/entities/block_index.h>
import "C"
import (
	"encoding/json"
	"runtime"
)

// BlockValidityStatus ...
type BlockValidityStatus uint32

var (
	// BlockValidUnknown - Default state for validity - validity state is unknown
	BlockValidUnknown BlockValidityStatus = BlockValidityStatus(C.BLOCK_VALID_UNKNOWN)
	// BlockBootstrap - This is a bootstrap block
	BlockBootstrap BlockValidityStatus = BlockValidityStatus(C.BLOCK_BOOTSTRAP)
	// BlockFailedBlock - Block is statelessly valid, but the altchain marked it as failed
	BlockFailedBlock BlockValidityStatus =  BlockValidityStatus(C.BLOCK_FAILED_BLOCK)
	// BlockFailedPop - Block failed state{less,ful} validation due to its payloads
	BlockFailedPop BlockValidityStatus = BlockValidityStatus(C.BLOCK_FAILED_POP)
	// BlockFailedChild - Block is state{lessly,fully} valid and the altchain did not report it as
	// invalid, but some of the ancestor blocks are invalid
	BlockFailedChild BlockValidityStatus = BlockValidityStatus(C.BLOCK_FAILED_CHILD)
	// BlockFailedMask - All invalidity flags
	BlockFailedMask BlockValidityStatus = BlockValidityStatus(C.BLOCK_FAILED_MASK)
	// BlockHasPayloads - AcceptBlock has been executed on this block
	BlockHasPayloads BlockValidityStatus = BlockValidityStatus(C.BLOCK_HAS_PAYLOADS)
	// BlockApplied - The block has been applied via PopStateMachine
	BlockApplied BlockValidityStatus = BlockValidityStatus(C.BLOCK_ACTIVE)
	// BlockApplied - The block is temporarily deleted
	BlockDeleted BlockValidityStatus = BlockValidityStatus(C.BLOCK_DELETED)
)


type AltBlockIndex struct {
	ref *C.pop_alt_block_index_t
}

type VbkBlockIndex struct {
	ref *C.pop_vbk_block_index_t
}

type BtcBlockIndex struct {
	ref *C.pop_btc_block_index_t
}

func (v *AltBlockIndex) validate() {
	if v.ref == nil {
		panic("AltBlockIndex does not initialized")
	}
}

func (v *VbkBlockIndex) validate() {
	if v.ref == nil {
		panic("VbkBlockIndex does not initialized")
	}
}

func (v *BtcBlockIndex) validate() {
	if v.ref == nil {
		panic("BtcBlockIndex does not initialized")
	}
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
	v.validate()
	return createAltBlock(C.pop_alt_block_index_get_header(v.ref))
}

func (v *VbkBlockIndex) GetHeader() *VbkBlock {
	v.validate()
	return createVbkBlock(C.pop_vbk_block_index_get_header(v.ref))
}

func (v *BtcBlockIndex) GetHeader() *BtcBlock {
	v.validate()
	return createBtcBlock(C.pop_btc_block_index_get_header(v.ref))
}

func (v *AltBlockIndex) GetHeight() uint32 {
	v.validate()
	return uint32(C.pop_alt_block_index_get_height(v.ref))
}

func (v *VbkBlockIndex) GetHeight() uint32 {
	v.validate()
	return uint32(C.pop_vbk_block_index_get_height(v.ref))
}

func (v *BtcBlockIndex) GetHeight() uint32 {
	v.validate()
	return uint32(C.pop_btc_block_index_get_height(v.ref))
}

func (v *AltBlockIndex) GetStatus() uint32 {
	v.validate()
	return uint32(C.pop_alt_block_index_get_status(v.ref))
}

func (v *VbkBlockIndex) GetStatus() uint32 {
	v.validate()
	return uint32(C.pop_vbk_block_index_get_status(v.ref))
}

func (v *BtcBlockIndex) GetStatus() uint32 {
	v.validate()
	return uint32(C.pop_btc_block_index_get_status(v.ref))
}

func (v *AltBlockIndex) HasFlag(flag BlockValidityStatus) bool {
	v.validate()
	return bool(C.pop_alt_block_index_function_has_flag(v.ref, C.uint32_t(flag)))
}

func (v *VbkBlockIndex) HasFlag(flag BlockValidityStatus) bool {
	v.validate()
	return bool(C.pop_vbk_block_index_function_has_flag(v.ref, C.uint32_t(flag)))
}

func (v *BtcBlockIndex) HasFlag(flag BlockValidityStatus) bool {
	v.validate()
	return bool(C.pop_btc_block_index_function_has_flag(v.ref, C.uint32_t(flag)))
}

func (v *AltBlockIndex) ToJSON() (map[string]interface{}, error) {
	v.validate()
	str := C.pop_alt_block_index_to_json(v.ref)
	defer freeArrayChar(&str)
	json_str := createString(&str)

	var res map[string]interface{}
	err := json.Unmarshal([]byte(json_str), &res)
	return res, err
}

func (v *VbkBlockIndex) ToJSON() (map[string]interface{}, error) {
	v.validate()
	str := C.pop_vbk_block_index_to_json(v.ref)
	defer freeArrayChar(&str)
	json_str := createString(&str)

	var res map[string]interface{}
	err := json.Unmarshal([]byte(json_str), &res)
	return res, err
}

func (v *BtcBlockIndex) ToJSON() (map[string]interface{}, error) {
	v.validate()
	str := C.pop_btc_block_index_to_json(v.ref)
	defer freeArrayChar(&str)
	json_str := createString(&str)

	var res map[string]interface{}
	err := json.Unmarshal([]byte(json_str), &res)
	return res, err
}
