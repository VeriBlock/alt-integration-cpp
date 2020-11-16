package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++
// #include <veriblock/c/utils.h>
import "C"
import (
	"unsafe"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
)

// VbkBlockGetID ...
func VbkBlockGetID(blockBytes []byte) []byte {
	idBytes := make([]byte, veriblock.VblakePreviousBlockHashSize)
	var idBytesSize int

	blockBytesC := (*C.uint8_t)(unsafe.Pointer(&blockBytes[0]))
	idBytesC := (*C.uint8_t)(unsafe.Pointer(&idBytes[0]))
	idBytesSizeC := (*C.int)(unsafe.Pointer(&idBytesSize))
	C.VBK_VbkBlock_getId(blockBytesC, C.int(len(blockBytes)), idBytesC, idBytesSizeC)

	return idBytes
}

// VtbGetID ...
func VtbGetID(vtbBytes []byte) []byte {
	idBytes := make([]byte, veriblock.Sha256HashSize)
	var idBytesSize int

	vtbBytesC := (*C.uint8_t)(unsafe.Pointer(&vtbBytes[0]))
	idBytesC := (*C.uint8_t)(unsafe.Pointer(&idBytes[0]))
	idBytesSizeC := (*C.int)(unsafe.Pointer(&idBytesSize))
	C.VBK_VTB_getId(vtbBytesC, C.int(len(vtbBytes)), idBytesC, idBytesSizeC)

	return idBytes
}

// AtvGetID ...
func AtvGetID(atvBytes []byte) []byte {
	idBytes := make([]byte, veriblock.Sha256HashSize)
	var idBytesSize int

	atvBytesC := (*C.uint8_t)(unsafe.Pointer(&atvBytes[0]))
	idBytesC := (*C.uint8_t)(unsafe.Pointer(&idBytes[0]))
	idBytesSizeC := (*C.int)(unsafe.Pointer(&idBytesSize))
	C.VBK_ATV_getId(atvBytesC, C.int(len(atvBytes)), idBytesC, idBytesSizeC)

	return idBytes
}

// VbkBlockGetHash ...
func VbkBlockGetHash(blockBytes []byte) []byte {
	hashBytes := make([]byte, veriblock.VblakeBlockHashSize)
	var hashBytesSize int

	blockBytesC := (*C.uint8_t)(unsafe.Pointer(&blockBytes[0]))
	hashBytesC := (*C.uint8_t)(unsafe.Pointer(&hashBytes[0]))
	hashBytesSizeC := (*C.int)(unsafe.Pointer(&hashBytesSize))
	C.VBK_VbkBlock_getHash(blockBytesC, C.int(len(blockBytes)), hashBytesC, hashBytesSizeC)

	return hashBytes
}

// BtcBlockGetHash ...
func BtcBlockGetHash(blockBytes []byte) []byte {
	hashBytes := make([]byte, veriblock.Sha256HashSize)
	var hashBytesSize int

	blockBytesC := (*C.uint8_t)(unsafe.Pointer(&blockBytes[0]))
	hashBytesC := (*C.uint8_t)(unsafe.Pointer(&hashBytes[0]))
	hashBytesSizeC := (*C.int)(unsafe.Pointer(&hashBytesSize))
	C.VBK_BtcBlock_getHash(blockBytesC, C.int(len(blockBytes)), hashBytesC, hashBytesSizeC)

	return hashBytes
}
