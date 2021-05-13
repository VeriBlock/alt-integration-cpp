// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package entities

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
)

// BlockStateStatus ...
type BlockStateStatus uint32

// BlockValidityStatus ...
type BlockValidityStatus uint32

const (

	//! BlockStateStatus flags

	// BlockValidTree - AcceptBlockHeader succeded. All ancestors are at least at this state.
	BlockValidTree BlockStateStatus = 1
	// BlockConnected - The block is connected via connectBlock
	BlockConnected BlockStateStatus = 2
	// BlockHasBeenApplied - The block has been successfully applied, likely along with another chain
	BlockCanBeAppliedMaybeWithOtherChain BlockStateStatus = 3
	// BlockCanBeApplied - The chain with the block at its tip is fully valid
	BlockCanBeApplied BlockStateStatus = 4
	// BlockValidMask - All stateful validity levels
	BlockValidMask BlockStateStatus = BlockValidTree |
		BlockConnected | BlockCanBeAppliedMaybeWithOtherChain | BlockCanBeApplied

	//! BlockValidityStatus flags

	// BlockValidUnknown - Default state for validity - validity state is unknown
	BlockValidUnknown BlockValidityStatus = 0

	//! all values from (0, 15] are reserved for BlockStateStatus

	// BlockBootstrap - This is a bootstrap block
	BlockBootstrap BlockValidityStatus = 1 << 4
	// BlockFailedBlock - Block is statelessly valid, but the altchain marked it as failed
	BlockFailedBlock BlockValidityStatus = 1 << 5
	// BlockFailedPop - Block failed state{less,ful} validation due to its payloads
	BlockFailedPop BlockValidityStatus = 1 << 6
	// BlockFailedChild - Block is state{lessly,fully} valid and the altchain did not report it as
	// invalid, but some of the ancestor blocks are invalid
	BlockFailedChild BlockValidityStatus = 1 << 7
	// BlockFailedMask - All invalidity flags
	BlockFailedMask BlockValidityStatus = BlockFailedChild | BlockFailedPop | BlockFailedBlock
	// BlockHasPayloads - AcceptBlock has been executed on this block
	BlockHasPayloads BlockValidityStatus = 1 << 8
	// BlockApplied - The block has been applied via PopStateMachine
	BlockApplied BlockValidityStatus = 1 << 9
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
	Addon veriblock.Serde
	// Height of the entry in the chain
	Height uint32
	// Block header
	Header GenericBlockHeader
	// Contains status flags
	Status uint32
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

// GetVbkBlockHeader ...
func (v *BlockIndex) GetVbkBlockHeader() (block *VbkBlock, err error) {
	block = new(VbkBlock)
	var buffer bytes.Buffer
	err = v.Header.ToRaw(&buffer)
	if err != nil {
		return nil, err
	}
	err = block.FromRaw(&buffer)
	return block, err
}

// GetBtcBlockHeader ...
func (v *BlockIndex) GetBtcBlockHeader() (block *BtcBlock, err error) {
	block = new(BtcBlock)
	var buffer bytes.Buffer
	err = v.Header.ToRaw(&buffer)
	if err != nil {
		return nil, err
	}
	err = block.FromRaw(&buffer)
	return block, err
}

// GetAltBlockHeader ...
func (v *BlockIndex) GetAltBlockHeader() (block *AltBlock, err error) {
	block = new(AltBlock)
	var buffer bytes.Buffer
	err = v.Header.ToRaw(&buffer)
	if err != nil {
		return nil, err
	}
	err = block.FromRaw(&buffer)
	return block, err
}

func (v *BlockIndex) HasFlags(flag BlockValidityStatus) bool {
	return v.Status&uint32(flag) != 0
}

// ToRaw ...
func (v *BlockIndex) ToVbkEncoding(stream io.Writer) error {
	if err := binary.Write(stream, binary.BigEndian, v.Height); err != nil {
		return err
	}
	if err := v.Header.ToRaw(stream); err != nil {
		return err
	}
	if err := binary.Write(stream, binary.BigEndian, v.Status); err != nil {
		return err
	}
	return v.Addon.ToVbkEncoding(stream)
}

// ToRawBytes ...
func (v *BlockIndex) ToVbkEncodingBytes() ([]byte, error) {
	var buffer bytes.Buffer
	err := v.ToVbkEncoding(&buffer)
	return buffer.Bytes(), err
}

// FromRaw ...
func (v *BlockIndex) FromVbkEncoding(stream io.Reader) error {
	if err := binary.Read(stream, binary.BigEndian, &v.Height); err != nil {
		return err
	}
	if err := v.Header.FromRaw(stream); err != nil {
		return err
	}
	if err := binary.Read(stream, binary.BigEndian, &v.Status); err != nil {
		return err
	}
	return v.Addon.FromVbkEncoding(stream)
}

// FromRawBytes ...
func (v *BlockIndex) FromVbkEncodingBytes(data []byte) error {
	buffer := bytes.NewBuffer(data)
	return v.FromVbkEncoding(buffer)
}

type withAltBlockGetEndorsedBy interface {
	AltBlockGetEndorsedBy(altblockHash []byte) ([]AltEndorsement, error)
}

// ToJSON ...
func (v *BlockIndex) ToJSON(popContext withAltBlockGetEndorsedBy) (map[string]interface{}, error) {
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
		err = fmt.Errorf("invalid block header")
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
		altblockHash := v.Header.(*AltBlock).Hash
		ends := []interface{}{}
		if popContext != nil {
			endorsements, err := popContext.AltBlockGetEndorsedBy(altblockHash)
			if err != nil {
				return nil, err
			}
			for _, endorsement := range endorsements {
				ends = append(ends, endorsement.ID)
			}
		}
		res = map[string]interface{}{
			"endorsedBy": ends,
		}
	default:
		return nil, fmt.Errorf("invalid block addon")
	}
	return res, nil
}
