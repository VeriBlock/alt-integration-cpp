package entities

import (
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
)

// VbkEndorsement ...
type VbkEndorsement struct {
	ID             [32]byte
	EndorsedHash   [24]byte
	ContainingHash [24]byte
	BlockOfProof   [32]byte
	PayoutInfo     []byte
}

// GetID ...
func (v *VbkEndorsement) GetID() [32]byte {
	return v.ID
}

// GetEndorsedHash ...
func (v *VbkEndorsement) GetEndorsedHash() []byte {
	return v.EndorsedHash[:]
}

// GetContainingHash ...
func (v *VbkEndorsement) GetContainingHash() []byte {
	return v.ContainingHash[:]
}

// GetBlockOfProof ...
func (v *VbkEndorsement) GetBlockOfProof() []byte {
	return v.BlockOfProof[:]
}

// GetPayoutInfo ...
func (v *VbkEndorsement) GetPayoutInfo() []byte {
	return v.PayoutInfo
}

// ToVbkEncoding ...
func (v *VbkEndorsement) ToVbkEncoding(stream io.Writer) error {
	if err := veriblock.WriteSingleByteLenValue(stream, v.ID[:]); err != nil {
		return err
	}
	if err := veriblock.WriteSingleByteLenValue(stream, v.EndorsedHash[:]); err != nil {
		return err
	}
	if err := veriblock.WriteSingleByteLenValue(stream, v.ContainingHash[:]); err != nil {
		return err
	}
	if err := veriblock.WriteSingleByteLenValue(stream, v.BlockOfProof[:]); err != nil {
		return err
	}
	if err := veriblock.WriteSingleByteLenValue(stream, v.PayoutInfo); err != nil {
		return err
	}
	return nil
}

// FromVbkEncoding ...
func (v *VbkEndorsement) FromVbkEncoding(stream io.Reader) error {
	id, err := veriblock.ReadSingleByteLenValueDefault(stream)
	if err != nil {
		return err
	}
	copy(v.ID[:], id)
	endorsedHash, err := veriblock.ReadSingleByteLenValueDefault(stream)
	if err != nil {
		return err
	}
	copy(v.EndorsedHash[:], endorsedHash)
	containingHash, err := veriblock.ReadSingleByteLenValueDefault(stream)
	if err != nil {
		return err
	}
	copy(v.ContainingHash[:], containingHash)
	blockOfProof, err := veriblock.ReadSingleByteLenValueDefault(stream)
	if err != nil {
		return err
	}
	copy(v.BlockOfProof[:], blockOfProof)
	v.PayoutInfo, err = veriblock.ReadSingleByteLenValueDefault(stream)
	if err != nil {
		return err
	}
	return nil
}
