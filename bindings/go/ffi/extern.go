package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++
// #include <veriblock/c/config.h>
import "C"
import "unsafe"

// Exported functions
var (
	OnGetAltchainID      = func() int64 { panic("OnGetAltchainID not set") }
	OnGetBootstrapBlock  = func() string { panic("OnGetBootstrapBlock not set") }
	OnGetBlockHeaderHash = func(toBeHashed []byte) []byte { panic("OnGetBlockHeaderHash not set") }
	OnGetAtv             = func(id []byte) []byte { panic("OnGetAtv not set") }
	OnGetVtb             = func(id []byte) []byte { panic("OnGetVtb not set") }
	OnGetVbk             = func(id []byte) []byte { panic("OnGetVbk not set") }
	OnIsHeader           = func(data []byte) bool { panic("OnIsHeader not set") }
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
}

//export VBK_isHeader
func VBK_isHeader(in *C.uint8_t, inlen C.int) C.int {
	data := convertToBytes(in, inlen)
	res := OnIsHeader(data)
	if res == true {
		return 1
	}
	return 0
}

// PayloadsProvider externs

//export VBK_getATV
func VBK_getATV(id *C.uint8_t, idSize C.int, atvBytesOut *C.uint8_t, atvBytesLen *C.int) C.int {
	resBytes := convertToBytes(id, idSize)
	data := OnGetAtv(resBytes)
	if data == nil {
		// false
		return 0
	}
	*atvBytesLen = C.int(len(data))
	*atvBytesOut = *(*C.uint8_t)(unsafe.Pointer(&data[0]))
	// true
	return 1
}

//export VBK_getVTB
func VBK_getVTB(id *C.uint8_t, idSize C.int, vtbBytesOut *C.uint8_t, vtbBytesLen *C.int) C.int {
	resBytes := convertToBytes(id, idSize)
	data := OnGetVtb(resBytes)
	if data == nil {
		// false
		return 0
	}
	*vtbBytesLen = C.int(len(data))
	*vtbBytesOut = *(*C.uint8_t)(unsafe.Pointer(&data[0]))
	// true
	return 1
}

//export VBK_getVBK
func VBK_getVBK(id *C.uint8_t, idSize C.int, vbkBytesOut *C.uint8_t, vbkBytesLen *C.int) C.int {
	resBytes := convertToBytes(id, idSize)
	data := OnGetVbk(resBytes)
	if data == nil {
		// false
		return 0
	}
	*vbkBytesLen = C.int(len(data))
	*vbkBytesOut = *(*C.uint8_t)(unsafe.Pointer(&data[0]))
	// true
	return 1
}
