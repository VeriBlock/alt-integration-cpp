package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++
// #include <veriblock/c/bytestream.h>
import "C"
import (
	"io"
	"unsafe"
)

// NewVbkByteStream - returns a new byte stream struct
func NewVbkByteStream(ref *C.VBK_ByteStream) VbkByteStream {
	return VbkByteStream{ref}
}

// VbkByteStream ...
type VbkByteStream struct {
	ref *C.VBK_ByteStream
}

// ReadAll ...
func (v *VbkByteStream) ReadAll(stream io.Writer) {
	buffer := make([]byte, 1024)
	bufferC := (*C.uint8_t)(unsafe.Pointer(&buffer[0]))

	n := C.VBK_ByteStream_Read(v.ref, bufferC, C.ulong(1024))
	for {
		stream.Write(buffer[:n])
		n := C.VBK_ByteStream_Read(v.ref, bufferC, C.ulong(1024))
		if n == 0 {
			break
		}
	}
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
