// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package api

import (
	"bytes"
	"io"
	"math"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
	entities "github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
	"github.com/VeriBlock/alt-integration-cpp/bindings/go/ffi"
)

// AltBlockTree ...
type AltBlockTree interface {
	AcceptBlockHeader(block *entities.AltBlock) error
	AcceptBlock(hash entities.AltHash, payloads *entities.PopData) error
	ComparePopScore(hashA entities.AltHash, hashB entities.AltHash) int
	RemoveSubtree(hash entities.AltHash)
	SetState(hash entities.AltHash) error
	GetPopPayout(tipHash entities.AltHash) (*entities.PopPayouts, error)
	BtcGetBlockIndex(hash entities.BtcHash) (*entities.BlockIndex, error)
	VbkGetBlockIndex(hash entities.VbkHash) (*entities.BlockIndex, error)
	AltGetBlockIndex(hash entities.AltHash) (*entities.BlockIndex, error)
	AltBestBlock() (*entities.BlockIndex, error)
	VbkBestBlock() (*entities.BlockIndex, error)
	BtcBestBlock() (*entities.BlockIndex, error)
	VbkBootstrapBlock() (*entities.BlockIndex, error)
	BtcBootstrapBlock() (*entities.BlockIndex, error)
	AltBlockAtActiveChainByHeight(height int) (*entities.BlockIndex, error)
	VbkBlockAtActiveChainByHeight(height int) (*entities.BlockIndex, error)
	BtcBlockAtActiveChainByHeight(height int) (*entities.BlockIndex, error)
	AltGetAtvContainingBlock(atvID entities.AtvID) ([]entities.AltHash, error)
	AltGetVtbContainingBlock(vtbID entities.VtbID) ([]entities.AltHash, error)
	AltGetVbkBlockContainingBlock(vbkID entities.VbkID) ([]entities.AltHash, error)
	VbkGetVtbContainingBlock(vtbID entities.VtbID) ([]entities.VbkID, error)
	GeneratePublicationData(endorsedBlockHeader []byte, txRootHash [veriblock.Sha256HashSize]byte, popData *entities.PopData, payoutInfo []byte) (*entities.PublicationData, error)
	CalculateTopLevelMerkleRoot(txRootHash [veriblock.Sha256HashSize]byte, prevAltBlockHash entities.AltHash, popData *entities.PopData) (*entities.ContextInfoContainerHash, error)
	CheckATV(atv *entities.Atv) error
	CheckVTB(vtb *entities.Vtb) error
	CheckVbkBlock(blk *entities.VbkBlock) error
	CheckPopData(popData *entities.PopData) error
	AltBlockGetEndorsedBy(altblockHash []byte) ([]entities.AltEndorsement, error)
}

// Code for interface test
var _ AltBlockTree = &PopContext{}

// MemPool ...
type MemPool interface {
	SubmitAtv(block *entities.Atv) (int, error)
	SubmitAtvBytes(data []byte) (int, error)
	SubmitVtb(block *entities.Vtb) (int, error)
	SubmitVtbBytes(data []byte) (int, error)
	SubmitVbk(block *entities.VbkBlock) (int, error)
	SubmitVbkBytes(data []byte) (int, error)
	GetPopData() (*entities.PopData, error)
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
	popContext *ffi.PopContext
	mutex      *SafeMutex
}

// NewPopContext ...
func NewPopContext(config *Config, storage *Storage, log_lvl string) *PopContext {
	if config == nil {
		panic("Config not provided")
	}

	return &PopContext{
		popContext: ffi.NewPopContext(config.Config, storage.Storage, log_lvl),
		mutex:      NewSafeMutex(),
	}
}

// Free
func (v *PopContext) Free() {
	v.popContext.Free()
}

