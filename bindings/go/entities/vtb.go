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
	err := binary.Write(stream, binary.BigEndian, v.Version)
	if err != nil {
		return err
	}
	if v.Version != 1 {
		return fmt.Errorf("VTB serialization version=%d is not implemented", v.Version)
	}
	err = v.Transaction.ToVbkEncoding(stream)
	if err != nil {
		return err
	}
	err = v.MerklePath.ToVbkEncoding(stream)
	if err != nil {
		return err
	}
	return v.ContainingBlock.ToVbkEncoding(stream)
}

// VtbFromVbkEncoding ...
func VtbFromVbkEncoding(stream io.Reader) (*Vtb, error) {
	var version uint32
	err := binary.Read(stream, binary.BigEndian, &version)
	if err != nil {
		return nil, err
	}
	if version != 1 {
		return nil, fmt.Errorf("VTB serialization version=%d is not implemented", version)
	}
	transaction, err := VbkPopTxFromVbkEncoding(stream)
	if err != nil {
		return nil, err
	}
	merklePath, err := VbkMerklePathFromVbkEncoding(stream)
	if err != nil {
		return nil, err
	}
	containingBlock, err := VbkBlockFromVbkEncoding(stream)
	if err != nil {
		return nil, err
	}
	vtb := Vtb{
		version,
		*transaction,
		*merklePath,
		*containingBlock,
	}
	return &vtb, nil
}
