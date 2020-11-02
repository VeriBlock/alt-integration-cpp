package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++
// #include <veriblock/c/entities_utils.h>
import "C"
import (
	"unsafe"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
)

func VbkBlock_getId(block_bytes []byte) []byte {
	id_bytes := make([]byte, veriblock.VblakePreviousBlockHashSize)
	var id_bytes_size int

	block_bytesC := (*C.uint8_t)(unsafe.Pointer(&block_bytes[0]))
	id_bytesC := (*C.uint8_t)(unsafe.Pointer(&id_bytes[0]))
	id_bytes_sizeC := (*C.int)(unsafe.Pointer(&id_bytes_size))
	C.VBK_VbkBlock_getId(block_bytesC, C.int(len(block_bytes)), id_bytesC, id_bytes_sizeC)

	return id_bytes
}

func Vtb_getId(vtb_bytes []byte) []byte {
	id_bytes := make([]byte, veriblock.Sha256HashSize)
	var id_bytes_size int

	vtb_bytesC := (*C.uint8_t)(unsafe.Pointer(&vtb_bytes[0]))
	id_bytesC := (*C.uint8_t)(unsafe.Pointer(&id_bytes[0]))
	id_bytes_sizeC := (*C.int)(unsafe.Pointer(&id_bytes_size))
	C.VBK_VTB_getId(vtb_bytesC, C.int(len(vtb_bytes)), id_bytesC, id_bytes_sizeC)

	return id_bytes
}

func Atv_getId(atv_bytes []byte) []byte {
	id_bytes := make([]byte, veriblock.Sha256HashSize)
	var id_bytes_size int

	atv_bytesC := (*C.uint8_t)(unsafe.Pointer(&atv_bytes[0]))
	id_bytesC := (*C.uint8_t)(unsafe.Pointer(&id_bytes[0]))
	id_bytes_sizeC := (*C.int)(unsafe.Pointer(&id_bytes_size))
	C.VBK_ATV_getId(atv_bytesC, C.int(len(atv_bytes)), id_bytesC, id_bytes_sizeC)

	return id_bytes
}

func VbkBlock_getHash(block_bytes []byte) []byte {
	hash_bytes := make([]byte, veriblock.VblakeBlockHashSize)
	var hash_bytes_size int

	block_bytesC := (*C.uint8_t)(unsafe.Pointer(&block_bytes[0]))
	hash_bytesC := (*C.uint8_t)(unsafe.Pointer(&hash_bytes[0]))
	hash_bytes_sizeC := (*C.int)(unsafe.Pointer(&hash_bytes_size))
	C.VBK_VbkBlock_getHash(block_bytesC, C.int(len(block_bytes)), hash_bytesC, hash_bytes_sizeC)

	return hash_bytes
}

func BtcBlock_getHash(block_bytes []byte) []byte {
	hash_bytes := make([]byte, veriblock.Sha256HashSize)
	var hash_bytes_size int

	block_bytesC := (*C.uint8_t)(unsafe.Pointer(&block_bytes[0]))
	hash_bytesC := (*C.uint8_t)(unsafe.Pointer(&hash_bytes[0]))
	hash_bytes_sizeC := (*C.int)(unsafe.Pointer(&hash_bytes_size))
	C.VBK_BtcBlock_getHash(block_bytesC, C.int(len(block_bytes)), hash_bytesC, hash_bytes_sizeC)

	return hash_bytes
}