// AcceptBlockHeader - Returns nil if block is valid, and added
func (v *PopContext) AcceptBlockHeader(block *entities.AltBlock) error {
	v.mutex.AssertMutexLocked("pop context is not locked")

	stream := new(bytes.Buffer)
	err := block.ToVbkEncoding(stream)
	if err != nil {
		return err
	}
	state := ffi.NewValidationState()
	defer state.Free()
	ok := v.popContext.AltBlockTreeAcceptBlockHeader(stream.Bytes(), state)
	if !ok {
		return state.Error()
	}
	return nil
}

// AcceptBlock - POP payloads stored in this block
func (v *PopContext) AcceptBlock(hash entities.AltHash, payloads *entities.PopData) error {
	v.mutex.AssertMutexLocked("pop context is not locked")

	if payloads == nil {
		payloads = entities.GetEmptyPopData()
	}

	stream := new(bytes.Buffer)
	err := payloads.ToVbkEncoding(stream)
	if err != nil {
		return err
	}
	state := ffi.NewValidationState()
	defer state.Free()
	v.popContext.AltBlockTreeAcceptBlock(hash, stream.Bytes(), state)
	return state.Error()
}

// ComparePopScore ...
func (v *PopContext) ComparePopScore(hashA entities.AltHash, hashB entities.AltHash) int {
	v.mutex.AssertMutexLocked("pop context is not locked")

	return v.popContext.AltBlockTreeComparePopScore(hashA, hashB)
}

// RemoveSubtree ...
func (v *PopContext) RemoveSubtree(hash entities.AltHash) {
	v.mutex.AssertMutexLocked("pop context is not locked")

	v.popContext.AltBlockTreeRemoveSubtree(hash)
}

// SetState - Return `false` if intermediate or target block is invalid. In this
// case tree will rollback into original state. `true` if state change is successful.
func (v *PopContext) SetState(hash entities.AltHash) error {
	v.mutex.AssertMutexLocked("pop context is not locked")

	state := ffi.NewValidationState()
	defer state.Free()
	ok := v.popContext.AltBlockTreeSetState(hash, state)
	if !ok {
		return state.Error()
	}
	return nil
}

func (v *PopContext) GetPopPayout(tipHash entities.AltHash) (*entities.PopPayouts, error) {
	v.mutex.AssertMutexLocked("pop context is not locked")

	stream := v.popContext.AltBlockTreeGetPopPayout(tipHash)
	defer stream.Free()
	popPayouts := &entities.PopPayouts{}
	if err := popPayouts.FromVbkEncoding(stream); err != nil {
		return nil, err
	}
	return popPayouts, nil
}

