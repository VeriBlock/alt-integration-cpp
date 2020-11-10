package api

import (
	"bytes"
	"errors"
	"sync"

	entities "github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
	ffi "github.com/VeriBlock/alt-integration-cpp/bindings/go/ffi"
)

// AltBlockTree ...
type AltBlockTree interface {
	AcceptBlockHeader(block entities.AltBlock) error
	AcceptBlock(hash []byte, payloads entities.PopData) error
	AddPayloads(hash []byte, payloads entities.PopData) error
	LoadTip(hash []byte)
	ComparePopScore(hashA []byte, hashB []byte) int
	RemoveSubtree(hash []byte)
	SetState(hash []byte) error
}

// MemPool ...
type MemPool interface {
	SubmitAtv(block entities.Atv) error
	SubmitVtb(block entities.Vtb) error
	SubmitVbk(block entities.VbkBlock) error
	GetPop() (*entities.PopData, error)
	RemoveAll(payloads entities.PopData) error
}

// PopContext ...
type PopContext struct {
	popContext ffi.PopContext

	mutex *sync.Mutex
}

// NewPopContext ...
func NewPopContext(config *Config) PopContext {
	if config == nil {
		panic("Config not provided")
	}
	return PopContext{
		popContext: ffi.NewPopContext(config.Config),
		mutex:      new(sync.Mutex),
	}
}

// Free - Frees memory allocated for the pop context
func (v *PopContext) Free() {
	v.popContext.Free()
}

// AcceptBlockHeader - Returns nil if block is valid, and added
func (v *PopContext) AcceptBlockHeader(block entities.AltBlock) error {
	stream := new(bytes.Buffer)
	err := block.ToVbkEncoding(stream)
	if err != nil {
		return err
	}
	defer v.lock()()
	ok := v.popContext.AltBlockTreeAcceptBlockHeader(stream.Bytes())
	if !ok {
		return errors.New("Failed to validate and add block header")
	}
	return nil
}

// AcceptBlock - POP payloads stored in this block
func (v *PopContext) AcceptBlock(hash []byte, payloads entities.PopData) error {
	stream := new(bytes.Buffer)
	err := payloads.ToVbkEncoding(stream)
	if err != nil {
		return err
	}
	defer v.lock()()
	v.popContext.AltBlockTreeAcceptBlock(hash, stream.Bytes())
	return nil
}

// AddPayloads - Returns nil if PopData does not contain duplicates (searched across active chain).
// However, it is far from certain that it is completely valid
func (v *PopContext) AddPayloads(hash []byte, payloads entities.PopData) error {
	stream := new(bytes.Buffer)
	err := payloads.ToVbkEncoding(stream)
	if err != nil {
		return err
	}
	defer v.lock()()
	v.popContext.AltBlockTreeAddPayloads(hash, stream.Bytes())
	return nil
}

// LoadTip ...
func (v PopContext) LoadTip(hash []byte) {
	defer v.lock()()
	v.popContext.AltBlockTreeLoadTip(hash)
}

// ComparePopScore ...
func (v PopContext) ComparePopScore(hashA []byte, hashB []byte) int {
	defer v.lock()()
	return v.popContext.AltBlockTreeComparePopScore(hashA, hashB)
}

// RemoveSubtree ...
func (v PopContext) RemoveSubtree(hash []byte) {
	defer v.lock()()
	v.popContext.AltBlockTreeRemoveSubtree(hash)
}

// SetState - Return `false` if intermediate or target block is invalid. In this
// case tree will rollback into original state. `true` if state change is successful.
func (v PopContext) SetState(hash []byte) error {
	defer v.lock()()
	ok := v.popContext.AltBlockTreeSetState(hash)
	if !ok {
		return errors.New("Intermediate or target block is invalid")
	}
	return nil
}

// // BtcGetBlockIndex ...
// func (v PopContext) BtcGetBlockIndex(hashBytes []byte) {
// 	hashBytesC := (*C.uint8_t)(unsafe.Pointer(&hashBytes[0]))
// 	blockindexC := (*C.uint8_t)(unsafe.Pointer(nil))
// 	var blockindexSize int = 0
// 	blockindexSizeC := (*C.int)(unsafe.Pointer(&blockindexSize))
// 	res := C.VBK_btc_getBlockIndex(v.ref, hashBytesC, C.int(len(hashBytes)), &blockindexC, blockindexSizeC)
// 	if !bool(res) {
// 		return
// 	}
// 	// blockindex := make([]byte, blockindexSize)
// 	// TODO: Retrieve data from blockindex with blockindexSize
// }

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

// SubmitAtv - Returns nil if payload is valid
func (v *PopContext) SubmitAtv(block entities.Atv) error {
	stream := new(bytes.Buffer)
	err := block.ToVbkEncoding(stream)
	if err != nil {
		return err
	}
	defer v.lock()()
	ok := v.popContext.MemPoolSubmitAtv(stream.Bytes())
	if !ok {
		return errors.New("Payload is invalid")
	}
	return nil
}

// SubmitVtb - Returns nil if payload is valid
func (v *PopContext) SubmitVtb(block entities.Vtb) error {
	stream := new(bytes.Buffer)
	err := block.ToVbkEncoding(stream)
	if err != nil {
		return err
	}
	defer v.lock()()
	ok := v.popContext.MemPoolSubmitVtb(stream.Bytes())
	if !ok {
		return errors.New("Payload is invalid")
	}
	return nil
}

// SubmitVbk - Returns nil if payload is valid
func (v *PopContext) SubmitVbk(block entities.VbkBlock) error {
	stream := new(bytes.Buffer)
	err := block.ToVbkEncoding(stream)
	if err != nil {
		return err
	}
	defer v.lock()()
	ok := v.popContext.MemPoolSubmitVbk(stream.Bytes())
	if !ok {
		return errors.New("Payload is invalid")
	}
	return nil
}

// GetPop ...
func (v *PopContext) GetPop() (*entities.PopData, error) {
	defer v.lock()()
	popBytes := v.popContext.MemPoolGetPop()
	stream := bytes.NewReader(popBytes)
	pop := &entities.PopData{}
	err := pop.FromVbkEncoding(stream)
	if err != nil {
		return nil, err
	}
	return pop, nil
}

// RemoveAll - Returns nil if payload is valid
func (v *PopContext) RemoveAll(payloads entities.PopData) error {
	stream := new(bytes.Buffer)
	err := payloads.ToVbkEncoding(stream)
	if err != nil {
		return err
	}
	defer v.lock()()
	v.popContext.MemPoolRemoveAll(stream.Bytes())
	return nil
}

func (v *PopContext) lock() (unlock func()) {
	v.mutex.Lock()
	return func() {
		v.mutex.Unlock()
	}
}
