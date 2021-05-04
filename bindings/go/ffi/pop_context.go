// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/pop_context.h>
import "C"
import (
	"runtime"
	"unsafe"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
)

// PopContext ...
type PopContext struct {
	ref     *C.PopContext
	popData []byte
}

// NewPopContext ...
func NewPopContext(config *Config, storage *Storage, log_lvl string) *PopContext {
	if config == nil {
		panic("Config not provided")
	}
	if storage == nil {
		panic("Storage not provided")
	}

	context := &PopContext{
		ref:     C.VBK_NewPopContext(config.ref, storage.ref, C.CString(log_lvl)),
		popData: make([]byte, config.GetMaxPopDataSize()),
	}
	runtime.SetFinalizer(context, func(v *PopContext) {
		v.Free()
	})
	return context
}

// Free - Dealocates memory allocated for the context.
func (v *PopContext) Free() {
	if v.ref != nil {
		C.VBK_FreePopContext(v.ref)
		v.ref = nil
	}
}

// AltBlockTreeAcceptBlockHeader - return true if block is valid, and added; false otherwise.
func (v *PopContext) AltBlockTreeAcceptBlockHeader(blockBytes []byte, state *ValidationState) bool {
	valsC := (*C.uint8_t)(unsafe.Pointer(&blockBytes[0]))
	res := C.VBK_AltBlockTree_acceptBlockHeader(v.ref, valsC, C.int(len(blockBytes)), state.ref)
	return bool(res)
}

// AltBlockTreeAcceptBlock - POP payloads stored in this block.
func (v *PopContext) AltBlockTreeAcceptBlock(hashBytes []byte, payloadsBytes []byte, state *ValidationState) {
	hashBytesC := (*C.uint8_t)(unsafe.Pointer(&hashBytes[0]))
	payloadsBytesC := (*C.uint8_t)(unsafe.Pointer(&payloadsBytes[0]))
	C.VBK_AltBlockTree_acceptBlock(v.ref, hashBytesC, C.int(len(hashBytes)), payloadsBytesC, C.int(len(payloadsBytes)), state.ref)
}

// AltBlockTreeComparePopScore ...
func (v *PopContext) AltBlockTreeComparePopScore(hashBytesA []byte, hashBytesB []byte) int {
	hashBytesAC := (*C.uint8_t)(unsafe.Pointer(&hashBytesA[0]))
	hashBytesBC := (*C.uint8_t)(unsafe.Pointer(&hashBytesB[0]))
	res := C.VBK_AltBlockTree_comparePopScore(v.ref, hashBytesAC, C.int(len(hashBytesA)), hashBytesBC, C.int(len(hashBytesB)))
	return int(res)
}

// AltBlockTreeRemoveSubtree ...
func (v *PopContext) AltBlockTreeRemoveSubtree(hashBytes []byte) {
	hashBytesC := (*C.uint8_t)(unsafe.Pointer(&hashBytes[0]))
	C.VBK_AltBlockTree_removeSubtree(v.ref, hashBytesC, C.int(len(hashBytes)))
}

// AltBlockTreeSetState return `false` if intermediate or target block is invalid. In this
// case tree will rollback into original state. `true` if state change is successful.
func (v *PopContext) AltBlockTreeSetState(hashBytes []byte, state *ValidationState) bool {
	hashBytesC := (*C.uint8_t)(unsafe.Pointer(&hashBytes[0]))
	res := C.VBK_AltBlockTree_setState(v.ref, hashBytesC, C.int(len(hashBytes)), state.ref)
	return bool(res)
}

// AltBlockTreeGetPopPayout return PopPayouts serialized to the VbkByteStream
func (v *PopContext) AltBlockTreeGetPopPayout(tipHashBytes []byte) *VbkByteStream {
	tipHashBytesC := (*C.uint8_t)(unsafe.Pointer(&tipHashBytes[0]))
	return NewVbkByteStream(C.VBK_AltBlockTree_getPopPayout(v.ref, tipHashBytesC, C.int(len(tipHashBytes))))
}

// BtcGetBlockIndex ...
func (v *PopContext) BtcGetBlockIndex(hashBytes [veriblock.Sha256HashSize]byte) *VbkByteStream {
	hashBytesC := (*C.uint8_t)(unsafe.Pointer(&hashBytes[0]))
	return NewVbkByteStream(C.VBK_btc_getBlockIndex(v.ref, hashBytesC, C.int(len(hashBytes))))
}

