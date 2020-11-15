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
	GetAtv(id []byte) (*entities.Atv, error)
	GetVtb(id []byte) (*entities.Vtb, error)
	GetVbkBlock(id []byte) (*entities.VbkBlock, error)
	GetAtvs() ([][]byte, error)
	GetVtbs() ([][]byte, error)
	GetVbkBlocks() ([][]byte, error)
	GetAtvsInFlight() ([][]byte, error)
	GetVtbsInFlight() ([][]byte, error)
	GetVbkBlocksInFlight() ([][]byte, error)
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

// SubmitAtv - Returns 0 if payload is valid, 1 if statefully invalid, 2 if statelessly invalid
func (v *PopContext) SubmitAtv(block *entities.Atv) int {
	stream := new(bytes.Buffer)
	err := block.ToVbkEncoding(stream)
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

// GetAtv ...
func (v *PopContext) GetAtv(id []byte) (*entities.Atv, error) {
	defer v.lock()()
	stream := v.popContext.MemPoolGetAtv(id)
	defer stream.Free()
	atv := &entities.Atv{}
	err := atv.FromVbkEncoding(&stream)
	if err != nil {
		return nil, err
	}
	return atv, nil
}

// GetVtb ...
func (v *PopContext) GetVtb(id []byte) (*entities.Vtb, error) {
	defer v.lock()()
	stream := v.popContext.MemPoolGetVtb(id)
	defer stream.Free()
	vtb := &entities.Vtb{}
	err := vtb.FromVbkEncoding(&stream)
	if err != nil {
		return nil, err
	}
	return vtb, nil
}

// GetVbkBlock ...
func (v *PopContext) GetVbkBlock(id []byte) (*entities.VbkBlock, error) {
	defer v.lock()()
	stream := v.popContext.MemPoolGetVbkBlock(id)
	defer stream.Free()
	vbkblock := &entities.VbkBlock{}
	err := vbkblock.FromVbkEncoding(&stream)
	if err != nil {
		return nil, err
	}
	return vbkblock, nil
}

// GetAtvs ...
func (v *PopContext) GetAtvs() ([]entities.Atv, error) {
	defer v.lock()()
	stream := v.popContext.MemPoolGetAtvs()
	defer stream.Free()
	atvsRaw, err := veriblock.ReadArrayOf(&stream, 0, math.MaxInt32, func(r io.Reader) (interface{}, error) {
		block := &entities.Atv{}
		if err := block.FromVbkEncoding(&stream); err != nil {
			return nil, err
		}
		return block, nil
	})
	if err != nil {
		return nil, err
	}
	atvs := make([]entities.Atv, len(atvsRaw))
	for i, atv := range atvsRaw {
		atvs[i] = *atv.(*entities.Atv)
	}
	return atvs, nil
}

// GetVtbs ...
func (v *PopContext) GetVtbs() ([]entities.Vtb, error) {
	defer v.lock()()
	stream := v.popContext.MemPoolGetVtbs()
	defer stream.Free()
	vtbsRaw, err := veriblock.ReadArrayOf(&stream, 0, math.MaxInt32, func(r io.Reader) (interface{}, error) {
		block := &entities.Vtb{}
		if err := block.FromVbkEncoding(&stream); err != nil {
			return nil, err
		}
		return block, nil
	})
	if err != nil {
		return nil, err
	}
	vtbs := make([]entities.Vtb, len(vtbsRaw))
	for i, vtb := range vtbsRaw {
		vtbs[i] = *vtb.(*entities.Vtb)
	}
	return vtbs, nil
}

// GetVbkBlocks ...
func (v *PopContext) GetVbkBlocks() ([]entities.VbkBlock, error) {
	defer v.lock()()
	stream := v.popContext.MemPoolGetVbkBlocks()
	defer stream.Free()
	vbkBlocksRaw, err := veriblock.ReadArrayOf(&stream, 0, math.MaxInt32, func(r io.Reader) (interface{}, error) {
		block := &entities.VbkBlock{}
		if err := block.FromVbkEncoding(&stream); err != nil {
			return nil, err
		}
		return block, nil
	})
	if err != nil {
		return nil, err
	}
	vbkBlocks := make([]entities.VbkBlock, len(vbkBlocksRaw))
	for i, vbkBlock := range vbkBlocksRaw {
		vbkBlocks[i] = *vbkBlock.(*entities.VbkBlock)
	}
	return vbkBlocks, nil
}

// GetAtvsInFlight ...
func (v *PopContext) GetAtvsInFlight() ([][]byte, error) {
	stream := v.popContext.MemPoolGetAtvsInFlight()
	defer stream.Free()
	var buffer bytes.Buffer
	stream.ReadAll(&buffer)
	atvIDs, err := veriblock.ReadArrayOf(&buffer, 0, math.MaxInt64, func(r io.Reader) (interface{}, error) {
		return veriblock.ReadSingleByteLenValueDefault(r)
	})
	if err != nil {
		return make([][]byte, 0), err
	}
	ids := make([][]byte, len(atvIDs))
	for i, atvID := range atvIDs {
		copy(ids[i][:], atvID.([]byte))
	}
	return ids, nil
}

// GetVtbsInFlight ...
func (v *PopContext) GetVtbsInFlight() ([][]byte, error) {
	stream := v.popContext.MemPoolGetVtbsInFlight()
	defer stream.Free()
	var buffer bytes.Buffer
	stream.ReadAll(&buffer)
	vtbIDs, err := veriblock.ReadArrayOf(&buffer, 0, math.MaxInt64, func(r io.Reader) (interface{}, error) {
		return veriblock.ReadSingleByteLenValueDefault(r)
	})
	if err != nil {
		return make([][]byte, 0), err
	}
	ids := make([][]byte, len(vtbIDs))
	for i, vtbID := range vtbIDs {
		copy(ids[i][:], vtbID.([]byte))
	}
	return ids, nil
}

// GetVbkBlocksInFlight ...
func (v *PopContext) GetVbkBlocksInFlight() ([][]byte, error) {
	stream := v.popContext.MemPoolGetVbkBlocksInFlight()
	defer stream.Free()
	var buffer bytes.Buffer
	stream.ReadAll(&buffer)
	vbkblockIDs, err := veriblock.ReadArrayOf(&buffer, 0, math.MaxInt64, func(r io.Reader) (interface{}, error) {
		return veriblock.ReadSingleByteLenValueDefault(r)
	})
	if err != nil {
		return make([][]byte, 0), err
	}
	ids := make([][]byte, len(vbkblockIDs))
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
