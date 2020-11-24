package api

import (
	"bytes"
	"errors"
	"io"
	"math"
	"sync"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
	entities "github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
	"github.com/VeriBlock/alt-integration-cpp/bindings/go/ffi"
)

// AltBlockTree ...
type AltBlockTree interface {
	AcceptBlockHeader(block *entities.AltBlock) error
	AcceptBlock(hash entities.AltHash, payloads *entities.PopData) error
	AddPayloads(hash entities.AltHash, payloads *entities.PopData) error
	LoadTip(hash entities.AltHash)
	ComparePopScore(hashA entities.AltHash, hashB entities.AltHash) int
	RemoveSubtree(hash entities.AltHash)
	SetState(hash entities.AltHash) error
	AltBestBlock() (*entities.BlockIndex, error)
	VbkBestBlock() (*entities.BlockIndex, error)
	BtcBestBlock() (*entities.BlockIndex, error)
	AltBlockAtActiveChainByHeight(height int) (*entities.BlockIndex, error)
	VbkBlockAtActiveChainByHeight(height int) (*entities.BlockIndex, error)
	BtcBlockAtActiveChainByHeight(height int) (*entities.BlockIndex, error)
	AltGetAtvContainingBlock(atvID entities.AtvID) ([]entities.AltHash, error)
	AltGetVtbContainingBlock(vtbID entities.VtbID) ([]entities.AltHash, error)
	AltGetVbkBlockContainingBlock(vbkID entities.VbkID) ([]entities.AltHash, error)
	VbkGetVtbContainingBlock(vtbID entities.VtbID) ([]entities.VbkID, error)
}

// Code for interface test
var _ AltBlockTree = &PopContext{}

// MemPool ...
type MemPool interface {
	SubmitAtv(block *entities.Atv) int
	SubmitVtb(block *entities.Vtb) int
	SubmitVbk(block *entities.VbkBlock) int
	GetPop() (*entities.PopData, error)
	RemoveAll(payloads *entities.PopData) error
	GetAtv(id entities.AtvID) (*entities.Atv, error)
	GetVtb(id entities.VtbID) (*entities.Vtb, error)
	GetVbkBlock(id entities.VbkID) (*entities.VbkBlock, error)
	GetAtvs() ([]entities.AtvID, error)
	GetVtbs() ([]entities.VtbID, error)
	GetVbkBlocks() ([]entities.VbkID, error)
	GetAtvsInFlight() ([]entities.AtvID, error)
	GetVtbsInFlight() ([]entities.VtbID, error)
	GetVbkBlocksInFlight() ([]entities.VbkID, error)
}

