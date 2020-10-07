package entities

import (
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp"
)

// AltEndorsement ...
type AltEndorsement struct {
	ID             [32]byte
	EndorsedHash   []byte
	ContainingHash []byte
	BlockOfProof   [24]byte
	PayoutInfo     []byte
}

// GetID ...
func (v *AltEndorsement) GetID() [32]byte {
	return v.ID
}

// GetEndorsedHash ...
func (v *AltEndorsement) GetEndorsedHash() []byte {
	return v.EndorsedHash
}

// GetContainingHash ...
func (v *AltEndorsement) GetContainingHash() []byte {
	return v.ContainingHash
}

// GetBlockOfProof ...
func (v *AltEndorsement) GetBlockOfProof() []byte {
	return v.BlockOfProof[:]
}

// GetPayoutInfo ...
func (v *AltEndorsement) GetPayoutInfo() []byte {
	return v.PayoutInfo
}

// ToVbkEncoding ...
func (v *AltEndorsement) ToVbkEncoding(stream io.Writer) error {
	if err := veriblock.WriteSingleByteLenValue(stream, v.ID[:]); err != nil {
		return err
	}
	if err := veriblock.WriteSingleByteLenValue(stream, v.EndorsedHash); err != nil {
		return err
	}
	if err := veriblock.WriteSingleByteLenValue(stream, v.ContainingHash); err != nil {
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
func (v *AltEndorsement) FromVbkEncoding(stream io.Reader) error {
	id, err := veriblock.ReadSingleByteLenValueDefault(stream)
	if err != nil {
		return err
	}
	copy(v.ID[:], id)
	v.EndorsedHash, err = veriblock.ReadSingleByteLenValueDefault(stream)
	if err != nil {
		return err
	}
	v.ContainingHash, err = veriblock.ReadSingleByteLenValueDefault(stream)
	if err != nil {
		return err
	}
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
