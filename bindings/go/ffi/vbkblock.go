package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++ -lrocksdb -ldl -lm
// #include <veriblock/pop/c/entities/vbkblock.h>
import "C"
import (
	"runtime"
	"unsafe"
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
	ptr := uintptr(unsafe.Pointer(array.data))
	for i := 0; i < len(res); i++ {
		val := (*C.pop_vbk_block_t)(unsafe.Pointer(ptr + unsafe.Sizeof(C.int(0))*uintptr(i)))
		res[i] = createVbkBlock(val)
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
