package entities

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io"

	"github.com/VeriBlock/alt-integration-cpp/bindings/go/ffi"
)

// Atv ...
type Atv struct {
	Version      uint32
	Transaction  VbkTx
	MerklePath   VbkMerklePath
	BlockOfProof VbkBlock
}

// GetID ...
func (v *Atv) GetID() []byte {
	buffer := new(bytes.Buffer)
	v.ToVbkEncoding(buffer)
	return ffi.AtvGetID(buffer.Bytes())
}

// ToVbkEncoding ...
func (v *Atv) ToVbkEncoding(stream io.Writer) error {
	if err := binary.Write(stream, binary.BigEndian, v.Version); err != nil {
		return err
	}
	if v.Version != 1 {
		return fmt.Errorf("Atv serialization version=%d is not implemented", v.Version)
	}
	if err := v.Transaction.ToVbkEncoding(stream); err != nil {
		return err
	}
	if err := v.MerklePath.ToVbkEncoding(stream); err != nil {
		return err
	}
	return v.BlockOfProof.ToVbkEncoding(stream)
}

// FromVbkEncoding ...
func (v *Atv) FromVbkEncoding(stream io.Reader) error {
	if err := binary.Read(stream, binary.BigEndian, &v.Version); err != nil {
		return err
	}
	if v.Version != 1 {
		return fmt.Errorf("Atv serialization version=%d is not implemented", v.Version)
	}
	if err := v.Transaction.FromVbkEncoding(stream); err != nil {
		return err
	}
	if err := v.MerklePath.FromVbkEncoding(stream); err != nil {
		return err
	}
	return v.BlockOfProof.FromVbkEncoding(stream)
}