// VbkGetBlockIndex ...
func (v *PopContext) VbkGetBlockIndex(hashBytes [veriblock.VblakeBlockHashSize]byte) *VbkByteStream {
	hashBytesC := (*C.uint8_t)(unsafe.Pointer(&hashBytes[0]))
	return NewVbkByteStream(C.VBK_vbk_getBlockIndex(v.ref, hashBytesC, C.int(len(hashBytes))))
}

// AltGetBlockIndex ...
func (v *PopContext) AltGetBlockIndex(hashBytes []byte) *VbkByteStream {
	hashBytesC := (*C.uint8_t)(unsafe.Pointer(&hashBytes[0]))
	return NewVbkByteStream(C.VBK_alt_getBlockIndex(v.ref, hashBytesC, C.int(len(hashBytes))))
}

// AltBestBlock ...
func (v *PopContext) AltBestBlock() *VbkByteStream {
	return NewVbkByteStream(C.VBK_alt_BestBlock(v.ref))
}

// VbkBestBlock ...
func (v *PopContext) VbkBestBlock() *VbkByteStream {
	return NewVbkByteStream(C.VBK_vbk_BestBlock(v.ref))
}

// BtcBestBlock ...
func (v *PopContext) BtcBestBlock() *VbkByteStream {
	return NewVbkByteStream(C.VBK_btc_BestBlock(v.ref))
}

// VbkBootstrapBlock ...
func (v *PopContext) VbkBootstrapBlock() *VbkByteStream {
	return NewVbkByteStream(C.VBK_vbk_BootstrapBlock(v.ref))
}

// BtcBootstrapBlock ...
func (v *PopContext) BtcBootstrapBlock() *VbkByteStream {
	return NewVbkByteStream(C.VBK_btc_BootstrapBlock(v.ref))
}

// AltBlockAtActiveChainByHeight ...
func (v *PopContext) AltBlockAtActiveChainByHeight(height int) *VbkByteStream {
	return NewVbkByteStream(C.VBK_alt_BlockAtActiveChainByHeight(v.ref, C.int(height)))
}

// VbkBlockAtActiveChainByHeight ...
func (v *PopContext) VbkBlockAtActiveChainByHeight(height int) *VbkByteStream {
	return NewVbkByteStream(C.VBK_vbk_BlockAtActiveChainByHeight(v.ref, C.int(height)))
}

// BtcBlockAtActiveChainByHeight ...
func (v *PopContext) BtcBlockAtActiveChainByHeight(height int) *VbkByteStream {
	return NewVbkByteStream(C.VBK_btc_BlockAtActiveChainByHeight(v.ref, C.int(height)))
}

// AltGetAtvContainingBlock ...
func (v *PopContext) AltGetAtvContainingBlock(atvID [veriblock.Sha256HashSize]byte) *VbkByteStream {
	atvIDC := (*C.uint8_t)(unsafe.Pointer(&atvID[0]))
	return NewVbkByteStream(C.VBK_alt_getATVContainingBlock(v.ref, atvIDC, C.int(len(atvID))))
}

// AltGetVtbContainingBlock ...
func (v *PopContext) AltGetVtbContainingBlock(vtbID [veriblock.Sha256HashSize]byte) *VbkByteStream {
	vtbIDC := (*C.uint8_t)(unsafe.Pointer(&vtbID[0]))
	return NewVbkByteStream(C.VBK_alt_getVTBContainingBlock(v.ref, vtbIDC, C.int(len(vtbID))))
}

// AltGetVbkBlockContainingBlock ...
func (v *PopContext) AltGetVbkBlockContainingBlock(vbkID [veriblock.VblakePreviousBlockHashSize]byte) *VbkByteStream {
	vbkIDC := (*C.uint8_t)(unsafe.Pointer(&vbkID[0]))
	return NewVbkByteStream(C.VBK_alt_getVbkBlockContainingBlock(v.ref, vbkIDC, C.int(len(vbkID))))
}

// VbkGetVtbContainingBlock ...
func (v *PopContext) VbkGetVtbContainingBlock(vtbID [veriblock.Sha256HashSize]byte) *VbkByteStream {
	vtbIDC := (*C.uint8_t)(unsafe.Pointer(&vtbID[0]))
	return NewVbkByteStream(C.VBK_vbk_getVTBContainingBlock(v.ref, vtbIDC, C.int(len(vtbID))))
}

// MemPoolSubmitAtv - returns true if payload is valid, false otherwise.
func (v *PopContext) MemPoolSubmitAtv(bytes []byte, state *ValidationState) int {
	bytesC := (*C.uint8_t)(unsafe.Pointer(&bytes[0]))
	res := C.VBK_MemPool_submit_atv(v.ref, bytesC, C.int(len(bytes)), state.ref)
	return int(res)
}

