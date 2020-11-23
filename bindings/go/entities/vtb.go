package entities

import (
	"bytes"
	"encoding/binary"
	"encoding/hex"
	"fmt"
	"io"

	"github.com/VeriBlock/alt-integration-cpp/bindings/go/ffi"
)

// VtbID is 32 byte ID of Vtb
type VtbID [32]byte

// Vtb ...
type Vtb struct {
	Version         uint32
	Transaction     VbkPopTx
	MerklePath      VbkMerklePath
	ContainingBlock VbkBlock
}

// GetID ...
func (v *Vtb) GetID() VtbID {
	buffer := new(bytes.Buffer)
	v.ToVbkEncoding(buffer)
	return ffi.VtbGetID(buffer.Bytes())
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

// ToJSON ...
func (v *Vtb) ToJSON() (map[string]interface{}, error) {
	transaction, err := v.Transaction.ToJSON()
	if err != nil {
		return nil, err
	}
	merklePath, err := v.MerklePath.ToJSON()
	if err != nil {
		return nil, err
	}
	containingBlock, err := v.ContainingBlock.ToJSON()
	if err != nil {
		return nil, err
	}
	id := v.GetID()
	res := map[string]interface{}{
		"id":              hex.EncodeToString(id[:]),
		"version":         v.Version,
		"transaction":     transaction,
		"merklePath":      merklePath,
		"containingBlock": containingBlock,
	}
	return res, nil
}
