// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package entities

import (
	"bytes"
	"encoding/binary"
	"encoding/hex"
	"errors"
	"fmt"
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
	"github.com/VeriBlock/alt-integration-cpp/bindings/go/ffi"
)

// AtvBlockName ...
const AtvBlockName = "ATV"

// AtvID is 32 byte ID of ATV
type AtvID [veriblock.Sha256HashSize]byte

// AtvIDFromBytes ...
func AtvIDFromBytes(bytes []byte) (AtvID, error) {
	var res AtvID
	if len(bytes) != len(res) {
		return res, errors.New("invalid bytes length")
	}
	copy(res[:], bytes)
	return res, nil
}

// Bytes ...
func (v *AtvID) Bytes() []byte {
	return v[:]
}

// UnmarshalJSON parses a hash in hex syntax.
func (v *AtvID) UnmarshalJSON(input []byte) error {
	return veriblock.UnmarshalJSON(input, v[:])
}

// ParseAtvID - Parses an ATV ID and panics if invalid size
func ParseAtvID(idBytes []byte) (id AtvID) {
	if len(idBytes) < veriblock.Sha256HashSize || len(idBytes) > veriblock.Sha256HashSize {
		panic("Invalid size of ATV ID")
	}
	copy(id[:], idBytes)
	return
}

// ParseErrAtvID - Parses an ATV ID and returns error if invalid size
func ParseErrAtvID(idBytes []byte) (id AtvID, err error) {
	if len(idBytes) < veriblock.Sha256HashSize || len(idBytes) > veriblock.Sha256HashSize {
		err = fmt.Errorf("Invalid size of ATV ID")
		return
	}
	copy(id[:], idBytes)
	return
}

// Atv ...
type Atv struct {
	Version      uint32
	Transaction  VbkTx
	MerklePath   VbkMerklePath
	BlockOfProof VbkBlock
}

// Name ...
func (v *Atv) Name() string { return AtvBlockName }

// GetID ...
func (v *Atv) GetID() AtvID {
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

// ToVbkEncodingBytes ...
func (v *Atv) ToVbkEncodingBytes() ([]byte, error) {
	var buffer bytes.Buffer
	err := v.ToVbkEncoding(&buffer)
	return buffer.Bytes(), err
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

// FromVbkEncodingBytes ...
func (v *Atv) FromVbkEncodingBytes(data []byte) error {
	buffer := bytes.NewBuffer(data)
	return v.FromVbkEncoding(buffer)
}

// ToJSON ...
func (v *Atv) ToJSON() (map[string]interface{}, error) {
	transaction, err := v.Transaction.ToJSON()
	if err != nil {
		return nil, err
	}
	merklePath, err := v.MerklePath.ToJSON()
	if err != nil {
		return nil, err
	}
	blockOfProof, err := v.BlockOfProof.ToJSON()
	if err != nil {
		return nil, err
	}
	id := v.GetID()
	res := map[string]interface{}{
		"id":           hex.EncodeToString(id[:]),
		"version":      v.Version,
		"transaction":  transaction,
		"merklePath":   merklePath,
		"blockOfProof": blockOfProof,
	}
	return res, nil
}
