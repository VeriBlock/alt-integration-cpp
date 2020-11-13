package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++
// #include <veriblock/c/bytestream.h>
import "C"
import "unsafe"

type VbkByteStream struct {
	ref *C.VBK_ByteStream
}

func (v VbkByteStream) ReadAll() []byte {
	var result []byte

	buffer := make([]byte, 1024)
	bufferC := (*C.uint8_t)(unsafe.Pointer(&buffer[0]))

	n := C.VBK_ByteStream_Read(v.ref, bufferC, C.ulong(1024))
	for {
		result = append(result, buffer[:n]...)
		n := C.VBK_ByteStream_Read(v.ref, bufferC, C.ulong(1024))
		if n == 0 {
			break
		}
	}

	return result
}

func (v VbkByteStream) Free() {
	C.VBK_ByteStream_Free(v.ref)
}
