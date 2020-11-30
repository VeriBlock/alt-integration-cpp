package entities

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
)

// BlockStatus ...
type BlockStatus uint32

const (
	// BlockValidUnknown - Default state for validity - validity state is unknown
	BlockValidUnknown BlockStatus = 0
	// BlockBootstrap - This is a bootstrap block
	BlockBootstrap BlockStatus = 1 << 1
	// BlockFailedBlock - Block is statelessly valid, but the altchain marked it as failed
	BlockFailedBlock BlockStatus = 1 << 2
	// BlockFailedPop - Block failed state{less,ful} validation due to its payloads
	BlockFailedPop BlockStatus = 1 << 3
	// BlockFailedChild - Block is state{lessly,fully} valid and the altchain did not report it as
	// invalid, but some of the ancestor blocks are invalid
	BlockFailedChild BlockStatus = 1 << 4
	// BlockFailedMask - All invalidity flags
	BlockFailedMask BlockStatus = BlockFailedChild | BlockFailedPop | BlockFailedBlock
	// BlockApplied - The block has been applied via PopStateMachine
	BlockApplied BlockStatus = 1 << 5

	// BlockValidTree - AcceptBlockHeader succeded. All ancestors are at least at this state.
	BlockValidTree BlockStatus = 1 << 6
	// BlockHasPayloads - AcceptBlock has been executed on this block; payloads are statelessly valid
	BlockHasPayloads BlockStatus = 2 << 6
	// BlockConnected - The block is connected via connectBlock
	BlockConnected BlockStatus = 3 << 6
	// BlockHasBeenApplied - The block has been successfully applied, likely along with another chain
	BlockHasBeenApplied BlockStatus = 4 << 6
	// BlockCanBeApplied - The chain with the block at its tip is fully valid
	BlockCanBeApplied BlockStatus = 5 << 6
	// BlockValidMask - All stateful validity levels
	// FIXME: BlockHasPayloads is not really a stateful validity level and does
	// not belong here since it does not depend on other block contents
	BlockValidMask BlockStatus = BlockValidUnknown | BlockValidTree | BlockHasPayloads |
		BlockConnected | BlockHasBeenApplied | BlockCanBeApplied
)

// GenericBlockHeader ...
type GenericBlockHeader interface {
	veriblock.SerdeRaw
	ToJSON() (map[string]interface{}, error)
	GetGenericHash() []byte
	GetBlockTime() uint32
	GetDifficulty() uint32
}

// NewBtcBlockIndex - Returns new BTC Block Index
func NewBtcBlockIndex() BlockIndex {
	blockIndex := BlockIndex{}
	blockIndex.Header = &BtcBlock{}
	blockIndex.Addon = &BtcBlockAddon{}
	return blockIndex
}

// NewVbkBlockIndex - Returns new VBK Block Index
func NewVbkBlockIndex() BlockIndex {
	blockIndex := BlockIndex{}
	blockIndex.Header = &VbkBlock{}
	blockIndex.Addon = &VbkBlockAddon{}
	return blockIndex
}

// NewAltBlockIndex - Returns new ALT Block Index
func NewAltBlockIndex() BlockIndex {
	blockIndex := BlockIndex{}
	blockIndex.Header = &AltBlock{}
	blockIndex.Addon = &AltBlockAddon{}
	return blockIndex
}

// BlockIndex ...
type BlockIndex struct {
	Addon veriblock.SerdeRaw
	// Height of the entry in the chain
	Height uint32
	// Block header
	Header GenericBlockHeader
	// Contains status flags
	Status BlockStatus
}

// GetHash ...
func (v *BlockIndex) GetHash() []byte {
	return v.Header.GetGenericHash()
}

// GetBlockTime ...
func (v *BlockIndex) GetBlockTime() uint32 {
	return v.Header.GetBlockTime()
}

// GetDifficulty ...
func (v *BlockIndex) GetDifficulty() uint32 {
	return v.Header.GetDifficulty()
}

// ToRaw ...
func (v *BlockIndex) ToRaw(stream io.Writer) error {
	if err := binary.Write(stream, binary.BigEndian, v.Height); err != nil {
		return err
	}
	if err := v.Header.ToRaw(stream); err != nil {
		return err
	}
	if err := binary.Write(stream, binary.BigEndian, uint32(v.Status)); err != nil {
		return err
	}
	return v.Addon.ToRaw(stream)
}

// ToRawBytes ...
func (v *BlockIndex) ToRawBytes() ([]byte, error) {
	var buffer bytes.Buffer
	err := v.ToRaw(&buffer)
	return buffer.Bytes(), err
}

// FromRaw ...
func (v *BlockIndex) FromRaw(stream io.Reader) error {
	if err := binary.Read(stream, binary.BigEndian, &v.Height); err != nil {
		return err
	}
	if err := v.Header.FromRaw(stream); err != nil {
		return err
	}
	var status uint32
	if err := binary.Read(stream, binary.LittleEndian, &status); err != nil {
		return err
	}
	v.Status = BlockStatus(status)
	return v.Addon.FromRaw(stream)
}

// FromRawBytes ...
func (v *BlockIndex) FromRawBytes(data []byte) error {
	buffer := bytes.NewBuffer(data)
	return v.FromRaw(buffer)
}

// ToJSON ...
func (v *BlockIndex) ToJSON() (map[string]interface{}, error) {
	var header map[string]interface{}
	var err error
	switch val := v.Header.(type) {
	case *VbkBlock:
		header, err = val.ToJSON()
	case *BtcBlock:
		header, err = val.ToJSON()
	case *AltBlock:
		header, err = val.ToJSON()
	default:
		err = fmt.Errorf("Invalid Block Header")
	}
	if err != nil {
		return nil, err
	}
	var res map[string]interface{}
	switch val := v.Addon.(type) {
	case *VbkBlockAddon:
		// TODO: Support chainWork, containingEndorsements, endorsedBy, stored
		res = map[string]interface{}{
			"chainWork":              "",
			"containingEndorsements": []interface{}{},
			"endorsedBy":             []interface{}{},
			"height":                 v.Height,
			"header":                 header,
			"status":                 v.Status,
			"ref":                    val.RefCount,
			"stored": map[string]interface{}{
				"vtbids": []interface{}{},
			},
		}
	case *BtcBlockAddon:
		// TODO: Support chainWork
		res = map[string]interface{}{
			"chainWork": "",
			"height":    v.Height,
			"header":    header,
			"status":    v.Status,
			"ref":       len(val.Refs),
		}
	case *AltBlockAddon:
	default:
		return nil, fmt.Errorf("Invalid Block Addon")
	}
	return res, nil
}
