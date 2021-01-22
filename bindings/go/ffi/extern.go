package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++ -lrocksdb -ldl -lm
// #include <veriblock/c/config.h>
// #include <string.h>
import "C"
import (
	"unsafe"
)

// Exported functions
var (
	OnGetAltchainID      = func() int64 { panic("OnGetAltchainID not set") }
	OnGetBootstrapBlock  = func() string { panic("OnGetBootstrapBlock not set") }
	OnGetBlockHeaderHash = func(toBeHashed []byte) []byte { panic("OnGetBlockHeaderHash not set") }
	OnCheckBlockHeader   = func(header []byte, root []byte) bool { panic("OnCheckBlockHeader not set") }
	OnAcceptedATV        = func(data []byte) { panic("OnAcceptedATV not set") }
	OnAcceptedVTB        = func(data []byte) { panic("OnAcceptedVTB not set") }
	OnAcceptedVBK        = func(data []byte) { panic("OnAcceptedVBK not set") }
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
func VBK_checkBlockHeader(header *C.uint8_t, headerlen C.int, root *C.uint8_t, rootlen C.int) C.int {
	header_bytes := convertToBytes(header, headerlen)
	root_bytes := convertToBytes(root, rootlen)
	res := OnCheckBlockHeader(header_bytes, root_bytes)
	if res == true {
		return 1
	}
	return 0
}

//export VBK_MemPool_onAcceptedATV
func VBK_MemPool_onAcceptedATV(bytes *C.uint8_t, size C.int) {
	data := convertToBytes(bytes, size)
	OnAcceptedATV(data)
}

//export VBK_MemPool_onAcceptedVTB
func VBK_MemPool_onAcceptedVTB(bytes *C.uint8_t, size C.int) {
	data := convertToBytes(bytes, size)
	OnAcceptedVTB(data)
}

//export VBK_MemPool_onAcceptedVBK
func VBK_MemPool_onAcceptedVBK(bytes *C.uint8_t, size C.int) {
	data := convertToBytes(bytes, size)
	OnAcceptedVBK(data)
}
