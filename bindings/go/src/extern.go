package veriblock

// #cgo LDFLAGS: -L../../c/lib -lveriblock-pop-c -lveriblock-pop-cpp -lstdc++
// #include "../../c/lib/config.h"
import "C"

//export VBK_getAltchainId
func VBK_getAltchainId() C.int64_t {
	// Todo: Return altchain id
	return C.int64_t(1)
}

//export VBK_getBootstrapBlock
func VBK_getBootstrapBlock() *C.char {
	// Todo: Return bootstrap block
	return C.CString("")
}

//export VBK_getBlockHeaderHash
func VBK_getBlockHeaderHash(in *C.uint8_t, inlen C.int, out *C.uint8_t, outlen *C.int) {
	// Todo: Put value in out
}

// PayloadsProvider externs

//export VBK_getATV
func VBK_getATV(idBytes *C.uint8_t, idSize C.int, atvBytesOut *C.uint8_t, atvBytesLen *C.int) {
	// Todo: Put value in out
}

//export VBK_getVTB
func VBK_getVTB(idBytes *C.uint8_t, idSize C.int, vtbBytesOut *C.uint8_t, vtbBytesLen *C.int) {
	// Todo: Put value in out
}

//export VBK_getVBK
func VBK_getVBK(idBytes *C.uint8_t, idSize C.int, vbkBytesOut *C.uint8_t, vbkBytesLen *C.int) {
	// Todo: Put value in out
}
