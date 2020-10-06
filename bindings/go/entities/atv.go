package entities

import (
	"encoding/binary"
	"fmt"
	"io"
)

// Atv ...
type Atv struct {
	Version      uint32
	Transaction  VbkTx
	MerklePath   VbkMerklePath
	BlockOfProof VbkBlock
}

// ToVbkEncoding ...
func (v *Atv) ToVbkEncoding(stream io.Writer) error {
	err := binary.Write(stream, binary.BigEndian, v.Version)
	if err != nil {
		return err
	}
	if v.Version != 1 {
		return fmt.Errorf("Atv serialization version=%d is not implemented", v.Version)
	}
	err = v.Transaction.ToVbkEncoding(stream)
	if err != nil {
		return err
	}
	err = v.MerklePath.ToVbkEncoding(stream)
	if err != nil {
		return err
	}
	return v.BlockOfProof.ToVbkEncoding(stream)
}

// AtvFromVbkEncoding ...
func AtvFromVbkEncoding(stream io.Reader) (*Atv, error) {
	var version uint32
	err := binary.Read(stream, binary.BigEndian, &version)
	if err != nil {
		return nil, err
	}
	if version != 1 {
		return nil, fmt.Errorf("Atv serialization version=%d is not implemented", version)
	}
	transaction, err := VbkTxFromVbkEncoding(stream)
	if err != nil {
		return nil, err
	}
	merklePath, err := VbkMerklePathFromVbkEncoding(stream)
	if err != nil {
		return nil, err
	}
	blockOfProof, err := VbkBlockFromVbkEncoding(stream)
	if err != nil {
		return nil, err
	}
	Atv := Atv{
		version,
		*transaction,
		*merklePath,
		*blockOfProof,
	}
	return &Atv, nil
}
