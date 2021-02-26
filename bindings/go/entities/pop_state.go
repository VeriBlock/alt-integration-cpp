package entities

import (
	"bytes"
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
)

// Endorsement ...
type Endorsement interface {
	veriblock.Serde
	GetID() [32]byte
	GetEndorsedHash() []byte
	GetContainingHash() []byte
	GetBlockOfProof() []byte
}

// PopState ...
type PopState struct {
	// (memory only) Is contains Alt or Vbk endorsements
	IsAlt bool
	// List of containing endorsements in this block
	ContainingEndorsements []Endorsement
}

// ToRaw ...
func (v *PopState) ToRaw(stream io.Writer) error {
	return veriblock.WriteArrayOf(stream, v.ContainingEndorsements, func(stream io.Writer, val interface{}) error {
		var endorsement Endorsement
		endorsement, ok := val.(*VbkEndorsement)
		if !ok {
			endorsement = val.(*AltEndorsement)
		}
		return endorsement.ToVbkEncoding(stream)
	})
}

// ToRawBytes ...
func (v *PopState) ToRawBytes() ([]byte, error) {
	var buffer bytes.Buffer
	err := v.ToRaw(&buffer)
	return buffer.Bytes(), err
}

// FromRaw ...
func (v *PopState) FromRaw(stream io.Reader) error {
	containingEndorsements, err := veriblock.ReadArrayOf(stream, 0, veriblock.MaxPopDataVbkBlock, func(stream io.Reader) (interface{}, error) {
		var endorsement Endorsement = &VbkEndorsement{}
		if v.IsAlt {
			endorsement = &AltEndorsement{}
		}
		err := endorsement.FromVbkEncoding(stream)
		if err != nil {
			return nil, err
		}
		return endorsement, nil
	})
	if err != nil {
		return err
	}
	v.ContainingEndorsements = make([]Endorsement, len(containingEndorsements))
	for i, containingEndorsement := range containingEndorsements {
		v.ContainingEndorsements[i] = containingEndorsement.(Endorsement)
	}
	return nil
}

// FromRawBytes ...
func (v *PopState) FromRawBytes(data []byte) error {
	buffer := bytes.NewBuffer(data)
	return v.FromRaw(buffer)
}
