// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/entities/btcblock.h>
import "C"
import (
	"encoding/json"
	"runtime"

	"github.com/stretchr/testify/assert"
)

type BtcBlock struct {
	ref *C.pop_btc_block_t
}

func GenerateDefaultBtcBlock() *BtcBlock {
	val := &BtcBlock{ref: C.pop_btc_block_generate_default_value()}
	runtime.SetFinalizer(val, func(v *BtcBlock) {
		v.Free()
	})
	return val
}

func createBtcBlock(ref *C.pop_btc_block_t) *BtcBlock {
	val := &BtcBlock{ref: ref}
	runtime.SetFinalizer(val, func(v *BtcBlock) {
		v.Free()
	})
	return val
}

func (v *BtcBlock) Free() {
	if v.ref != nil {
		C.pop_btc_block_free(v.ref)
		v.ref = nil
	}
}

func (v *BtcBlock) GetHash() []byte {
	if v.ref == nil {
		panic("BtcBlock does not initialized")
	}
	array := C.pop_btc_block_get_hash(v.ref)
	defer freeArrayU8(&array)
	return createBytes(&array)
}

func (v *BtcBlock) GetPreviousBlock() []byte {
	if v.ref == nil {
		panic("BtcBlock does not initialized")
	}
	array := C.pop_btc_block_get_previous_block(v.ref)
	defer freeArrayU8(&array)
	return createBytes(&array)
}

func (v *BtcBlock) GetMerkleRoot() []byte {
	if v.ref == nil {
		panic("BtcBlock does not initialized")
	}
	array := C.pop_btc_block_get_merkle_root(v.ref)
	defer freeArrayU8(&array)
	return createBytes(&array)
}

func (v *BtcBlock) GetVersion() uint32 {
	if v.ref == nil {
		panic("BtcBlock does not initialized")
	}
	return uint32(C.pop_btc_block_get_version(v.ref))
}

func (v *BtcBlock) GetTimestamp() uint32 {
	if v.ref == nil {
		panic("BtcBlock does not initialized")
	}
	return uint32(C.pop_btc_block_get_timestamp(v.ref))
}

func (v *BtcBlock) GetNonce() uint32 {
	if v.ref == nil {
		panic("BtcBlock does not initialized")
	}
	return uint32(C.pop_btc_block_get_nonce(v.ref))
}

func (v *BtcBlock) GetDifficulty() uint32 {
	if v.ref == nil {
		panic("BtcBlock does not initialized")
	}
	return uint32(C.pop_btc_block_get_difficulty(v.ref))
}

func (v *BtcBlock) ToJSON() (map[string]interface{}, error) {
	if v.ref == nil {
		panic("BtcBlock does not initialized")
	}
	str := C.pop_btc_block_to_json(v.ref)
	defer freeArrayChar(&str)
	json_str := createString(&str)

	var res map[string]interface{}
	err := json.Unmarshal([]byte(json_str), &res)
	return res, err
}

func (val1 *BtcBlock) assertEquals(assert *assert.Assertions, val2 *BtcBlock) {
	assert.Equal(val1.GetDifficulty(), val2.GetDifficulty())
	assert.Equal(val1.GetNonce(), val2.GetNonce())
	assert.Equal(val1.GetTimestamp(), val2.GetTimestamp())
	assert.Equal(val1.GetMerkleRoot(), val2.GetMerkleRoot())
	assert.Equal(val1.GetPreviousBlock(), val2.GetPreviousBlock())
	assert.Equal(val1.GetHash(), val2.GetHash())
}
