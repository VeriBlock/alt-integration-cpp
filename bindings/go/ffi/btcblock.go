package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++ -lrocksdb -ldl -lm
// #include <veriblock/pop/c/entities/btcblock.h>
import "C"
import "runtime"

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
	return ConvertToBytes(&array)
}

func (v *BtcBlock) GetPreviousBlock() []byte {
	if v.ref == nil {
		panic("BtcBlock does not initialized")
	}
	array := C.pop_btc_block_get_previous_block(v.ref)
	return ConvertToBytes(&array)
}

func (v *BtcBlock) GetMerkleRoot() []byte {
	if v.ref == nil {
		panic("BtcBlock does not initialized")
	}
	array := C.pop_btc_block_get_merkle_root(v.ref)
	return ConvertToBytes(&array)
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
