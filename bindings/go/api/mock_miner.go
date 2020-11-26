package api

import (
	"bytes"
	"sync"

	entities "github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
	"github.com/VeriBlock/alt-integration-cpp/bindings/go/ffi"
)

// MockMiner ...
type MockMiner struct {
	miner *ffi.MockMiner
	mutex *sync.Mutex
}

// NewMockMiner ...
func NewMockMiner() *MockMiner {
	return &MockMiner{
		miner: ffi.NewMockMiner(),
		mutex: new(sync.Mutex),
	}
}

// Free ...
func (v *MockMiner) Free() { v.miner.Free() }

// MineBtcBlockTip - Mine new altintegration::BtcBlock on the top of the current btctree.
func (v *MockMiner) MineBtcBlockTip() (*entities.BlockIndex, error) {
	defer v.lock()()
	stream := v.miner.MineBtcBlockTip()
	if stream == nil {
		panic("MineBtcBlockTip returned nullptr")
	}
	defer stream.Free()
	blockIndex := entities.NewBtcBlockIndex()
	err := blockIndex.FromRaw(stream)
	if err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// MineBtcBlock - Mine new altintegration::BtcBlock on the top of the provided block.
func (v *MockMiner) MineBtcBlock(blockHash []byte) (*entities.BlockIndex, error) {
	defer v.lock()()
	stream := v.miner.MineBtcBlock(blockHash)
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	blockIndex := entities.NewBtcBlockIndex()
	err := blockIndex.FromRaw(stream)
	if err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// MineVbkBlockTip - Mine new altintegration::VbkBlock on the top of the current vbktree.
func (v *MockMiner) MineVbkBlockTip() (*entities.BlockIndex, error) {
	defer v.lock()()
	stream := v.miner.MineVbkBlockTip()
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	blockIndex := entities.NewVbkBlockIndex()
	err := blockIndex.FromRaw(stream)
	if err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// MineVbkBlock - Mine new altintegration::VbkBlock on the top of the provided block.
func (v *MockMiner) MineVbkBlock(blockHash []byte) (*entities.BlockIndex, error) {
	defer v.lock()()
	stream := v.miner.MineVbkBlock(blockHash)
	defer stream.Free()
	if stream == nil {
		return nil, nil
	}
	blockIndex := entities.NewVbkBlockIndex()
	err := blockIndex.FromRaw(stream)
	if err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// MineAtv ...
func (v *MockMiner) MineAtv(publicationData *entities.PublicationData) (*entities.Atv, error) {
	defer v.lock()()
	var buffer bytes.Buffer
	err := publicationData.ToRaw(&buffer)
	if err != nil {
		return nil, err
	}
	stream := v.miner.MineAtv(buffer.Bytes())
	if stream == nil {
		return nil, nil
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
	defer v.lock()()
	var buffer bytes.Buffer
	err := endorsedBlock.ToVbkEncoding(&buffer)
	if err != nil {
		return nil, err
	}
	stream := v.miner.MineVtb(buffer.Bytes(), hash)
	if stream == nil {
		return nil, nil
	}
	defer stream.Free()
	var vtb entities.Vtb
	err = vtb.FromVbkEncoding(stream)
	if err != nil {
		return nil, err
	}
	return &vtb, nil
}

func (v *MockMiner) lock() (unlock func()) {
	v.mutex.Lock()
	return func() {
		v.mutex.Unlock()
	}
}
