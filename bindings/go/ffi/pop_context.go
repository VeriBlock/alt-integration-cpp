package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++
// #include <veriblock/c/pop_context.h>
import "C"
import "unsafe"

// PopContext ...
type PopContext struct {
	ref     *C.PopContext
	popData []byte
}

// NewPopContext ...
func NewPopContext(config *Config) PopContext {
	if config == nil {
		panic("Config not provided")
	}
	return PopContext{
		ref:     C.VBK_NewPopContext(config.ref),
		popData: make([]byte, config.GetMaxPopDataSize()),
	}
}

// Free ...
func (v PopContext) Free() { C.VBK_FreePopContext(v.ref) }

// AltBlockTreeAcceptBlockHeader - return true if block is valid, and added; false otherwise.
func (v PopContext) AltBlockTreeAcceptBlockHeader(blockBytes []byte) bool {
	valsC := (*C.uint8_t)(unsafe.Pointer(&blockBytes[0]))
	res := C.VBK_AltBlockTree_acceptBlockHeader(v.ref, valsC, C.int(len(blockBytes)))
	return bool(res)
}

// AltBlockTreeAcceptBlock - POP payloads stored in this block.
func (v PopContext) AltBlockTreeAcceptBlock(hashBytes []byte, payloadsBytes []byte) {
	hashBytesC := (*C.uint8_t)(unsafe.Pointer(&hashBytes[0]))
	payloadsBytesC := (*C.uint8_t)(unsafe.Pointer(&payloadsBytes[0]))
	C.VBK_AltBlockTree_acceptBlock(v.ref, hashBytesC, C.int(len(hashBytes)), payloadsBytesC, C.int(len(payloadsBytes)))
}

// AltBlockTreeAddPayloads - true if altintegration::PopData does not contain duplicates
// (searched across active chain). However, it is far from certain that it is completely valid.
func (v PopContext) AltBlockTreeAddPayloads(hashBytes []byte, payloadsBytes []byte) {
	hashBytesC := (*C.uint8_t)(unsafe.Pointer(&hashBytes[0]))
	payloadsBytesC := (*C.uint8_t)(unsafe.Pointer(&payloadsBytes[0]))
	C.VBK_AltBlockTree_addPayloads(v.ref, hashBytesC, C.int(len(hashBytes)), payloadsBytesC, C.int(len(payloadsBytes)))
}

// AltBlockTreeLoadTip - true on success, false otherwise.
func (v PopContext) AltBlockTreeLoadTip(hashBytes []byte) bool {
	hashBytesC := (*C.uint8_t)(unsafe.Pointer(&hashBytes[0]))
	res := C.VBK_AltBlockTree_loadTip(v.ref, hashBytesC, C.int(len(hashBytes)))
	return bool(res)
}

// AltBlockTreeComparePopScore ...
func (v PopContext) AltBlockTreeComparePopScore(hashBytesA []byte, hashBytesB []byte) int {
	hashBytesAC := (*C.uint8_t)(unsafe.Pointer(&hashBytesA[0]))
	hashBytesBC := (*C.uint8_t)(unsafe.Pointer(&hashBytesB[0]))
	res := C.VBK_AltBlockTree_comparePopScore(v.ref, hashBytesAC, C.int(len(hashBytesA)), hashBytesBC, C.int(len(hashBytesB)))
	return int(res)
}

// AltBlockTreeRemoveSubtree ...
func (v PopContext) AltBlockTreeRemoveSubtree(hashBytes []byte) {
	hashBytesC := (*C.uint8_t)(unsafe.Pointer(&hashBytes[0]))
	C.VBK_AltBlockTree_removeSubtree(v.ref, hashBytesC, C.int(len(hashBytes)))
}

// AltBlockTreeSetState return `false` if intermediate or target block is invalid. In this
// case tree will rollback into original state. `true` if state change is successful.
func (v PopContext) AltBlockTreeSetState(hashBytes []byte) bool {
	hashBytesC := (*C.uint8_t)(unsafe.Pointer(&hashBytes[0]))
	res := C.VBK_AltBlockTree_setState(v.ref, hashBytesC, C.int(len(hashBytes)))
	return bool(res)
}