// BtcGetBlockIndex ...
func (v *PopContext) BtcGetBlockIndex(hash entities.BtcHash) (*entities.BlockIndex, error) {
	v.mutex.AssertMutexLocked("pop context is not locked")

	stream := v.popContext.BtcGetBlockIndex([veriblock.Sha256HashSize]byte(hash))
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	blockIndex := entities.NewBtcBlockIndex()
	if err := blockIndex.FromVbkEncoding(stream); err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// VbkGetBlockIndex ...
func (v *PopContext) VbkGetBlockIndex(hash entities.VbkHash) (*entities.BlockIndex, error) {
	v.mutex.AssertMutexLocked("pop context is not locked")

	stream := v.popContext.VbkGetBlockIndex([veriblock.VblakeBlockHashSize]byte(hash))
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	blockIndex := entities.NewVbkBlockIndex()
	if err := blockIndex.FromVbkEncoding(stream); err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// AltGetBlockIndex ...
func (v *PopContext) AltGetBlockIndex(hash entities.AltHash) (*entities.BlockIndex, error) {
	v.mutex.AssertMutexLocked("pop context is not locked")

	stream := v.popContext.AltGetBlockIndex([]byte(hash))
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	blockIndex := entities.NewAltBlockIndex()
	if err := blockIndex.FromVbkEncoding(stream); err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// AltBestBlock ...
func (v *PopContext) AltBestBlock() (*entities.BlockIndex, error) {
	v.mutex.AssertMutexLocked("pop context is not locked")

	stream := v.popContext.AltBestBlock()
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	blockIndex := entities.NewAltBlockIndex()
	if err := blockIndex.FromVbkEncoding(stream); err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// VbkBestBlock ...
func (v *PopContext) VbkBestBlock() (*entities.BlockIndex, error) {
	v.mutex.AssertMutexLocked("pop context is not locked")

	stream := v.popContext.VbkBestBlock()
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	blockIndex := entities.NewVbkBlockIndex()
	if err := blockIndex.FromVbkEncoding(stream); err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// BtcBestBlock ...
func (v *PopContext) BtcBestBlock() (*entities.BlockIndex, error) {
	v.mutex.AssertMutexLocked("pop context is not locked")

	stream := v.popContext.BtcBestBlock()
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	blockIndex := entities.NewBtcBlockIndex()
	if err := blockIndex.FromVbkEncoding(stream); err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// VbkBootstrapBlock ...
func (v *PopContext) VbkBootstrapBlock() (*entities.BlockIndex, error) {
	v.mutex.AssertMutexLocked("pop context is not locked")

	stream := v.popContext.VbkBootstrapBlock()
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	blockIndex := entities.NewVbkBlockIndex()
	if err := blockIndex.FromVbkEncoding(stream); err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// BtcBootstrapBlock ...
func (v *PopContext) BtcBootstrapBlock() (*entities.BlockIndex, error) {
	v.mutex.AssertMutexLocked("pop context is not locked")

	stream := v.popContext.BtcBootstrapBlock()
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	blockIndex := entities.NewBtcBlockIndex()
	if err := blockIndex.FromVbkEncoding(stream); err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// AltBlockAtActiveChainByHeight ...
func (v *PopContext) AltBlockAtActiveChainByHeight(height int) (*entities.BlockIndex, error) {
	v.mutex.AssertMutexLocked("pop context is not locked")

	stream := v.popContext.AltBlockAtActiveChainByHeight(height)
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	blockIndex := entities.NewAltBlockIndex()
	if err := blockIndex.FromVbkEncoding(stream); err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// VbkBlockAtActiveChainByHeight ...
func (v *PopContext) VbkBlockAtActiveChainByHeight(height int) (*entities.BlockIndex, error) {
	v.mutex.AssertMutexLocked("pop context is not locked")

	stream := v.popContext.VbkBlockAtActiveChainByHeight(height)
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	blockIndex := entities.NewVbkBlockIndex()
	if err := blockIndex.FromVbkEncoding(stream); err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// BtcBlockAtActiveChainByHeight ...
func (v *PopContext) BtcBlockAtActiveChainByHeight(height int) (*entities.BlockIndex, error) {
	v.mutex.AssertMutexLocked("pop context is not locked")

	stream := v.popContext.BtcBlockAtActiveChainByHeight(height)
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	blockIndex := entities.NewBtcBlockIndex()
	if err := blockIndex.FromVbkEncoding(stream); err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// AltGetAtvContainingBlock ...
func (v *PopContext) AltGetAtvContainingBlock(atvID entities.AtvID) ([]entities.AltHash, error) {
	v.mutex.AssertMutexLocked("pop context is not locked")

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
		hashes[i] = make(entities.AltHash, len(encodedHash.([]byte)))
		copy(hashes[i][:], encodedHash.([]byte))
	}
	return hashes, nil
}

// AltGetVtbContainingBlock ...
func (v *PopContext) AltGetVtbContainingBlock(vtbID entities.VtbID) ([]entities.AltHash, error) {
	v.mutex.AssertMutexLocked("pop context is not locked")

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
	v.mutex.AssertMutexLocked("pop context is not locked")

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
	v.mutex.AssertMutexLocked("pop context is not locked")

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
func (v *PopContext) SubmitAtv(atv *entities.Atv) (int, error) {
	v.mutex.AssertMutexLocked("pop context is not locked")

	stream := new(bytes.Buffer)
	err := atv.ToVbkEncoding(stream)
	if err != nil {
		return 2, err
	}
	state := ffi.NewValidationState()
	defer state.Free()
	res := v.popContext.MemPoolSubmitAtv(stream.Bytes(), state)
	return res, state.Error()
}

// SubmitAtvBytes - Returns 0 if payload is valid, 1 if statefully invalid, 2 if statelessly invalid
func (v *PopContext) SubmitAtvBytes(data []byte) (int, error) {
	v.mutex.AssertMutexLocked("pop context is not locked")

	state := ffi.NewValidationState()
	defer state.Free()
	res := v.popContext.MemPoolSubmitAtv(data, state)
	return res, state.Error()
}

// SubmitVtb - Returns 0 if payload is valid, 1 if statefully invalid, 2 if statelessly invalid
func (v *PopContext) SubmitVtb(block *entities.Vtb) (int, error) {
	v.mutex.AssertMutexLocked("pop context is not locked")

	stream := new(bytes.Buffer)
	err := block.ToVbkEncoding(stream)
	if err != nil {
		return 2, err
	}
	state := ffi.NewValidationState()
	defer state.Free()
	res := v.popContext.MemPoolSubmitVtb(stream.Bytes(), state)
	return res, state.Error()
}

// SubmitVtbBytes - Returns 0 if payload is valid, 1 if statefully invalid, 2 if statelessly invalid
func (v *PopContext) SubmitVtbBytes(data []byte) (int, error) {
	v.mutex.AssertMutexLocked("pop context is not locked")

	state := ffi.NewValidationState()
	defer state.Free()
	res := v.popContext.MemPoolSubmitVtb(data, state)
	return res, state.Error()
}

// SubmitVbk - Returns 0 if payload is valid, 1 if statefully invalid, 2 if statelessly invalid
func (v *PopContext) SubmitVbk(block *entities.VbkBlock) (int, error) {
	v.mutex.AssertMutexLocked("pop context is not locked")

	stream := new(bytes.Buffer)
	err := block.ToVbkEncoding(stream)
	if err != nil {
		return 2, err
	}
	state := ffi.NewValidationState()
	defer state.Free()
	res := v.popContext.MemPoolSubmitVbk(stream.Bytes(), state)
	return res, state.Error()
}

// SubmitVbkBytes - Returns 0 if payload is valid, 1 if statefully invalid, 2 if statelessly invalid
func (v *PopContext) SubmitVbkBytes(data []byte) (int, error) {
	v.mutex.AssertMutexLocked("pop context is not locked")

	state := ffi.NewValidationState()
	defer state.Free()
	res := v.popContext.MemPoolSubmitVbk(data, state)
	return res, state.Error()
}

// GetPop ...
func (v *PopContext) GetPopData() (*entities.PopData, error) {
	v.mutex.AssertMutexLocked("pop context is not locked")

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
	v.mutex.AssertMutexLocked("pop context is not locked")

	if payloads == nil {
		return nil
	}

	stream := new(bytes.Buffer)
	err := payloads.ToVbkEncoding(stream)
	if err != nil {
		return err
	}
	state := ffi.NewValidationState()
	defer state.Free()
	v.popContext.MemPoolRemoveAll(stream.Bytes(), state)
	return state.Error()
}

// GetAtv ...
func (v *PopContext) GetAtv(id entities.AtvID) (*entities.Atv, error) {
	v.mutex.AssertMutexLocked("pop context is not locked")

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
	v.mutex.AssertMutexLocked("pop context is not locked")

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
	v.mutex.AssertMutexLocked("pop context is not locked")

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
	v.mutex.AssertMutexLocked("pop context is not locked")

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
	v.mutex.AssertMutexLocked("pop context is not locked")

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
	v.mutex.AssertMutexLocked("pop context is not locked")

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
	v.mutex.AssertMutexLocked("pop context is not locked")

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
	v.mutex.AssertMutexLocked("pop context is not locked")

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
	v.mutex.AssertMutexLocked("pop context is not locked")

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

func (v *PopContext) Lock() (unlock func()) {
	v.mutex.Lock()
	return func() {
		v.mutex.Unlock()
	}
}
