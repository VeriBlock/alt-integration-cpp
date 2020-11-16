package api

import (
	"bytes"
	"sync"

	entities "github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
	"github.com/VeriBlock/alt-integration-cpp/bindings/go/ffi"
)

// MockMiner ...
type MockMiner struct {
	miner ffi.MockMiner
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
	defer stream.Free()
	blockIndex := entities.BlockIndex{}
	blockIndex.Header = &entities.BtcBlock{}
	blockIndex.Addon = &entities.BtcBlockAddon{}
	err := blockIndex.FromRaw(&stream)
	if err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// MineBtcBlock - Mine new altintegration::BtcBlock on the top of the provided block.
func (v *MockMiner) MineBtcBlock(tipBlockIndex *entities.BlockIndex) (*entities.BlockIndex, error) {
	byteStream := new(bytes.Buffer)
	if err := tipBlockIndex.ToRaw(byteStream); err != nil {
		return nil, err
	}
	defer v.lock()()
	stream := v.miner.MineBtcBlock(byteStream.Bytes())
	defer stream.Free()
	blockIndex := entities.BlockIndex{}
	blockIndex.Header = &entities.BtcBlock{}
	blockIndex.Addon = &entities.BtcBlockAddon{}
	err := blockIndex.FromRaw(&stream)
	if err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// MineVbkBlockTip - Mine new altintegration::VbkBlock on the top of the current vbktree.
func (v *MockMiner) MineVbkBlockTip() (*entities.BlockIndex, error) {
	defer v.lock()()
	stream := v.miner.MineVbkBlockTip()
	defer stream.Free()
	blockIndex := entities.BlockIndex{}
	blockIndex.Header = &entities.VbkBlock{}
	blockIndex.Addon = &entities.VbkBlockAddon{}
	err := blockIndex.FromRaw(&stream)
	if err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

// MineVbkBlock - Mine new altintegration::VbkBlock on the top of the provided block.
func (v *MockMiner) MineVbkBlock(tipBlockIndex *entities.BlockIndex) (*entities.BlockIndex, error) {
	byteStream := new(bytes.Buffer)
	if err := tipBlockIndex.ToRaw(byteStream); err != nil {
		return nil, err
	}
	defer v.lock()()
	stream := v.miner.MineVbkBlock(byteStream.Bytes())
	defer stream.Free()
	blockIndex := entities.BlockIndex{}
	blockIndex.Header = &entities.VbkBlock{}
	blockIndex.Addon = &entities.VbkBlockAddon{}
	err := blockIndex.FromRaw(&stream)
	if err != nil {
		return nil, err
	}
	return &blockIndex, nil
}

func (v *MockMiner) lock() (unlock func()) {
	v.mutex.Lock()
	return func() {
		v.mutex.Unlock()
	}
}
