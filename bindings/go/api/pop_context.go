package api

import (
	"bytes"
	"errors"
	"io"
	"math"
	"sync"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
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
func (v *PopContext) SubmitAtv(block entities.Atv) int {
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
func (v *PopContext) SubmitVtb(block entities.Vtb) int {
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
func (v *PopContext) SubmitVbk(block entities.VbkBlock) int {
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

func (v *PopContext) GetAtv(id []byte) (*entities.Atv, error) {
	stream := v.popContext.MemPoolGetAtv(id)
	var buffer bytes.Buffer
	stream.ReadAll(&buffer)

	var atv entities.Atv
	err := atv.FromVbkEncoding(&buffer)
	if err != nil {
		return nil, err
	}
	return &atv, nil
}

func (v *PopContext) GetVtb(id []byte) (*entities.Vtb, error) {
	stream := v.popContext.MemPoolGetVtb(id)
	var buffer bytes.Buffer
	stream.ReadAll(&buffer)

	var vtb entities.Vtb
	err := vtb.FromVbkEncoding(&buffer)
	if err != nil {
		return nil, err
	}
	return &vtb, nil
}

func (v *PopContext) GetVbkBlock(id []byte) (*entities.VbkBlock, error) {
	stream := v.popContext.MemPoolGetVbkBlock(id)
	var buffer bytes.Buffer
	stream.ReadAll(&buffer)

	var vbkblock entities.VbkBlock
	err := vbkblock.FromVbkEncoding(&buffer)
	if err != nil {
		return nil, err
	}
	return &vbkblock, nil
}

func (v *PopContext) GetAtvs() ([][]byte, error) {
	stream := v.popContext.MemPoolGetAtvs()
	var buffer bytes.Buffer
	stream.ReadAll(&buffer)

	atv_ids, err := veriblock.ReadArrayOf(&buffer, 0, math.MaxInt64, func(r io.Reader) (interface{}, error) {
		return veriblock.ReadSingleByteLenValueDefault(r)
	})

	if err != nil {
		return make([][]byte, 0), err
	}

	ids := make([][]byte, len(atv_ids))
	for i, atv_id := range atv_ids {
		copy(ids[i][:], atv_id.([]byte))
	}

	return ids, nil
}

func (v *PopContext) GetVtbs() ([][]byte, error) {
	stream := v.popContext.MemPoolGetVtbs()
	var buffer bytes.Buffer
	stream.ReadAll(&buffer)

	vtb_ids, err := veriblock.ReadArrayOf(&buffer, 0, math.MaxInt64, func(r io.Reader) (interface{}, error) {
		return veriblock.ReadSingleByteLenValueDefault(r)
	})

	if err != nil {
		return make([][]byte, 0), err
	}

	ids := make([][]byte, len(vtb_ids))
	for i, vtb_id := range vtb_ids {
		copy(ids[i][:], vtb_id.([]byte))
	}

	return ids, nil
}

func (v *PopContext) GetVbkBlocks() ([][]byte, error) {
	stream := v.popContext.MemPoolGetVbkBlocks()
	var buffer bytes.Buffer
	stream.ReadAll(&buffer)

	vbkblock_ids, err := veriblock.ReadArrayOf(&buffer, 0, math.MaxInt64, func(r io.Reader) (interface{}, error) {
		return veriblock.ReadSingleByteLenValueDefault(r)
	})

	if err != nil {
		return make([][]byte, 0), err
	}

	ids := make([][]byte, len(vbkblock_ids))
	for i, vbkblock_id := range vbkblock_ids {
		copy(ids[i][:], vbkblock_id.([]byte))
	}

	return ids, nil
}

func (v *PopContext) GetAtvsInFlight() ([][]byte, error) {
	stream := v.popContext.MemPoolGetAtvsInFlight()
	var buffer bytes.Buffer
	stream.ReadAll(&buffer)

	atv_ids, err := veriblock.ReadArrayOf(&buffer, 0, math.MaxInt64, func(r io.Reader) (interface{}, error) {
		return veriblock.ReadSingleByteLenValueDefault(r)
	})

	if err != nil {
		return make([][]byte, 0), err
	}

	ids := make([][]byte, len(atv_ids))
	for i, atv_id := range atv_ids {
		copy(ids[i][:], atv_id.([]byte))
	}

	return ids, nil
}

func (v *PopContext) GetVtbsInFlight() ([][]byte, error) {
	stream := v.popContext.MemPoolGetVtbsInFlight()
	var buffer bytes.Buffer
	stream.ReadAll(&buffer)

	vtb_ids, err := veriblock.ReadArrayOf(&buffer, 0, math.MaxInt64, func(r io.Reader) (interface{}, error) {
		return veriblock.ReadSingleByteLenValueDefault(r)
	})

	if err != nil {
		return make([][]byte, 0), err
	}

	ids := make([][]byte, len(vtb_ids))
	for i, vtb_id := range vtb_ids {
		copy(ids[i][:], vtb_id.([]byte))
	}

	return ids, nil
}

func (v *PopContext) GetVbkBlocksInFlight() ([][]byte, error) {
	stream := v.popContext.MemPoolGetVbkBlocksInFlight()
	var buffer bytes.Buffer
	stream.ReadAll(&buffer)

	vbkblock_ids, err := veriblock.ReadArrayOf(&buffer, 0, math.MaxInt64, func(r io.Reader) (interface{}, error) {
		return veriblock.ReadSingleByteLenValueDefault(r)
	})

	if err != nil {
		return make([][]byte, 0), err
	}

	ids := make([][]byte, len(vbkblock_ids))
	for i, vbkblock_id := range vbkblock_ids {
		copy(ids[i][:], vbkblock_id.([]byte))
	}

	return ids, nil
}

func (v *PopContext) lock() (unlock func()) {
	v.mutex.Lock()
	return func() {
		v.mutex.Unlock()
	}
}
