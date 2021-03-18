package api

import (
	"bytes"

	entities "github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
	"github.com/VeriBlock/alt-integration-cpp/bindings/go/ffi"
)

// MockMiner ...
type MockMiner struct {
	miner *ffi.MockMiner
	mutex *SafeMutex
}

// NewMockMiner ...
func NewMockMiner() *MockMiner {
	return &MockMiner{
		miner: ffi.NewMockMiner(),
		mutex: NewSafeMutex(),
	}
}

// MineBtcBlockTip - Mine new altintegration::BtcBlock on the top of the current btctree.
func (v *MockMiner) MineBtcBlockTip() (*entities.BlockIndex, error) {
	v.mutex.AssertMutexLocked("mock miner is not locked")

	stream := v.miner.MineBtcBlockTip()
	if stream == nil {
		panic("MineBtcBlockTip returned nullptr")
	}
	defer stream.Free()
	blockIndex := entities.NewBtcBlockIndex()
	err := blockIndex.FromVbkEncoding(stream)
	if err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// MineBtcBlock - Mine new altintegration::BtcBlock on the top of the provided block.
func (v *MockMiner) MineBtcBlock(blockHash []byte) (*entities.BlockIndex, error) {
	v.mutex.AssertMutexLocked("mock miner is not locked")

	stream := v.miner.MineBtcBlock(blockHash)
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	blockIndex := entities.NewBtcBlockIndex()
	err := blockIndex.FromVbkEncoding(stream)
	if err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// MineVbkBlockTip - Mine new altintegration::VbkBlock on the top of the current vbktree.
func (v *MockMiner) MineVbkBlockTip() (*entities.BlockIndex, error) {
	v.mutex.AssertMutexLocked("mock miner is not locked")

	stream := v.miner.MineVbkBlockTip()
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	blockIndex := entities.NewVbkBlockIndex()
	err := blockIndex.FromVbkEncoding(stream)
	if err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// MineVbkBlock - Mine new altintegration::VbkBlock on the top of the provided block.
func (v *MockMiner) MineVbkBlock(blockHash []byte) (*entities.BlockIndex, error) {
	v.mutex.AssertMutexLocked("mock miner is not locked")

	stream := v.miner.MineVbkBlock(blockHash)
	defer stream.Free()
	if stream == nil {
		return nil, nil
	}
	blockIndex := entities.NewVbkBlockIndex()
	err := blockIndex.FromVbkEncoding(stream)
	if err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// MineAtv ...
func (v *MockMiner) MineAtv(publicationData *entities.PublicationData) (*entities.Atv, error) {
	v.mutex.AssertMutexLocked("mock miner is not locked")

	var buffer bytes.Buffer
	err := publicationData.ToVbkEncoding(&buffer)
	if err != nil {
		return nil, err
	}
	state := ffi.NewValidationState()
	defer state.Free()
	stream := v.miner.MineAtv(buffer.Bytes(), state)
	if stream == nil {
		return nil, state.Error()
	}
	defer stream.Free()
	var atv entities.Atv
	err = atv.FromVbkEncoding(stream)
	if err != nil {
		return nil, err
	}
	return &atv, nil
}

// MineVtb ...
func (v *MockMiner) MineVtb(endorsedBlock *entities.VbkBlock, hash []byte) (*entities.Vtb, error) {
	v.mutex.AssertMutexLocked("mock miner is not locked")

	var buffer bytes.Buffer
	err := endorsedBlock.ToVbkEncoding(&buffer)
	if err != nil {
		return nil, err
	}
	state := ffi.NewValidationState()
	defer state.Free()
	stream := v.miner.MineVtb(buffer.Bytes(), hash, state)
	if stream == nil {
		return nil, state.Error()
	}
	defer stream.Free()
	var vtb entities.Vtb
	err = vtb.FromVbkEncoding(stream)
	if err != nil {
		return nil, err
	}
	return &vtb, nil
}

func (v *MockMiner) Lock() (unlock func()) {
	v.mutex.Lock()
	return func() {
		v.mutex.Unlock()
	}
}
