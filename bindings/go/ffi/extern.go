// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/config.h>
// #include <veriblock/pop/c/validation_state.h>
// #include <string.h>
import "C"
import (
	"unsafe"
)

// Exported functions
var (
	OnGetAltchainID      = func() int64 { panic("OnGetAltchainID not set") }
	OnGetBootstrapBlock  = func() string { panic("OnGetBootstrapBlock not set fuck") }
	OnGetBlockHeaderHash = func(toBeHashed []byte) []byte { panic("OnGetBlockHeaderHash not set") }
	OnCheckBlockHeader   = func(header []byte, root []byte, state *ValidationState) bool { panic("OnCheckBlockHeader not set") }
	OnLog                = func(log_level string, msg string) { panic("OnLog not set") }
)

//export VBK_getAltchainId
func VBK_getAltchainId() C.int64_t {
	return C.int64_t(OnGetAltchainID())
}

//export VBK_getBootstrapBlock
func VBK_getBootstrapBlock() *C.char {
	return C.CString(OnGetBootstrapBlock())
}

func convertToBytes(in *C.uint8_t, inlen C.int) []byte {
	bytesSize := int(inlen)
	bytes := make([]byte, 0, bytesSize)
	for bytesSize > 0 {
		buffer := (*[1024]byte)(unsafe.Pointer(in))
		if bytesSize > 1024 {
			bytes = append(bytes, buffer[:]...)
		} else {
			bytes = append(bytes, buffer[:bytesSize]...)
		}
		bytesSize -= 1024
	}
	return bytes
}

//export VBK_getBlockHeaderHash
func VBK_getBlockHeaderHash(in *C.uint8_t, inlen C.int, out *C.uint8_t, outlen *C.int) {
	resBytes := convertToBytes(in, inlen)
	data := OnGetBlockHeaderHash(resBytes)
	*outlen = C.int(len(data))
	*out = *(*C.uint8_t)(unsafe.Pointer(&data[0]))

	*outlen = C.int(len(data))
	C.memcpy(unsafe.Pointer(out), unsafe.Pointer(&data[0]), C.size_t(len(data)))
}

//export VBK_checkBlockHeader
func VBK_checkBlockHeader(header *C.uint8_t, headerlen C.int, root *C.uint8_t, rootlen C.int, c_state *C.VbkValidationState) C.int {
	state := &ValidationState{ref: c_state}
	header_bytes := convertToBytes(header, headerlen)
	root_bytes := convertToBytes(root, rootlen)
	res := OnCheckBlockHeader(header_bytes, root_bytes, state)
	if res == true {
		return 1
	}
	return 0
}

//export VBK_Logger_log
func VBK_Logger_log(log_level *C.char, msg *C.char) {
	OnLog(C.GoString(log_level), C.GoString(msg))
}
