// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/bytestream.h>
import "C"
import (
	"runtime"
	"unsafe"
)

// NewVbkByteStream - returns a new byte stream struct
func NewVbkByteStream(ref *C.VBK_ByteStream) *VbkByteStream {
	if ref == nil {
		return nil
	}
	stream := &VbkByteStream{ref}
	runtime.SetFinalizer(stream, func(v *VbkByteStream) {
		v.Free()
	})
	return stream
}

// Free - Dealocates memory allocated for the VbkByteStream.
func (v *VbkByteStream) Free() {
	if v.ref != nil {
		C.VBK_ByteStream_Free(v.ref)
		v.ref = nil
	}
}

// VbkByteStream ...
type VbkByteStream struct {
	ref *C.VBK_ByteStream
}

// Read ...
func (v *VbkByteStream) Read(p []byte) (n int, err error) {
	len := C.size_t(len(p))
	if len != 0 {
		bufferC := (*C.uint8_t)(unsafe.Pointer(&p[0]))
		res := C.VBK_ByteStream_Read(v.ref, bufferC, len)
		return int(res), nil
	}
	return 0, nil
}