// Code for interface test
var _ MemPool = &PopContext{}

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
func (v *PopContext) AcceptBlockHeader(block *entities.AltBlock) error {
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
func (v *PopContext) AcceptBlock(hash entities.AltHash, payloads *entities.PopData) error {
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
func (v *PopContext) AddPayloads(hash entities.AltHash, payloads *entities.PopData) error {
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
func (v *PopContext) LoadTip(hash entities.AltHash) {
	defer v.lock()()
	v.popContext.AltBlockTreeLoadTip(hash)
}

// ComparePopScore ...
func (v *PopContext) ComparePopScore(hashA entities.AltHash, hashB entities.AltHash) int {
	defer v.lock()()
	return v.popContext.AltBlockTreeComparePopScore(hashA, hashB)
}

// RemoveSubtree ...
func (v *PopContext) RemoveSubtree(hash entities.AltHash) {
	defer v.lock()()
	v.popContext.AltBlockTreeRemoveSubtree(hash)
}

// SetState - Return `false` if intermediate or target block is invalid. In this
// case tree will rollback into original state. `true` if state change is successful.
func (v *PopContext) SetState(hash entities.AltHash) error {
	defer v.lock()()
	ok := v.popContext.AltBlockTreeSetState(hash)
	if !ok {
		return errors.New("Intermediate or target block is invalid")
	}
	return nil
}

// AltBestBlock ...
func (v *PopContext) AltBestBlock() (*entities.BlockIndex, error) {
	defer v.lock()()
	stream := v.popContext.AltBestBlock()
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	blockIndex := entities.NewAltBlockIndex()
	if err := blockIndex.FromRaw(stream); err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// VbkBestBlock ...
func (v *PopContext) VbkBestBlock() (*entities.BlockIndex, error) {
	defer v.lock()()
	stream := v.popContext.VbkBestBlock()
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	blockIndex := entities.NewVbkBlockIndex()
	if err := blockIndex.FromRaw(stream); err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// BtcBestBlock ...
func (v *PopContext) BtcBestBlock() (*entities.BlockIndex, error) {
	defer v.lock()()
	stream := v.popContext.BtcBestBlock()
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	blockIndex := entities.NewBtcBlockIndex()
	if err := blockIndex.FromRaw(stream); err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// AltBlockAtActiveChainByHeight ...
func (v *PopContext) AltBlockAtActiveChainByHeight(height int) (*entities.BlockIndex, error) {
	defer v.lock()()
	stream := v.popContext.AltBlockAtActiveChainByHeight(height)
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	blockIndex := entities.NewAltBlockIndex()
	if err := blockIndex.FromRaw(stream); err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// VbkBlockAtActiveChainByHeight ...
func (v *PopContext) VbkBlockAtActiveChainByHeight(height int) (*entities.BlockIndex, error) {
	defer v.lock()()
	stream := v.popContext.VbkBlockAtActiveChainByHeight(height)
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	blockIndex := entities.NewVbkBlockIndex()
	if err := blockIndex.FromRaw(stream); err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// BtcBlockAtActiveChainByHeight ...
func (v *PopContext) BtcBlockAtActiveChainByHeight(height int) (*entities.BlockIndex, error) {
	defer v.lock()()
	stream := v.popContext.BtcBlockAtActiveChainByHeight(height)
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	blockIndex := entities.NewBtcBlockIndex()
	if err := blockIndex.FromRaw(stream); err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// AltGetAtvContainingBlock ...
func (v *PopContext) AltGetAtvContainingBlock(atvID entities.AtvID) ([]entities.AltHash, error) {
	defer v.lock()()
	stream := v.popContext.AltGetAtvContainingBlock(atvID)
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	encodedHashes, err := veriblock.ReadArrayOf(stream, 0, math.MaxInt64, func(r io.Reader) (interface{}, error) {
		return veriblock.ReadSingleByteLenValueDefault(r)
	})
	if err != nil {
		return make([]entities.AltHash, 0), err
	}
	hashes := make([]entities.AltHash, len(encodedHashes))
	for i, encodedHash := range encodedHashes {
		copy(hashes[i][:], encodedHash.([]byte))
	}
	return hashes, nil
}

// AltGetVtbContainingBlock ...
func (v *PopContext) AltGetVtbContainingBlock(vtbID entities.VtbID) ([]entities.AltHash, error) {
	defer v.lock()()
	stream := v.popContext.AltGetVtbContainingBlock(vtbID)
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	encodedHashes, err := veriblock.ReadArrayOf(stream, 0, math.MaxInt64, func(r io.Reader) (interface{}, error) {
		return veriblock.ReadSingleByteLenValueDefault(r)
	})
	if err != nil {
		return make([]entities.AltHash, 0), err
	}
	hashes := make([]entities.AltHash, len(encodedHashes))
	for i, encodedHash := range encodedHashes {
		copy(hashes[i][:], encodedHash.([]byte))
	}
	return hashes, nil
}

// AltGetVbkBlockContainingBlock ...
func (v *PopContext) AltGetVbkBlockContainingBlock(vbkID entities.VbkID) ([]entities.AltHash, error) {
	defer v.lock()()
	stream := v.popContext.AltGetVbkBlockContainingBlock(vbkID)
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	encodedHashes, err := veriblock.ReadArrayOf(stream, 0, math.MaxInt64, func(r io.Reader) (interface{}, error) {
		return veriblock.ReadSingleByteLenValueDefault(r)
	})
	if err != nil {
		return make([]entities.AltHash, 0), err
	}
	hashes := make([]entities.AltHash, len(encodedHashes))
	for i, encodedHash := range encodedHashes {
		copy(hashes[i][:], encodedHash.([]byte))
	}
	return hashes, nil
}

// VbkGetVtbContainingBlock ...
func (v *PopContext) VbkGetVtbContainingBlock(vtbID entities.VtbID) ([]entities.VbkID, error) {
	defer v.lock()()
	stream := v.popContext.VbkGetVtbContainingBlock(vtbID)
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	encodedHashes, err := veriblock.ReadArrayOf(stream, 0, math.MaxInt64, func(r io.Reader) (interface{}, error) {
		return veriblock.ReadSingleByteLenValueDefault(r)
	})
	if err != nil {
		return make([]entities.VbkID, 0), err
	}
	hashes := make([]entities.VbkID, len(encodedHashes))
	for i, encodedHash := range encodedHashes {
		copy(hashes[i][:], encodedHash.([]byte))
	}
	return hashes, nil
}

// SubmitAtv - Returns 0 if payload is valid, 1 if statefully invalid, 2 if statelessly invalid
func (v *PopContext) SubmitAtv(atv *entities.Atv) int {
	stream := new(bytes.Buffer)
	err := atv.ToVbkEncoding(stream)
	if err != nil {
		return 2
	}
	defer v.lock()()
	res := v.popContext.MemPoolSubmitAtv(stream.Bytes())
	return res
}

// SubmitVtb - Returns 0 if payload is valid, 1 if statefully invalid, 2 if statelessly invalid
func (v *PopContext) SubmitVtb(block *entities.Vtb) int {
	stream := new(bytes.Buffer)
	err := block.ToVbkEncoding(stream)
	if err != nil {
		return 2
	}
	defer v.lock()()
	res := v.popContext.MemPoolSubmitVtb(stream.Bytes())
	return res
}

// SubmitVbk - Returns 0 if payload is valid, 1 if statefully invalid, 2 if statelessly invalid
func (v *PopContext) SubmitVbk(block *entities.VbkBlock) int {
	stream := new(bytes.Buffer)
	err := block.ToVbkEncoding(stream)
	if err != nil {
		return 2
	}
	defer v.lock()()
	res := v.popContext.MemPoolSubmitVbk(stream.Bytes())
	return res
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
func (v *PopContext) RemoveAll(payloads *entities.PopData) error {
	stream := new(bytes.Buffer)
	err := payloads.ToVbkEncoding(stream)
	if err != nil {
		return err
	}
	defer v.lock()()
	v.popContext.MemPoolRemoveAll(stream.Bytes())
	return nil
}

// GetAtv ...
func (v *PopContext) GetAtv(id entities.AtvID) (*entities.Atv, error) {
	defer v.lock()()
	stream := v.popContext.MemPoolGetAtv(id)
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	var atv entities.Atv
	err := atv.FromVbkEncoding(stream)
	if err != nil {
		return nil, err
	}
	return &atv, nil
}

// GetVtb ...
func (v *PopContext) GetVtb(id entities.VtbID) (*entities.Vtb, error) {
	defer v.lock()()
	stream := v.popContext.MemPoolGetVtb(id)
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	var vtb entities.Vtb
	err := vtb.FromVbkEncoding(stream)
	if err != nil {
		return nil, err
	}
	return &vtb, nil
}

// GetVbkBlock ...
func (v *PopContext) GetVbkBlock(id entities.VbkID) (*entities.VbkBlock, error) {
	defer v.lock()()
	stream := v.popContext.MemPoolGetVbkBlock(id)
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	var vbkblock entities.VbkBlock
	err := vbkblock.FromVbkEncoding(stream)
	if err != nil {
		return nil, err
	}
	return &vbkblock, nil
}

// GetAtvs ...
func (v *PopContext) GetAtvs() ([]entities.AtvID, error) {
	defer v.lock()()
	stream := v.popContext.MemPoolGetAtvs()
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	atvIDs, err := veriblock.ReadArrayOf(stream, 0, math.MaxInt64, func(r io.Reader) (interface{}, error) {
		return veriblock.ReadSingleByteLenValueDefault(r)
	})
	if err != nil {
		return make([]entities.AtvID, 0), err
	}
	ids := make([]entities.AtvID, len(atvIDs))
	for i, atvID := range atvIDs {
		copy(ids[i][:], atvID.([]byte))
	}
	return ids, nil
}

// GetVtbs ...
func (v *PopContext) GetVtbs() ([]entities.VtbID, error) {
	defer v.lock()()
	stream := v.popContext.MemPoolGetVtbs()
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	vtbIDs, err := veriblock.ReadArrayOf(stream, 0, math.MaxInt64, func(r io.Reader) (interface{}, error) {
		return veriblock.ReadSingleByteLenValueDefault(r)
	})
	if err != nil {
		return make([]entities.VtbID, 0), err
	}
	ids := make([]entities.VtbID, len(vtbIDs))
	for i, vtbID := range vtbIDs {
		copy(ids[i][:], vtbID.([]byte))
	}
	return ids, nil
}

// GetVbkBlocks ...
func (v *PopContext) GetVbkBlocks() ([]entities.VbkID, error) {
	defer v.lock()()
	stream := v.popContext.MemPoolGetVbkBlocks()
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	vbkblockIDs, err := veriblock.ReadArrayOf(stream, 0, math.MaxInt64, func(r io.Reader) (interface{}, error) {
		return veriblock.ReadSingleByteLenValueDefault(r)
	})
	if err != nil {
		return make([]entities.VbkID, 0), err
	}
	ids := make([]entities.VbkID, len(vbkblockIDs))
	for i, vbkblockID := range vbkblockIDs {
		copy(ids[i][:], vbkblockID.([]byte))
	}
	return ids, nil
}

// GetAtvsInFlight ...
func (v *PopContext) GetAtvsInFlight() ([]entities.AtvID, error) {
	defer v.lock()()
	stream := v.popContext.MemPoolGetAtvsInFlight()
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	atvIDs, err := veriblock.ReadArrayOf(stream, 0, math.MaxInt64, func(r io.Reader) (interface{}, error) {
		return veriblock.ReadSingleByteLenValueDefault(r)
	})
	if err != nil {
		return make([]entities.AtvID, 0), err
	}
	ids := make([]entities.AtvID, len(atvIDs))
	for i, atvID := range atvIDs {
		copy(ids[i][:], atvID.([]byte))
	}
	return ids, nil
}

// GetVtbsInFlight ...
func (v *PopContext) GetVtbsInFlight() ([]entities.VtbID, error) {
	defer v.lock()()
	stream := v.popContext.MemPoolGetVtbsInFlight()
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	vtbIDs, err := veriblock.ReadArrayOf(stream, 0, math.MaxInt64, func(r io.Reader) (interface{}, error) {
		return veriblock.ReadSingleByteLenValueDefault(r)
	})
	if err != nil {
		return make([]entities.VtbID, 0), err
	}
	ids := make([]entities.VtbID, len(vtbIDs))
	for i, vtbID := range vtbIDs {
		copy(ids[i][:], vtbID.([]byte))
	}
	return ids, nil
}

// GetVbkBlocksInFlight ...
func (v *PopContext) GetVbkBlocksInFlight() ([]entities.VbkID, error) {
	defer v.lock()()
	stream := v.popContext.MemPoolGetVbkBlocksInFlight()
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	vbkblockIDs, err := veriblock.ReadArrayOf(stream, 0, math.MaxInt64, func(r io.Reader) (interface{}, error) {
		return veriblock.ReadSingleByteLenValueDefault(r)
	})
	if err != nil {
		return make([]entities.VbkID, 0), err
	}
	ids := make([]entities.VbkID, len(vbkblockIDs))
	for i, vbkblockID := range vbkblockIDs {
		copy(ids[i][:], vbkblockID.([]byte))
	}
	return ids, nil
}

func (v *PopContext) lock() (unlock func()) {
	v.mutex.Lock()
	return func() {
		v.mutex.Unlock()
	}
}