// BtcGetBlockIndex ...
func (v PopContext) BtcGetBlockIndex(hashBytes []byte) {
	hashBytesC := (*C.uint8_t)(unsafe.Pointer(&hashBytes[0]))
	blockindexC := (*C.uint8_t)(unsafe.Pointer(nil))
	var blockindexSize int = 0
	blockindexSizeC := (*C.int)(unsafe.Pointer(&blockindexSize))
	res := C.VBK_btc_getBlockIndex(v.ref, hashBytesC, C.int(len(hashBytes)), &blockindexC, blockindexSizeC)
	if !bool(res) {
		return
	}
	// blockindex := make([]byte, blockindexSize)
	// TODO: Retrieve data from blockindex with blockindexSize
}

// MemPoolSubmitAtv - returns true if payload is valid, false otherwise.
func (v PopContext) MemPoolSubmitAtv(bytes []byte) int {
	bytesC := (*C.uint8_t)(unsafe.Pointer(&bytes[0]))
	res := C.VBK_MemPool_submit_atv(v.ref, bytesC, C.int(len(bytes)))
	return int(res)
}

// MemPoolSubmitVtb - returns true if payload is valid, false otherwise.
func (v PopContext) MemPoolSubmitVtb(bytes []byte) int {
	bytesC := (*C.uint8_t)(unsafe.Pointer(&bytes[0]))
	res := C.VBK_MemPool_submit_vtb(v.ref, bytesC, C.int(len(bytes)))
	return int(res)
}

// MemPoolSubmitVbk - returns true if payload is valid, false otherwise.
func (v PopContext) MemPoolSubmitVbk(bytes []byte) int {
	bytesC := (*C.uint8_t)(unsafe.Pointer(&bytes[0]))
	res := C.VBK_MemPool_submit_vbk(v.ref, bytesC, C.int(len(bytes)))
	return int(res)
}

// MemPoolGetPop ...
func (v PopContext) MemPoolGetPop() []byte {
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
func (v PopContext) MemPoolRemoveAll(bytes []byte) {
	bytesC := (*C.uint8_t)(unsafe.Pointer(&bytes[0]))
	C.VBK_MemPool_removeAll(v.ref, bytesC, C.int(len(bytes)))
}

func (v PopContext) MemPoolGetAtv(atv_id []byte) VbkByteStream {
	id_bytesC := (*C.uint8_t)(unsafe.Pointer(&atv_id[0]))
	return VbkByteStream{ref: C.VBK_MemPool_GetATV(v.ref, id_bytesC, C.int(len(atv_id)))}
}

func (v PopContext) MemPoolGetVtb(vtb_id []byte) VbkByteStream {
	id_bytesC := (*C.uint8_t)(unsafe.Pointer(&vtb_id[0]))
	return VbkByteStream{ref: C.VBK_MemPool_GetVTB(v.ref, id_bytesC, C.int(len(vtb_id)))}
}

func (v PopContext) MemPoolGetVbkBlock(vbk_id []byte) VbkByteStream {
	id_bytesC := (*C.uint8_t)(unsafe.Pointer(&vbk_id[0]))
	return VbkByteStream{ref: C.VBK_MemPool_GetVbkBlock(v.ref, id_bytesC, C.int(len(vbk_id)))}
}

func (v PopContext) MemPoolGetAtvs() VbkByteStream {
	return VbkByteStream{ref: C.VBK_MemPool_GetATVs(v.ref)}
}

func (v PopContext) MemPoolGetVtbs() VbkByteStream {
	return VbkByteStream{ref: C.VBK_MemPool_GetVTBs(v.ref)}
}

func (v PopContext) MemPoolGetVbkBlocks() VbkByteStream {
	return VbkByteStream{ref: C.VBK_MemPool_GetVbkBlocks(v.ref)}
}

func (v PopContext) MemPoolGetAtvsInFlight() VbkByteStream {
	return VbkByteStream{ref: C.VBK_MemPool_GetATVsInFlight(v.ref)}
}

func (v PopContext) MemPoolGetVtbsInFlight() VbkByteStream {
	return VbkByteStream{ref: C.VBK_MemPool_GetVTBsInFlight(v.ref)}
}

func (v PopContext) MemPoolGetVbkBlocksInFlight() VbkByteStream {
	return VbkByteStream{ref: C.VBK_MemPool_GetVbkBlocksInFlight(v.ref)}
}

// MemPoolClear ...
func (v PopContext) MemPoolClear() {
	C.VBK_MemPool_clear(v.ref)
}
