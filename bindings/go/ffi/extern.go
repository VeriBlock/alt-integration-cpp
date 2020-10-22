package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++
// #include <veriblock/c/config.h>
import "C"
import "unsafe"

// Exported functions
var (
	OnGetAltchainID      = func() int { panic("OnGetAltchainID not set") }
	OnGetBootstrapBlock  = func() string { panic("OnGetBootstrapBlock not set") }
	OnGetBlockHeaderHash = func(toBeHashed []byte) []byte { panic("OnGetBlockHeaderHash not set") }
	OnGetAtv             = func(id []byte) []byte { panic("OnGetAtv not set") }
	OnGetVtb             = func(id []byte) []byte { panic("OnGetVtb not set") }
	OnGetVbk             = func(id []byte) []byte { panic("OnGetVbk not set") }
)

//export VBK_getAltchainId
func VBK_getAltchainId() C.int64_t {
	return C.int64_t(OnGetAltchainID())
}

//export VBK_getBootstrapBlock
func VBK_getBootstrapBlock() *C.char {
	return C.CString(OnGetBootstrapBlock())
}

//export VBK_getBlockHeaderHash
func VBK_getBlockHeaderHash(in *C.uint8_t, inlen C.int, out *C.uint8_t, outlen *C.int) {
	bytesSize := int(inlen)
	resBytes := (*(*[]byte)(unsafe.Pointer(in)))[:bytesSize:bytesSize]
	data := OnGetBlockHeaderHash(resBytes)
	*outlen = C.int(len(data))
	*out = *(*C.uint8_t)(unsafe.Pointer(&data[0]))
}

// PayloadsProvider externs

//export VBK_getATV
func VBK_getATV(id *C.uint8_t, idSize C.int, atvBytesOut *C.uint8_t, atvBytesLen *C.int) {
	bytesSize := int(idSize)
	resBytes := (*(*[]byte)(unsafe.Pointer(id)))[:bytesSize:bytesSize]
	data := OnGetAtv(resBytes)
	*atvBytesLen = C.int(len(data))
	*atvBytesOut = *(*C.uint8_t)(unsafe.Pointer(&data[0]))
}

//export VBK_getVTB
func VBK_getVTB(id *C.uint8_t, idSize C.int, vtbBytesOut *C.uint8_t, vtbBytesLen *C.int) {
	bytesSize := int(idSize)
	resBytes := (*(*[]byte)(unsafe.Pointer(id)))[:bytesSize:bytesSize]
	data := OnGetVtb(resBytes)
	*vtbBytesLen = C.int(len(data))
	*vtbBytesOut = *(*C.uint8_t)(unsafe.Pointer(&data[0]))
}

//export VBK_getVBK
func VBK_getVBK(id *C.uint8_t, idSize C.int, vbkBytesOut *C.uint8_t, vbkBytesLen *C.int) {
	bytesSize := int(idSize)
	resBytes := (*(*[]byte)(unsafe.Pointer(id)))[:bytesSize:bytesSize]
	data := OnGetVbk(resBytes)
	*vbkBytesLen = C.int(len(data))
	*vbkBytesOut = *(*C.uint8_t)(unsafe.Pointer(&data[0]))
}
