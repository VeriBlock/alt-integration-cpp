package entities

import (
	"bytes"
	"encoding/binary"
	"encoding/hex"
	"fmt"
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
	"github.com/VeriBlock/alt-integration-cpp/bindings/go/ffi"
)

// VtbBlockName ...
const VtbBlockName = "VTB"

// VtbID is 32 byte ID of Vtb
type VtbID [veriblock.Sha256HashSize]byte

// Bytes ...
func (v *VtbID) Bytes() []byte {
	return v[:]
}

// UnmarshalJSON parses a hash in hex syntax.
func (v *VtbID) UnmarshalJSON(input []byte) error {
	return veriblock.UnmarshalJSON(input, v[:])
}

// ParseVtbID - Parses an VTB ID and panics if invalid size
func ParseVtbID(idBytes []byte) (id VtbID) {
	if len(idBytes) < veriblock.Sha256HashSize || len(idBytes) > veriblock.Sha256HashSize {
		panic("Invalid size of VTB ID")
	}
	copy(id[:], idBytes)
	return
}

// ParseErrVtbID - Parses an VTB ID and returns error if invalid size
func ParseErrVtbID(idBytes []byte) (id VtbID, err error) {
	if len(idBytes) < veriblock.Sha256HashSize || len(idBytes) > veriblock.Sha256HashSize {
		err = fmt.Errorf("Invalid size of VTB ID")
		return
	}
	copy(id[:], idBytes)
	return
}

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

// ToVbkEncodingBytes ...
func (v *Vtb) ToVbkEncodingBytes() ([]byte, error) {
	var buffer bytes.Buffer
	err := v.ToVbkEncoding(&buffer)
	return buffer.Bytes(), err
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

// FromVbkEncodingBytes ...
func (v *Vtb) FromVbkEncodingBytes(data []byte) error {
	buffer := bytes.NewBuffer(data)
	return v.FromVbkEncoding(buffer)
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
