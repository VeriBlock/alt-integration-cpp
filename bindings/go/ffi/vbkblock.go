// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo CFLAGS: -I../../../include -fsanitize=address
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++ -lrocksdb -ldl -lm
// #include <veriblock/pop/c/entities/vbkblock.h>
import "C"
import (
	"runtime"

	"github.com/stretchr/testify/assert"
)

type VbkBlock struct {
	ref *C.pop_vbk_block_t
}

func GenerateDefaultVbkBlock() *VbkBlock {
	val := &VbkBlock{ref: C.pop_vbk_block_generate_default_value()}
	runtime.SetFinalizer(val, func(v *VbkBlock) {
		v.Free()
	})
	return val
}

func createVbkBlock(ref *C.pop_vbk_block_t) *VbkBlock {
	val := &VbkBlock{ref: ref}
	runtime.SetFinalizer(val, func(v *VbkBlock) {
		v.Free()
	})
	return val
}

func freeArrayVbkBlock(array *C.pop_array_vbk_block_t) {
	C.pop_array_vbk_block_free(array)
}

func createArrayVbkBlock(array *C.pop_array_vbk_block_t) []*VbkBlock {
	res := make([]*VbkBlock, array.size, array.size)
	for i := 0; i < len(res); i++ {
		res[i] = createVbkBlock(C.pop_array_vbk_block_at(array, C.size_t(i)))
	}
	return res
}

func (v *VbkBlock) Free() {
	if v.ref != nil {
		C.pop_vbk_block_free(v.ref)
		v.ref = nil
	}
}

func (v *VbkBlock) GetHash() []byte {
	if v.ref == nil {
		panic("VbkBlock does not initialized")
	}
	array := C.pop_vbk_block_get_hash(v.ref)
	defer freeArrayU8(&array)
	return createBytes(&array)
}

func (v *VbkBlock) GetPreviousBlock() []byte {
	if v.ref == nil {
		panic("VbkBlock does not initialized")
	}
	array := C.pop_vbk_block_get_previous_block(v.ref)
	defer freeArrayU8(&array)
	return createBytes(&array)
}

func (v *VbkBlock) GetMerkleRoot() []byte {
	if v.ref == nil {
		panic("VbkBlock does not initialized")
	}
	array := C.pop_vbk_block_get_merkle_root(v.ref)
	defer freeArrayU8(&array)
	return createBytes(&array)
}

func (v *VbkBlock) GetPreviousKeystone() []byte {
	if v.ref == nil {
		panic("VbkBlock does not initialized")
	}
	array := C.pop_vbk_block_get_previous_keystone(v.ref)
	defer freeArrayU8(&array)
	return createBytes(&array)
}

func (v *VbkBlock) GetSecondPreviousKeystone() []byte {
	if v.ref == nil {
		panic("VbkBlock does not initialized")
	}
	array := C.pop_vbk_block_get_second_previous_keystone(v.ref)
	defer freeArrayU8(&array)
	return createBytes(&array)
}

func (v *VbkBlock) GetVersion() int16 {
	if v.ref == nil {
		panic("VbkBlock does not initialized")
	}
	return int16(C.pop_vbk_block_get_version(v.ref))
}

func (v *VbkBlock) GetTimestamp() uint32 {
	if v.ref == nil {
		panic("VbkBlock does not initialized")
	}
	return uint32(C.pop_vbk_block_get_timestamp(v.ref))
}

func (v *VbkBlock) GetDifficulty() int32 {
	if v.ref == nil {
		panic("VbkBlock does not initialized")
	}
	return int32(C.pop_vbk_block_get_difficulty(v.ref))
}

func (v *VbkBlock) GetNonce() uint64 {
	if v.ref == nil {
		panic("VbkBlock does not initialized")
	}
	return uint64(C.pop_vbk_block_get_nonce(v.ref))
}

func (v *VbkBlock) GetHeight() int32 {
	if v.ref == nil {
		panic("VbkBlock does not initialized")
	}
	return int32(C.pop_vbk_block_get_height(v.ref))
}

func (val1 *VbkBlock) assertEquals(assert *assert.Assertions, val2 *VbkBlock) {
	assert.Equal(val1.GetDifficulty(), val2.GetDifficulty())
	assert.Equal(val1.GetHeight(), val2.GetHeight())
	assert.Equal(val1.GetNonce(), val2.GetNonce())
	assert.Equal(val1.GetTimestamp(), val2.GetTimestamp())
	assert.Equal(val1.GetVersion(), val2.GetVersion())
	assert.Equal(val1.GetMerkleRoot(), val2.GetMerkleRoot())
	assert.Equal(val1.GetPreviousBlock(), val2.GetPreviousBlock())
	assert.Equal(val1.GetPreviousKeystone(), val2.GetPreviousKeystone())
	assert.Equal(val1.GetSecondPreviousKeystone(), val2.GetSecondPreviousKeystone())
	assert.Equal(val1.GetHash(), val2.GetHash())
}