// MemPoolSubmitVtb - returns true if payload is valid, false otherwise.
func (v *PopContext) MemPoolSubmitVtb(bytes []byte, state *ValidationState) int {
	bytesC := (*C.uint8_t)(unsafe.Pointer(&bytes[0]))
	res := C.VBK_MemPool_submit_vtb(v.ref, bytesC, C.int(len(bytes)), state.ref)
	return int(res)
}

// MemPoolSubmitVbk - returns true if payload is valid, false otherwise.
func (v *PopContext) MemPoolSubmitVbk(bytes []byte, state *ValidationState) int {
	bytesC := (*C.uint8_t)(unsafe.Pointer(&bytes[0]))
	res := C.VBK_MemPool_submit_vbk(v.ref, bytesC, C.int(len(bytes)), state.ref)
	return int(res)
}

// MemPoolGetPop ...
func (v *PopContext) MemPoolGetPop() []byte {
	var bytesSize int
	bytesC := (*C.uint8_t)(unsafe.Pointer(&v.popData[0]))
	bytesSizeC := (*C.int)(unsafe.Pointer(&bytesSize))
	if v.ref == nil {
		panic("CGO ref on PopContext is nil!")
	}
	C.VBK_MemPool_getPop(v.ref, bytesC, bytesSizeC)
	out := make([]byte, bytesSize)
	copy(out, v.popData)
	return out
}

// MemPoolRemoveAll ...
func (v *PopContext) MemPoolRemoveAll(bytes []byte, state *ValidationState) {
	bytesC := (*C.uint8_t)(unsafe.Pointer(&bytes[0]))
	C.VBK_MemPool_removeAll(v.ref, bytesC, C.int(len(bytes)), state.ref)
}

// MemPoolGetAtv ...
func (v *PopContext) MemPoolGetAtv(atvID [veriblock.Sha256HashSize]byte) *VbkByteStream {
	idBytesC := (*C.uint8_t)(unsafe.Pointer(&atvID[0]))
	return NewVbkByteStream(C.VBK_MemPool_GetATV(v.ref, idBytesC, C.int(len(atvID))))
}

// MemPoolGetVtb ...
func (v *PopContext) MemPoolGetVtb(vtbID [veriblock.Sha256HashSize]byte) *VbkByteStream {
	idBytesC := (*C.uint8_t)(unsafe.Pointer(&vtbID[0]))
	return NewVbkByteStream(C.VBK_MemPool_GetVTB(v.ref, idBytesC, C.int(len(vtbID))))
}

// MemPoolGetVbkBlock ...
func (v *PopContext) MemPoolGetVbkBlock(vbkID [veriblock.VblakePreviousBlockHashSize]byte) *VbkByteStream {
	idBytesC := (*C.uint8_t)(unsafe.Pointer(&vbkID[0]))
	return NewVbkByteStream(C.VBK_MemPool_GetVbkBlock(v.ref, idBytesC, C.int(len(vbkID))))
}

// MemPoolGetAtvs ...
func (v *PopContext) MemPoolGetAtvs() *VbkByteStream {
	return NewVbkByteStream(C.VBK_MemPool_GetATVs(v.ref))
}

// MemPoolGetVtbs ...
func (v *PopContext) MemPoolGetVtbs() *VbkByteStream {
	return NewVbkByteStream(C.VBK_MemPool_GetVTBs(v.ref))
}

// MemPoolGetVbkBlocks ...
func (v *PopContext) MemPoolGetVbkBlocks() *VbkByteStream {
	return NewVbkByteStream(C.VBK_MemPool_GetVbkBlocks(v.ref))
}

// MemPoolGetAtvsInFlight ...
func (v *PopContext) MemPoolGetAtvsInFlight() *VbkByteStream {
	return NewVbkByteStream(C.VBK_MemPool_GetATVsInFlight(v.ref))
}

// MemPoolGetVtbsInFlight ...
func (v *PopContext) MemPoolGetVtbsInFlight() *VbkByteStream {
	return NewVbkByteStream(C.VBK_MemPool_GetVTBsInFlight(v.ref))
}

// MemPoolGetVbkBlocksInFlight ...
func (v *PopContext) MemPoolGetVbkBlocksInFlight() *VbkByteStream {
	return NewVbkByteStream(C.VBK_MemPool_GetVbkBlocksInFlight(v.ref))
}

// MemPoolClear ...
func (v *PopContext) MemPoolClear() {
	C.VBK_MemPool_clear(v.ref)
}
