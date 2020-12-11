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
func VbkBlockGetID(blockBytes []byte) [veriblock.VblakePreviousBlockHashSize]byte {
	var idBytes [veriblock.VblakePreviousBlockHashSize]byte
	var idBytesSize int
	blockBytesC := (*C.uint8_t)(unsafe.Pointer(&blockBytes[0]))
	idBytesC := (*C.uint8_t)(unsafe.Pointer(&idBytes[0]))
	idBytesSizeC := (*C.int)(unsafe.Pointer(&idBytesSize))
	C.VBK_VbkBlock_getId(blockBytesC, C.int(len(blockBytes)), idBytesC, idBytesSizeC)
	if idBytesSize != len(idBytes) {
		panic("invalid bytes size")
	}
	return idBytes
}

// VtbGetID ...
func VtbGetID(vtbBytes []byte) [veriblock.Sha256HashSize]byte {
	var idBytes [veriblock.Sha256HashSize]byte
	var idBytesSize int
	vtbBytesC := (*C.uint8_t)(unsafe.Pointer(&vtbBytes[0]))
	idBytesC := (*C.uint8_t)(unsafe.Pointer(&idBytes[0]))
	idBytesSizeC := (*C.int)(unsafe.Pointer(&idBytesSize))
	C.VBK_VTB_getId(vtbBytesC, C.int(len(vtbBytes)), idBytesC, idBytesSizeC)
	if idBytesSize != len(idBytes) {
		panic("invalid bytes size")
	}
	return idBytes
}

// AtvGetID ...
func AtvGetID(atvBytes []byte) [veriblock.Sha256HashSize]byte {
	var idBytes [veriblock.Sha256HashSize]byte
	var idBytesSize int
	atvBytesC := (*C.uint8_t)(unsafe.Pointer(&atvBytes[0]))
	idBytesC := (*C.uint8_t)(unsafe.Pointer(&idBytes[0]))
	idBytesSizeC := (*C.int)(unsafe.Pointer(&idBytesSize))
	C.VBK_ATV_getId(atvBytesC, C.int(len(atvBytes)), idBytesC, idBytesSizeC)
	if idBytesSize != len(idBytes) {
		panic("invalid bytes size")
	}
	return idBytes
}

// VbkBlockGetHash ...
func VbkBlockGetHash(blockBytes []byte) [veriblock.VblakeBlockHashSize]byte {
	var hashBytes [veriblock.VblakeBlockHashSize]byte
	var hashBytesSize int
	blockBytesC := (*C.uint8_t)(unsafe.Pointer(&blockBytes[0]))
	hashBytesC := (*C.uint8_t)(unsafe.Pointer(&hashBytes[0]))
	hashBytesSizeC := (*C.int)(unsafe.Pointer(&hashBytesSize))
	C.VBK_VbkBlock_getHash(blockBytesC, C.int(len(blockBytes)), hashBytesC, hashBytesSizeC)
	if hashBytesSize != len(hashBytes) {
		panic("invalid bytes size")
	}
	return hashBytes
}

// BtcBlockGetHash ...
func BtcBlockGetHash(blockBytes []byte) [veriblock.Sha256HashSize]byte {
	var hashBytes [veriblock.Sha256HashSize]byte
	var hashBytesSize int
	blockBytesC := (*C.uint8_t)(unsafe.Pointer(&blockBytes[0]))
	hashBytesC := (*C.uint8_t)(unsafe.Pointer(&hashBytes[0]))
	hashBytesSizeC := (*C.int)(unsafe.Pointer(&hashBytesSize))
	C.VBK_BtcBlock_getHash(blockBytesC, C.int(len(blockBytes)), hashBytesC, hashBytesSizeC)
	if hashBytesSize != len(hashBytes) {
		panic("invalid bytes size")
	}
	return hashBytes
}

// AltBlockCalculateContextInfoContainerHash ...
func (v *PopContext) AltBlockCalculateContextInfoContainerHash(prevBlockHash []byte, popDataBytes []byte) [veriblock.Sha256HashSize]byte {
	var hashBytesSize int
	var hashBytes [veriblock.Sha256HashSize]byte
	hashBytesC := (*C.uint8_t)(unsafe.Pointer(&hashBytes[0]))
	hashBytesSizeC := (*C.int)(unsafe.Pointer(&hashBytesSize))
	prevBlockHashC := (*C.uint8_t)(unsafe.Pointer(&prevBlockHash[0]))
	popDataBytesC := (*C.uint8_t)(unsafe.Pointer(&popDataBytes[0]))
	C.VBK_AltBlock_calculateContextInfoContainerHash(v.ref, prevBlockHashC, C.int(len(prevBlockHash)), popDataBytesC, C.int(len(popDataBytes)), hashBytesC, hashBytesSizeC)
	if hashBytesSize != len(hashBytes) {
		panic("invalid bytes size")
	}
	return hashBytes
}

// checkATV ...
func (v *PopContext) CheckATV(atv_bytes []byte) (bool, *ValidationState) {
	atvBytesC := (*C.uint8_t)(unsafe.Pointer(&atv_bytes[0]))
	state := NewValidationState()
	res := C.VBK_checkATV(v.ref, atvBytesC, C.int(len(atv_bytes)), state.ref)
	return bool(res), &state
}

// checkVTB ...
func (v *PopContext) CheckVTB(vtb_bytes []byte) (bool, *ValidationState) {
	vtbBytesC := (*C.uint8_t)(unsafe.Pointer(&vtb_bytes[0]))
	state := NewValidationState()
	res := C.VBK_checkVTB(v.ref, vtbBytesC, C.int(len(vtb_bytes)), state.ref)
	return bool(res), &state
}

// checkVbkBlock ...
func (v *PopContext) CheckVbkBlock(vbk_bytes []byte) (bool, *ValidationState) {
	vbkBytesC := (*C.uint8_t)(unsafe.Pointer(&vbk_bytes[0]))
	state := NewValidationState()
	res := C.VBK_checkVbkBlock(v.ref, vbkBytesC, C.int(len(vbk_bytes)), state.ref)
	return bool(res), &state
}

// checkPopData ...
func (v *PopContext) CheckPopData(pop_data_bytes []byte) (bool, *ValidationState) {
	popDataBytesC := (*C.uint8_t)(unsafe.Pointer(&pop_data_bytes[0]))
	state := NewValidationState()
	res := C.VBK_checkPopData(v.ref, popDataBytesC, C.int(len(pop_data_bytes)), state.ref)
	return bool(res), &state
}
