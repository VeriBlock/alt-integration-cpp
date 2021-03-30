package entities

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++ -lrocksdb -ldl -lm
// #include <veriblock/pop/c/entities/altblock.h>
import "C"
import "runtime"

type AltBlock struct {
	ref *C.pop_alt_block_t
}

func GenerateDefaultAltBlock() *AltBlock {
	val := &AltBlock{ref: C.pop_alt_block_generate_default_value()}
	runtime.SetFinalizer(val, func(v *AltBlock) {
		v.Free()
	})

	return val
}

func (v *AltBlock) Free() {
	if v.ref != nil {
		C.pop_alt_block_free(v.ref)
		v.ref = nil
	}
}

func (v *AltBlock) GetHash() []byte {
	if v.ref == nil {
		panic("VbkBlock does not initialized")
	}
	array := C.pop_alt_block_get_hash(v.ref)
	return ConvertToBytes(&array)
}

func (v *AltBlock) GetPreviousBlock() []byte {
	if v.ref == nil {
		panic("VbkBlock does not initialized")
	}
	array := C.pop_alt_block_get_previous_block(v.ref)
	return ConvertToBytes(&array)
}

func (v *AltBlock) GetTimestamp() uint32 {
	if v.ref == nil {
		panic("VbkBlock does not initialized")
	}
	return uint32(C.pop_alt_block_get_timestamp(v.ref))
}

func (v *AltBlock) GetHeight() int32 {
	if v.ref == nil {
		panic("VbkBlock does not initialized")
	}
	return int32(C.pop_alt_block_get_height(v.ref))
}
