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
func (v *MockMiner) MineBtcBlock(block_hash []byte) (*entities.BlockIndex, error) {
	defer v.lock()()
	stream := v.miner.MineBtcBlock(block_hash)
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
func (v *MockMiner) MineVbkBlock(block_hash []byte) (*entities.BlockIndex, error) {
	defer v.lock()()
	stream := v.miner.MineVbkBlock(block_hash)
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

// MineAtv ...
func (v *MockMiner) MineAtv(publication_data *entities.PublicationData) (*entities.Atv, error) {
	defer v.lock()()
	var buffer bytes.Buffer
	err := publication_data.ToRaw(&buffer)
	if err != nil {
		return nil, err
	}

	stream := v.miner.MineAtv(buffer.Bytes())
	defer stream.Free()

	var atv entities.Atv
	err = atv.FromVbkEncoding(&stream)
	if err != nil {
		return nil, err
	}
	return &atv, nil
}

// MineVtb ...
func (v *MockMiner) MineVtb(endorsed_block *entities.VbkBlock) (*entities.Vtb, error) {
	defer v.lock()()
	var buffer bytes.Buffer
	err := endorsed_block.ToVbkEncoding(&buffer)
	if err != nil {
		return nil, err
	}

	stream := v.miner.MineVtb(buffer.Bytes())
	defer stream.Free()

	var vtb entities.Vtb
	err = vtb.FromVbkEncoding(&stream)
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
