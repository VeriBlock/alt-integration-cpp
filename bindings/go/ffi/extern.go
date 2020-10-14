package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++
// #include <veriblock/c/config.h>
import "C"
import "unsafe"

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
func VBK_getATV(idBytes *C.uint8_t, idSize C.int, atvBytesOut *C.uint8_t, atvBytesLen *C.int) {
	OnGetAtv()
}

//export VBK_getVTB
func VBK_getVTB(idBytes *C.uint8_t, idSize C.int, vtbBytesOut *C.uint8_t, vtbBytesLen *C.int) {
	OnGetVtb()
}

//export VBK_getVBK
func VBK_getVBK(idBytes *C.uint8_t, idSize C.int, vbkBytesOut *C.uint8_t, vbkBytesLen *C.int) {
	OnGetVbk()
}
