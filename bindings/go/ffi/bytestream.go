package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++
// #include <veriblock/c/bytestream.h>
import "C"
import (
	"unsafe"
)

// NewVbkByteStream - returns a new byte stream struct
func NewVbkByteStream(ref *C.VBK_ByteStream) *VbkByteStream {
	if ref == nil {
		return nil
	}
	return &VbkByteStream{ref}
}

// VbkByteStream ...
type VbkByteStream struct {
	ref *C.VBK_ByteStream
}

// Read ...
func (v *VbkByteStream) Read(p []byte) (n int, err error) {
	// TODO: Check is this correct way
	len := C.ulong(len(p))
	bufferC := (*C.uint8_t)(unsafe.Pointer(&p[0]))
	res := C.VBK_ByteStream_Read(v.ref, bufferC, len)
	return int(res), nil
}

// Free ...
func (v *VbkByteStream) Free() {
	C.VBK_ByteStream_Free(v.ref)
}
