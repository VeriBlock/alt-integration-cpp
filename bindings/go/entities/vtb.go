package entities

import (
	"encoding/binary"
	"fmt"
	"io"
)

// Vtb ...
type Vtb struct {
	Version         uint32
	Transaction     VbkPopTx
	MerklePath      VbkMerklePath
	ContainingBlock VbkBlock
}

// ToVbkEncoding ...
func (v *Vtb) ToVbkEncoding(stream io.Writer) error {
	if err := binary.Write(stream, binary.BigEndian, v.Version); err != nil {
		return err
	}
	if v.Version != 1 {
		return fmt.Errorf("VTB serialization version=%d is not implemented", v.Version)
	}
	if err := v.Transaction.ToVbkEncoding(stream); err != nil {
		return err
	}
	if err := v.MerklePath.ToVbkEncoding(stream); err != nil {
		return err
	}
	return v.ContainingBlock.ToVbkEncoding(stream)
}

// FromVbkEncoding ...
func (v *Vtb) FromVbkEncoding(stream io.Reader) error {
	if err := binary.Read(stream, binary.BigEndian, &v.Version); err != nil {
		return err
	}
	if v.Version != 1 {
		return fmt.Errorf("VTB serialization version=%d is not implemented", v.Version)
	}
	if err := v.Transaction.FromVbkEncoding(stream); err != nil {
		return err
	}
	if err := v.MerklePath.FromVbkEncoding(stream); err != nil {
		return err
	}
	return v.ContainingBlock.FromVbkEncoding(stream)
}
