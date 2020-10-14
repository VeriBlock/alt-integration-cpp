package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++
// #include <veriblock/c/pop_context.h>
import "C"
import "unsafe"

// Exported functions
var (
	OnGetAltchainID      = func() int { panic("OnGetAltchainID not set") }
	OnGetBootstrapBlock  = func() string { panic("OnGetBootstrapBlock not set") }
	OnGetBlockHeaderHash = func(toBeHashed []byte) []byte { panic("OnGetBlockHeaderHash not set") }
	OnGetAtv             = func() { panic("OnGetAtv not set") }
	OnGetVtb             = func() { panic("OnGetVtb not set") }
	OnGetVbk             = func() { panic("OnGetVbk not set") }
)

// PopContext ...
type PopContext struct {
	Config *Config

	ref     *C.PopContext
	popData []byte
}

// NewPopContext ...
func NewPopContext(config *Config) PopContext {
	return PopContext{
		Config:  config,
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

// bool VBK_btc_getBlockIndex(PopContext* self,
//                            const uint8_t* hash_bytes,
//                            int hash_bytes_size,
//                            uint8_t** blockindex,
//                            int* blockindex_size);

// /**
//  * Find a VbkBlock index from the VbkTree
//  *
//  * @param[in] self PopContext
//  * @param[in] hash_bytes altintegration::VbkBlock hash bytes
//  * @param[in] hash_bytes_size size of input hash
//  * @param[out] blockindex pointer to the blockindex bytes (memory will allocated
//  * by this method)
//  * @param[out] blockindex_size blockindex bytes size
//  * @return `false` while block is not found. 'true' while block is found.
//  * @ingroup c-api
//  */
// bool VBK_vbk_getBlockIndex(PopContext* self,
//                            const uint8_t* hash_bytes,
//                            int hash_bytes_size,
//                            uint8_t** blockindex,
//                            int* blockindex_size);

// /**
//  * Find a AltBlock index from the AltTree
//  *
//  * @param[in] self PopContext
//  * @param[in] hash_bytes altintegration::AltBlock hash bytes
//  * @param[in] hash_bytes_size size of input hash
//  * @param[out] blockindex pointer to the blockindex bytes (memory will allocated
//  * by this method)
//  * @param[out] blockindex_size blockindex bytes size
//  * @return `false` while block is not found. 'true' while block is found.
//  * @ingroup c-api
//  */
// bool VBK_alt_getBlockIndex(PopContext* self,
//                            const uint8_t* hash_bytes,
//                            int hash_bytes_size,
//                            uint8_t** blockindex,
//                            int* blockindex_size);

// MemPoolSubmitAtv - returns true if payload is valid, false otherwise.
func (v PopContext) MemPoolSubmitAtv(bytes []byte) bool {
	bytesC := (*C.uint8_t)(unsafe.Pointer(&bytes[0]))
	res := C.VBK_MemPool_submit_atv(v.ref, bytesC, C.int(len(bytes)))
	return bool(res)
}

// MemPoolSubmitVtb - returns true if payload is valid, false otherwise.
func (v PopContext) MemPoolSubmitVtb(bytes []byte) bool {
	bytesC := (*C.uint8_t)(unsafe.Pointer(&bytes[0]))
	res := C.VBK_MemPool_submit_vtb(v.ref, bytesC, C.int(len(bytes)))
	return bool(res)
}

// MemPoolSubmitVbk - returns true if payload is valid, false otherwise.
func (v PopContext) MemPoolSubmitVbk(bytes []byte) bool {
	bytesC := (*C.uint8_t)(unsafe.Pointer(&bytes[0]))
	res := C.VBK_MemPool_submit_vbk(v.ref, bytesC, C.int(len(bytes)))
	return bool(res)
}

// MemPoolGetPop ...
func (v PopContext) MemPoolGetPop() []byte {
	var bytesSize int
	bytesC := (*C.uint8_t)(unsafe.Pointer(&v.popData[0]))
	bytesSizeC := (*C.int)(unsafe.Pointer(&bytesSize))
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

// MemPoolClear ...
func (v PopContext) MemPoolClear() {
	C.VBK_MemPool_clear(v.ref)
}
