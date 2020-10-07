package entities

import (
	"encoding/binary"
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp"
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
	GetHash() []byte
	GetBlockTime() uint32
	GetDifficulty() uint32
}

// BlockIndex ...
type BlockIndex struct {
	addon veriblock.SerdeRaw
	// Height of the entry in the chain
	Height uint32
	// Block header
	Header GenericBlockHeader
	// Contains status flags
	Status BlockStatus
}

// GetHash ...
func (v *BlockIndex) GetHash() []byte {
	return v.Header.GetHash()
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
	return v.addon.ToRaw(stream)
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
	return v.addon.FromRaw(stream)
}
